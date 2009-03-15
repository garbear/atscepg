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

#include <vdr/plugin.h>
#include <vdr/filter.h>
#include  <vdr/device.h>

#include "filter.h"
#include "tables.h"
#include "tools.h"


///////////////////////////////////////////////////////////////////////////////


cATSCFilter* cATSCFilter::instance = NULL;


///////////////////////////////////////////////////////////////////////////////


cATSCFilter::cATSCFilter()
{
  dprint(L_DBGV, "ATSCFilter Created.");
  mgt = NULL;

  gotVCT = false;
  gotRRT = false;
  
  lastScanMGT = 0;
  lastScanSTT = 0;
   
  attachedDevice = NULL;
  currentChannel = NULL;
  
  Set(0x1FFB, 0xC7); // MGT
  Set(0x1FFB, 0xC8); // VCT-T
  Set(0x1FFB, 0xC9); // VCT-C
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


//-----------------------------------------------------------------------------

cATSCFilter* cATSCFilter::Instance(void)
{
  if (!instance)
    instance = new cATSCFilter();
    
  return instance;
}


//-----------------------------------------------------------------------------

void cATSCFilter::Destroy(void)
{
  delete instance;
  instance = NULL;
  dprint(L_DBGV, "ATSCFilter Destroyed.");
}


//----------------------------------------------------------------------------

void cATSCFilter::Attach(cDevice* device, const cChannel* channel)
{
  if (!attachedDevice) 
  {
    attachedDevice = device;
    currentChannel = channel;
    if (attachedDevice) 
    {
      attachedDevice->AttachFilter(this);
      dprint(L_DBGV, "ATSCFilter Attached.");
    }
  }
  
}

//----------------------------------------------------------------------------

void cATSCFilter::Detach(void)
{
  if (attachedDevice) 
  {
    currentChannel = NULL;
    attachedDevice->Detach(this);
    attachedDevice = NULL;
    dprint(L_DBGV, "ATSCFilter Detached.");
  }
}
  
//----------------------------------------------------------------------------

void cATSCFilter::SetStatus(bool On)
{ 
  if (On) 
  {
    dprint(L_DBG, "Resetting ATSC Filter");
    gotVCT = false;
    gotRRT = false;
    
    lastScanMGT = 0;
    lastScanSTT = 0;
    
    channelSIDs.clear();

    eitPids.clear();
    ettEIDs.clear();
    ettPids.clear();
    
    if (mgt) {
      delete mgt;
      mgt = NULL;
    }  
  }
  
  cFilter::SetStatus(On);
}


//----------------------------------------------------------------------------

#define MGT_SCAN_DELAY 60 
#define STT_SCAN_DELAY 60 


void cATSCFilter::Process(u_short Pid, u_char Tid, const u_char* Data, int Length)
{  
  time_t now = time(NULL);
  
  switch (Tid)
  {
    case 0xC7: // MGT: Master Guide Table
      if (!gotVCT || now - lastScanMGT <= MGT_SCAN_DELAY) return;
      ProcessMGT(Data);
      lastScanMGT = time(NULL);
    break;
      
    case 0xC8: // VCT-T: Terrestrial Virtual Channel Table
    case 0xC9: // VCT-C: Cable Virtual Channel Table
      if (gotVCT) return; 
      ProcessVCT(Data);
      gotVCT = true;
    break; 
      
    case 0xCA: // RRT: Rating Region Table 
      if (gotRRT) return;
      dprint(L_DBG, "Received RRT: Not yet implemented.");
      //RRT rrt(Data);
      gotRRT = true; 
    break; 
      
    case 0xCB: // EIT: Event Information Table
      ProcessEIT(Data, Pid);
    break;
      
    case 0xCC: // ETT: Extended Text Table
      ProcessETT(Data);
    break; 
      
    case 0xCD: // STT: System Time Table  
      if (now - lastScanSTT <= STT_SCAN_DELAY) return;
      dprint(L_MSG, "Received STT.");
      //vdrInterface.UpdateSTT(Data);
      lastScanSTT = time(NULL);
    break;
      
    case 0xCE: // DET
      dprint(L_DBG, "Received DET: Not yet implemented.");
    break;
             
    case 0xD3: // DCC 
      dprint(L_DBG, "Received DCC: Not yet implemented.");
    break;
      
    case 0xD4: // DCCSCT
      dprint(L_DBG, "Received DCCSCT: Not yet implemented.");
    break;
      
    default:
      dprint(L_DBG, "Unknown TID: 0x%02X", Tid);
    break;   
  }
  
}


//----------------------------------------------------------------------------

void cATSCFilter::ProcessMGT(const uint8_t* data)
{
  // Do we have a newer version?
  uint8_t newVersion = PSIPTable::ExtractVersion(data);

  if ( ((int)newVersion) != GetMGTVersion())
  {
    dprint(L_MSG|L_MGT, "Received MGT: new version, updating (%d -> %d).", GetMGTVersion(), newVersion);
    if (!mgt)
      mgt = new MGT(data);
    else
      mgt->Update(data);

    SetMGTVersion(newVersion); //XXX: This should probably be done after we have received all data
  }
  else 
  { 
    dprint(L_MSG|L_MGT, "Received MGT: same version, no update (%d).", newVersion); 
    return;
  }
    
  eitPids.clear();  
  ettEIDs.clear();  
  ettPids.clear();
      
  for (u8 k = 0; k < mgt->NumberOfTables(); k++)
  {
    const Table* t = mgt->GetTable(k);
        
    if (t->tid == 0xCC) // ETT 
    { 
      if (t->table_type == 0x0004) { // Channel ETT
        dprint(L_MGT, "MGT: Found channel ETT PID");
        // Usually provides a short description, not very useful.
      }  
      else { // Event ETT 
        dprint(L_MGT, "MGT: Found ETT PID: %d", t->pid);
        ettPids.push_back(t->pid); // Save these for after we have the EITs
      }  
    }  
    else if (t->tid == 0xCB) // EIT
    {
      dprint(L_MGT, "MGT: Found EIT PID: %d", t->pid);
          
      for (size_t i = 0; i < channelSIDs.size(); i++)
      {
        u32 v = (((u32) channelSIDs[i]) << 16) | t->pid;
        eitPids.push_back(v);
        Add(t->pid, t->tid);
      }
    }
    else
      dprint(L_MGT, "MGT: Unhandled table id 0x%02X", t->tid);
  }
}


//----------------------------------------------------------------------------

void cATSCFilter::ProcessVCT(const uint8_t* data)
{
  dprint(L_VCT, "Received VCT.");

  VCT vct(data);
  vdrInterface.AddChannels(vct);
    
  for (u32 i=0; i<vct.NumberOfChannels(); i++) 
  {
    channelSIDs.push_back( vct.GetChannel(i)->Sid() );
  }
}


//----------------------------------------------------------------------------

void cATSCFilter::ProcessEIT(const uint8_t* data, uint16_t Pid)
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
      dprint(L_EIT, "Received EIT (SID: %d PID: %d) [Already seen]", sid, Pid );
    else  
      dprint(L_EIT, "Received EIT not referred to in MGT (SID: %d PID: %d)", sid, Pid );
  }  
  else // Add events to schedule 
  {
    dprint(L_EIT, "Received EIT (SID: %d PID: %d) [%d left]", sid, Pid , eitPids.size() );
    eitPids.erase(itr);
    Del(Pid, 0xCB);
      
    EIT eit(data);
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
    dprint(L_MSG|L_EIT, "Received all EITs.");

    // Now start looking for ETTs
    for(std::list<uint16_t>::iterator i = ettPids.begin(); i != ettPids.end(); i++) {
      Add(*i, 0xCC);
    } 
  } 
}


