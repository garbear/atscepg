/*
 * Copyright (C) 2006-2010 Alex Lasnier <alex@fepg.org>
 *
 * This file is part of ATSC EPG
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdarg.h>
#include <algorithm>

#include <vdr/filter.h>
#include <libsi/section.h>
#include <libsi/descriptor.h>

#include "filter.h"
#include "filterManager.h"
#include "tables.h"
#include "tools.h"


///////////////////////////////////////////////////////////////////////////////


#ifdef AE_ENABLE_LOG
#define F_LOG(T, s, ...) Logger.Printf(T, "(F:%d) "s, fNum, ##__VA_ARGS__)
#else
#define F_LOG(T, s, ...) 
#endif


#define MGT_SCAN_DELAY 60 
#define STT_SCAN_DELAY 60 


///////////////////////////////////////////////////////////////////////////////


cATSCFilter::cATSCFilter(int num)
{
  fNum = num;
  F_LOG(L_DBGV, "Created.");
  FilterManager.AddFilter(this);
  
  newMGTVersion = 0;
  gotMGT = false;
  gotVCT = false;
  gotRRT = false;
  
  lastScanMGT = 0;
  lastScanSTT = 0;
  prevTransponder = -1;
 
  //attachedDevice = NULL;
  
  Set(0x1FFB, 0xC7); // MGT

  // Set(0x1FFB, 0xCA); // RRT
  // Set(0x1FFB, 0xCD); // SST
  // Set(0x1FFB, 0xCE); // DET
  // Set(0x1FFB, 0xCF); // DST
  // Set(0x1FFB, 0xD3); // DCCT
  // Set(0x1FFB, 0xD4); // DCCSCT
}


//----------------------------------------------------------------------------

cATSCFilter::~cATSCFilter()
{

}


//----------------------------------------------------------------------------

void cATSCFilter::SetStatus(bool On)
{ 
  if (!On)
  {
    if (prevTransponder != Transponder())
      cFilter::SetStatus(false); 
  }
  else 
  {
    if (const cChannel* c = Channel()) {
      if (c->Number())
        F_LOG(L_MSG, "Switched to channel %d (%s)", c->Number(), *(c->GetChannelID().ToString()));
      else {
        F_LOG(L_DBG, "Channel scan: not activating filter");
        prevTransponder = -1;
        cFilter::SetStatus(false);
        return;
      } 
    }

    if (prevTransponder != Transponder())
    {
      F_LOG(L_DBG, "Different transponder: resetting");
      prevTransponder = Transponder();
      ResetFilter();
      
      if (FilterManager.Set(this, Transponder()))
        cFilter::SetStatus(true);
      else
        F_LOG(L_DBG, "This transponder is being updated by another filter.");
    }
    else 
    {
      F_LOG(L_DBG, "Same transponder: not resetting");
    }
  }
}


//----------------------------------------------------------------------------

void cATSCFilter::ResetFilter(void)
{
  gotMGT = false;
  gotVCT = false;
  gotRRT = false;

  newMGTVersion = 0;
  lastScanMGT = 0;
  lastScanSTT = 0;

  channelSIDs.clear();
  FilterManager.Reset(this);
  
  eitPids.clear();
  ettEIDs.clear();
  ettPids.clear();
    
  // Add(0x0000, 0x00); // PAT
  Add(0x1FFB, 0xC8, 0xFE); // VCT-T/C
}


//----------------------------------------------------------------------------

void cATSCFilter::Process(u_short Pid, u_char Tid, const u_char* Data, int length)
{  
  if (length < 15) {
    F_LOG(L_ERR, "Insufficient data length.");
    return;
  }
  
  time_t now = time(NULL);
  
  switch (Tid)
  {
    case 0x00: // PAT
      if (ProcessPAT(Data, length))
        Del(0x0000, 0x00);
    break;
    
    case 0x02: // PMT
      if (ProcessPMT(Data, length))
        Del(Pid, 0x02);
    break;
    
    case 0xC7: // MGT: Master Guide Table
      if (!gotVCT || gotMGT || now - lastScanMGT <= MGT_SCAN_DELAY) return;
      if (ProcessMGT(Data, length)) {
        gotMGT = true;
      }
      lastScanMGT = now;
    break;
      
    case 0xC8: // VCT-T: Terrestrial Virtual Channel Table
    case 0xC9: // VCT-C: Cable Virtual Channel Table
      if (gotVCT) return;
      F_LOG(L_MSG, "Received VCT-%c.", Tid==0xC8?'T':'C');
      if (ProcessVCT(Data, length)) {
        gotVCT = true;
        Del(0x1FFB, Tid);
      }
    break; 
      
    case 0xCA: // RRT: Rating Region Table 
      if (gotRRT) return;
      F_LOG(L_DBG, "Received RRT: Not yet implemented.");
      //RRT rrt(Data, length);
      gotRRT = true; 
    break; 
      
    case 0xCB: // EIT: Event Information Table
      ProcessEIT(Data, length, Pid);
    break;
      
    case 0xCC: // ETT: Extended Text Table
      ProcessETT(Data, length);
    break; 
      
    case 0xCD: // STT: System Time Table  
      if (now - lastScanSTT <= STT_SCAN_DELAY) return;
      F_LOG(L_MSG, "Received STT.");
      lastScanSTT = now;
    break;
      
    case 0xCE: // DET
      F_LOG(L_DBG, "Received DET: Not yet implemented.");
    break;
             
    case 0xD3: // DCC 
      F_LOG(L_DBG, "Received DCC: Not yet implemented.");
    break;
      
    case 0xD4: // DCCSCT
      F_LOG(L_DBG, "Received DCCSCT: Not yet implemented.");
    break;
      
    default:
      F_LOG(L_DBG, "Unknown TID: 0x%02X", Tid);
    break;   
  }
}


//----------------------------------------------------------------------------

bool cATSCFilter::ProcessPAT(const uint8_t* data, int length)
{
  SI::PAT pat(data, false);
  if (!pat.CheckCRCAndParse())
    return false;

  F_LOG(L_DBG, "Received PAT.");
  SI::PAT::Association assoc;
  for (SI::Loop::Iterator it; pat.associationLoop.getNext(assoc, it); ) 
  {
    F_LOG(L_DBG, "PAT: Found PMT (PID: %d, SID: %d)", assoc.getPid(), assoc.getServiceId());
    Add(assoc.getPid(), 0x02);
  }
  
  return true;
}

  
//----------------------------------------------------------------------------

bool cATSCFilter::ProcessPMT(const uint8_t* data, int length)
{
  SI::PMT pmt(data, false);
  if (!pmt.CheckCRCAndParse())
    return false;
  
  F_LOG(L_DBG, "================== PMT ==================");
  F_LOG(L_DBG, "  SID: %d", pmt.getServiceId());
  
  int numStream=1;
  SI::PMT::Stream stream;
  for (SI::Loop::Iterator it; pmt.streamLoop.getNext(stream, it); ) 
  {
    F_LOG(L_DBG, "  Stream %d --- PID: %d, Stream type: 0x%02X", numStream++, stream.getPid(), stream.getStreamType());
    SI::Descriptor *d;
    for (SI::Loop::Iterator it; (d = stream.streamDescriptors.getNext(it)); )
    {
      F_LOG(L_DBG, "    Descriptor --- Tag: 0x%02X", d->getDescriptorTag());
      delete d;
    }
  } 
  F_LOG(L_DBG, "=========================================");
  return true;
}


//----------------------------------------------------------------------------
  
bool cATSCFilter::ProcessMGT(const uint8_t* data, int length)
{
  // Do we have a newer version?
  newMGTVersion = PSIPTable::ExtractVersion(data);
  int oldMGTVersion = FilterManager.GetMgtVersion(Transponder());
  
  if (int(newMGTVersion) == oldMGTVersion)
  {
    F_LOG(L_MSG, "Received MGT: same version, no update (%d).", newMGTVersion); 
    return false;  
  }
  
  F_LOG(L_MSG, "Received MGT: new/imcomplete version, updating (%d -> %d).", oldMGTVersion, newMGTVersion);
  MGT mgt(data, length);
  if (!mgt.CheckCRC())
    return false;
    
  eitPids.clear();  
  ettEIDs.clear();  
  ettPids.clear();
      
  for (u8 k = 0; k < mgt.NumberOfTables(); k++)
  {
    const Table* t = mgt.GetTable(k);
    dprint(L_MGT, "Table ID: 0x%02X, PID: 0x%04X, Bytes: %d", t->tid, t->pid, t->number_bytes);
    if (t->number_bytes == 0)
      continue;

    switch (t->tid)
    {   
      case 0xCC: // ETT 
        if (t->table_type == 0x0004) { // Channel ETT
          F_LOG(L_MGT, "MGT: Found channel ETT PID");
          // Usually provides a short description, not very useful.
        }  
        else { // Event ETT 
          F_LOG(L_MGT, "MGT: Found ETT PID: %d", t->pid);
          ettPids.push_back(t->pid); // Save these for after we have the EITs
        }
      break; 

      case 0xCB: // EIT
      {
        F_LOG(L_MGT, "MGT: Found EIT PID: %d", t->pid);
        
        std::list<uint16_t>::const_iterator itr;
        for(itr = channelSIDs.begin(); itr != channelSIDs.end(); itr++)
        {
          u32 v = (((u32) *itr) << 16) | t->pid;
          eitPids.push_back(v);
          Add(t->pid, t->tid);
        }
      }
      break;
      
      case 0xCA: // RRT
      case 0xC8: // VCT
        // Always the same pid...
      break;
      
      default:
        F_LOG(L_MGT, "MGT: Unhandled table id 0x%02X", t->tid);
    }
  }
  
  return true;
}


//----------------------------------------------------------------------------

bool cATSCFilter::ProcessVCT(const uint8_t* data, int length)
{
  VCT vct(data, length);
  if (!vct.CheckCRC())
    return false;

  currentTID = vct.TID();
  sidTranslator.Update(&vct);
    
  for (u32 i=0; i<vct.NumberOfChannels(); i++) 
  {
    F_LOG(L_EIT, "  PMT SID: %d --> VCT SID: %d", vct.GetChannel(i)->ProgramNumber(), vct.GetChannel(i)->Sid());
    if (vct.GetChannel(i)->HasEit())
      channelSIDs.push_back( vct.GetChannel(i)->Sid() );
  }
  
  channelSIDs.sort();
  channelSIDs.unique();
   
  return true;
}


//----------------------------------------------------------------------------

bool cATSCFilter::ProcessEIT(const uint8_t* data, int length, uint16_t Pid)
{
  u16 sid = EIT::ExtractSourceID(data);
  u32 val = (((u32) sid) << 16) | Pid;
  
  // Check if we are expecting this EIT              
  std::list<uint32_t>::iterator itr = find(eitPids.begin(), eitPids.end(), val);
    
  if (itr == eitPids.end()) // We have already seen or are not expecting this EIT
  {
    std::list<uint16_t>::const_iterator cit = find(channelSIDs.begin(), channelSIDs.end(), sid);
    
    if (cit != channelSIDs.end())
      F_LOG(L_EIT, "Received EIT (SID: %d PID: 0x%04X) [Already seen]", sid, Pid );
    else {
      F_LOG(L_EIT, "Received EIT not referred to in MGT (SID: %d PID: 0x%04X)", sid, Pid );
      /*
      EIT eit(data, length);
      for (u32 i=0; i<eit.NumberOfEvents(); i++) 
        F_LOG(L_EIT, ">> %s", eit.GetEvent(i)->TitleText());
      */
    }
  }  
  else // Add events to schedule 
  {
    EIT eit(data, length);
    if (!eit.CheckCRC())
      return false;

    F_LOG(L_EIT, "Received EIT (SID: %d PID: 0x%04X) [%d left]", sid, Pid , eitPids.size() );
    eitPids.erase(itr);
    Del(Pid, 0xCB);
    
    VDRInterface::AddEvents(GetChannel(eit.SourceID()), eit);
      
    // Now look for ETTs for these events
    for (u32 i=0; i<eit.NumberOfEvents(); i++)
    {
      const Event* e = eit.GetEvent(i);
      if (e->ETM_location == 0x01 || e->ETM_location == 0x02) // There is an ETT for this event
        ettEIDs.push_back(e->event_id);
    }
  }
    
  if (eitPids.size() == 0) 
  {
    F_LOG(L_MSG, "Received all EITs.");

    // Now start looking for ETTs
    for(std::list<uint16_t>::iterator i = ettPids.begin(); i != ettPids.end(); i++) {
      Add(*i, 0xCC);
    } 
  }
  
  return true;
}


