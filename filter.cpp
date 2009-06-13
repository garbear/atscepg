/*
 * Copyright (C) 2006-2009 Alex Lasnier <alex@fepg.org>
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

#include <vdr/plugin.h>
#include <vdr/filter.h>
#include <vdr/device.h>

#include <libsi/section.h>
#include <libsi/descriptor.h>

#include "filter.h"
#include "tables.h"
#include "tools.h"


///////////////////////////////////////////////////////////////////////////////


cATSCFilter::cATSCFilter(int num)
{
  fNum = num;
  dfprint(L_DBGV, "Created.");
  mgt = NULL;
  newMGTVersion = 0;
  gotVCT = false;
  gotRRT = false;
  
  lastScanMGT = 0;
  lastScanSTT = 0;
  prevTransponder = -1;
 
  attachedDevice = NULL;
  
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
  Detach();
  delete mgt;
}


//----------------------------------------------------------------------------

void cATSCFilter::Attach(cDevice* device)
{
  if (!attachedDevice) 
  {
    attachedDevice = device;

    if (attachedDevice) 
    {
      attachedDevice->AttachFilter(this);
      dfprint(L_DBGV, "Attached.");
    }
  }
  
}

//----------------------------------------------------------------------------

void cATSCFilter::Detach(void)
{
  if (attachedDevice) 
  {
    attachedDevice->Detach(this);
    attachedDevice = NULL;
    dfprint(L_DBGV, "Detached.");
  }
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
        dfprint(L_MSG, "Switched to channel %d", c->Number());
      else {
        dfprint(L_DBG, "Channel scan: not activating filter");
        prevTransponder = -1;
        cFilter::SetStatus(false);
        return;
      } 
    }

    if (prevTransponder != Transponder())
    {
      dfprint(L_DBG, "Different transponder: resetting");
      prevTransponder = Transponder();
      ResetFilter();
      cFilter::SetStatus(true);
    }
    else 
    {
      dfprint(L_DBG, "Same transponder: not resetting");
    }
  }
}


//----------------------------------------------------------------------------

void cATSCFilter::ResetFilter(void)
{
  gotVCT = false;
  gotRRT = false;

  newMGTVersion = 0;
  lastScanMGT = 0;
  lastScanSTT = 0;

  channelSIDs.clear();

  eitPids.clear();
  ettEIDs.clear();
  ettPids.clear();
    
  // Add(0x0000, 0x00); // PAT
  Add(0x1FFB, 0xC8); // VCT-T
  Add(0x1FFB, 0xC9); // VCT-C
      
  delete mgt;
  mgt = NULL;
}


//----------------------------------------------------------------------------

#define MGT_SCAN_DELAY 60 
#define STT_SCAN_DELAY 60 


void cATSCFilter::Process(u_short Pid, u_char Tid, const u_char* Data, int length)
{  
  if (length < 15) {
    dfprint(L_ERR, "Insufficient data length.");
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
      if (!gotVCT || now - lastScanMGT <= MGT_SCAN_DELAY) return;
      ProcessMGT(Data, length);
      lastScanMGT = time(NULL);
    break;
      
    case 0xC8: // VCT-T: Terrestrial Virtual Channel Table
    case 0xC9: // VCT-C: Cable Virtual Channel Table
      if (gotVCT) return; 
      if (ProcessVCT(Data, length)) {
        gotVCT = true;
        Del(0x1FFB, Tid);
      }
    break; 
      
    case 0xCA: // RRT: Rating Region Table 
      if (gotRRT) return;
      dfprint(L_DBG, "Received RRT: Not yet implemented.");
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
      dfprint(L_MSG, "Received STT.");
      vdrInterface.UpdateSTT(Data, length);
      lastScanSTT = time(NULL);
    break;
      
    case 0xCE: // DET
      dfprint(L_DBG, "Received DET: Not yet implemented.");
    break;
             
    case 0xD3: // DCC 
      dfprint(L_DBG, "Received DCC: Not yet implemented.");
    break;
      
    case 0xD4: // DCCSCT
      dfprint(L_DBG, "Received DCCSCT: Not yet implemented.");
    break;
      
    default:
      dfprint(L_DBG, "Unknown TID: 0x%02X", Tid);
    break;   
  }
}


//----------------------------------------------------------------------------

bool cATSCFilter::ProcessPAT(const uint8_t* data, int length)
{
  SI::PAT pat(data, false);
  if (!pat.CheckCRCAndParse())
    return false;

  dfprint(L_DBG, "Received PAT.");
  SI::PAT::Association assoc;
  for (SI::Loop::Iterator it; pat.associationLoop.getNext(assoc, it); ) 
  {
    dfprint(L_DBG, "PAT: Found PMT (pid: %d, sid: %d)", assoc.getPid(), assoc.getServiceId());
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
  
  SI::PMT::Stream stream;
  for (SI::Loop::Iterator it; pmt.streamLoop.getNext(stream, it); ) 
  {
    SI::Descriptor *d;
    for (SI::Loop::Iterator it; (d = stream.streamDescriptors.getNext(it)); )
    {
      dfprint(L_DBG, "DESC: PID=%04X  ST=%02X  TAG=%02X", stream.getPid(), stream.getStreamType(), d->getDescriptorTag());
      delete d;
    }
  } 

  return true;
}


//----------------------------------------------------------------------------
  
bool cATSCFilter::ProcessMGT(const uint8_t* data, int length)
{
  // Do we have a newer version?
  newMGTVersion = PSIPTable::ExtractVersion(data);

  if ( ((int)newMGTVersion) != GetMGTVersion())
  {
    dfprint(L_MSG|L_MGT, "Received MGT: new/imcomplete version, updating (%d -> %d).", GetMGTVersion(), newMGTVersion);
    if (!mgt)
      mgt = new MGT(data, length);
    else
      mgt->Update(data, length);
      
    if (!mgt->CheckCRC())
      return false;

    //SetMGTVersion(newMGTVersion); //XXX: This should probably be done after we have received all data
  }
  else 
  { 
    dfprint(L_MSG|L_MGT, "Received MGT: same version, no update (%d).", newMGTVersion); 
    return true;
  }
    
  eitPids.clear();  
  ettEIDs.clear();  
  ettPids.clear();
      
  for (u8 k = 0; k < mgt->NumberOfTables(); k++)
  {
    const Table* t = mgt->GetTable(k);
    
    switch (t->tid)
    {   
      case 0xCC: // ETT 
        if (t->table_type == 0x0004) { // Channel ETT
          dfprint(L_MGT, "MGT: Found channel ETT PID");
          // Usually provides a short description, not very useful.
        }  
        else { // Event ETT 
          dfprint(L_MGT, "MGT: Found ETT PID: %d", t->pid);
          ettPids.push_back(t->pid); // Save these for after we have the EITs
        }
      break; 

      case 0xCB: // EIT
        dfprint(L_MGT, "MGT: Found EIT PID: %d", t->pid);
          
        for (size_t i = 0; i < channelSIDs.size(); i++)
        {
          u32 v = (((u32) channelSIDs[i]) << 16) | t->pid;
          eitPids.push_back(v);
          Add(t->pid, t->tid);
        }
      break;
      
      case 0xCA: // RRT
      case 0xC8: // VCT
        // Always the same pid...
      break;
      
      default:
        dfprint(L_MGT, "MGT: Unhandled table id 0x%02X", t->tid);
    }
  }
  
  return true;
}


//----------------------------------------------------------------------------

bool cATSCFilter::ProcessVCT(const uint8_t* data, int length)
{
  dfprint(L_VCT|L_MSG, "Received VCT.");

  VCT vct(data, length);
  if (!vct.CheckCRC())
    return false;

  vdrInterface.AddChannels(vct);
    
  for (u32 i=0; i<vct.NumberOfChannels(); i++) 
  {
    channelSIDs.push_back( vct.GetChannel(i)->Sid() );
  }
  
  return true;
}


//----------------------------------------------------------------------------

bool cATSCFilter::ProcessEIT(const uint8_t* data, int length, uint16_t Pid)
{
  u16 sid = EIT::ExtractSourceID(data);
  u32 val = (((u32) sid) << 16) | Pid;
  
  // Check if we are expecting this EIT              
  std::list<uint32_t>::iterator itr;
  for(itr = eitPids.begin(); itr != eitPids.end() && val != *itr; itr++) { /* do nothing */ } 
    
  if (itr == eitPids.end()) // We have already seen or are not expecting this EIT
  {
    bool found = false;
    for (size_t i = 0; i < channelSIDs.size() && !found; i++) {
      if (channelSIDs[i] == sid)
        found = true;
    }
    if (found)
      dfprint(L_EIT, "Received EIT (SID: %d PID: %d) [Already seen]", sid, Pid );
    else  
      dfprint(L_EIT, "Received EIT not referred to in MGT (SID: %d PID: %d)", sid, Pid );
  }  
  else // Add events to schedule 
  {
    EIT eit(data, length);
    if (!eit.CheckCRC())
      return false;

    dfprint(L_EIT, "Received EIT (SID: %d PID: %d) [%d left]", sid, Pid , eitPids.size() );
    eitPids.erase(itr);
    Del(Pid, 0xCB);
    
    vdrInterface.AddEventsToSchedule(eit);
      
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
    dfprint(L_MSG|L_EIT, "Received all EITs.");

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
    
  std::list<uint16_t>::iterator itr;
  for(itr = ettEIDs.begin(); itr != ettEIDs.end() && eid != *itr; itr++) { /* do nothing */ }
    
  if (itr == ettEIDs.end()) 
  {
    dfprint(L_ETT, "Unexpected ETT (EID: %d)", eid);
  }
  else
  {
    ETT ett(data, length);
    if (!ett.CheckCRC())
      return false;

    dfprint(L_ETT, "Received ETT (EID: %d)", eid);
    ettEIDs.erase(itr);
    // We cannot Del(Pid, Tid) because we do not know how many ETTs 
    // we will get per PID. Or maybe there is a way to know this...
    vdrInterface.AddDescription(ett);
  }
    
  if (ettEIDs.size() == 0) 
  {
    dfprint(L_MSG|L_ETT, "Received all ETTs.");
    dfprint(L_MSG, "Got all event information for this transport stream.");
    
    SetMGTVersion(newMGTVersion);
    
    // Stop looking for ETTs
    for(std::list<uint16_t>::iterator i = ettPids.begin(); i != ettPids.end(); i++) {
      Del(*i, 0xCC);
    }
  }
  
  return true; 
}


//----------------------------------------------------------------------------

int cATSCFilter::GetMGTVersion(void)
{
  std::map<int,uint8_t>::const_iterator itr = MGTVersions.find(Transponder());
  if (itr == MGTVersions.end()) // Key not found
    return -1;

  return itr->second;
}


//----------------------------------------------------------------------------

void cATSCFilter::SetMGTVersion(uint8_t version)
{
  MGTVersions[Transponder()] = version;
}


//----------------------------------------------------------------------------
#ifdef AE_DEBUG

void cATSCFilter::dfprint(uint16_t type, const char* msg, ...)
{
  char* output = NULL;
  va_list ap;
  va_start(ap, msg);
  vasprintf(&output, msg, ap);
  va_end(ap);
  
  dprint(type, "(F:%d) %s", fNum, output);
  free(output);
}

#endif

//////////////////////////////////////////////////////////////////////////////