//----------------------------------------------------------------------------

void cATSCFilter::ProcessETT(const uint8_t* data)
{
  u16 eid = ETT::ExtractEventID(data);
    
  std::list<uint16_t>::iterator itr;
  for(itr = ettEIDs.begin(); itr != ettEIDs.end() && eid != *itr; itr++) { /* do nothing */ }
    
  if (itr == ettEIDs.end()) 
  {
    dprint(L_ETT, "Unexpected ETT (EID: %d)", eid);
  }
  else
  {
    dprint(L_ETT, "Received ETT (EID: %d)", eid);
    ettEIDs.erase(itr);
    // We cannot Del(Pid, Tid) because we do not know how many ETTs 
    // we will get per PID. Or maybe there is a way to know this...
    ETT ett(data);
    vdrInterface.AddDescription(ett);
  }
    
  if (ettEIDs.size() == 0) 
  {
    dprint(L_MSG|L_ETT, "Received all ETTs.");
    dprint(L_MSG, "Got all event information for this transport stream.");
      
    // Stop looking for ETTs
    for(std::list<uint16_t>::iterator i = ettPids.begin(); i != ettPids.end(); i++) {
      Del(*i, 0xCC);
    }
  }   
}


//----------------------------------------------------------------------------

int cATSCFilter::GetMGTVersion(void) const
{
  std::map<int,uint8_t>::const_iterator itr = MGTVersions.find( currentChannel->Tid() );
  if (itr == MGTVersions.end()) // Key not found
    return -1;

  return itr->second;
}


//----------------------------------------------------------------------------

void cATSCFilter::SetMGTVersion(uint8_t version)
{
  MGTVersions[currentChannel->Tid()] = version;
}


//////////////////////////////////////////////////////////////////////////////