//----------------------------------------------------------------------------

bool cATSCFilter::ProcessETT(const uint8_t* data, int length)
{
  u16 eid = ETT::ExtractEventID(data);
    
  std::list<uint16_t>::iterator itr = find(ettEIDs.begin(), ettEIDs.end(), eid);
    
  if (itr == ettEIDs.end()) 
  {
    F_LOG(L_ETT, "Unexpected ETT (EID: %d)", eid);
  }
  else
  {
    ETT ett(data, length);
    if (!ett.CheckCRC())
      return false;

    F_LOG(L_ETT, "Received ETT (EID: %d)", eid);
    ettEIDs.erase(itr);
    // We cannot Del(Pid, Tid) because we do not know how many ETTs 
    // we will get per PID. Or maybe there is a way to know this...
    VDRInterface::AddDescription(GetChannel(ett.SourceID()), ett);
  }
    
  if (ettEIDs.size() == 0) 
  {
    F_LOG(L_MSG, "Received all ETTs.");
    F_LOG(L_MSG, "Got all event information for this transport stream.");

    // Stop looking for ETTs
    for(std::list<uint16_t>::iterator i = ettPids.begin(); i != ettPids.end(); i++) {
      Del(*i, 0xCC);
    }
    
    FilterManager.SetMgtVersion(Transponder(), newMGTVersion);
    gotMGT = false; // Start looking for new versions
  }
  
  return true; 
}


//----------------------------------------------------------------------------

cChannel* cATSCFilter::GetChannel(uint16_t sid) const
{
  cChannel* channel = NULL;
  uint16_t pmtSid = sidTranslator.GetPmtSid(sid);
  if (pmtSid)
  {
    tChannelID channelIDSearch(cSource::stAtsc, 0x00, currentTID, pmtSid);
    channel = Channels.GetByChannelID(channelIDSearch, true);  
  }
  
  return channel;
}


//////////////////////////////////////////////////////////////////////////////


