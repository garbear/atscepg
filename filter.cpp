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
	Set(0x1FFB, 0xCA); // RRT
	Set(0x1FFB, 0xCD); // SST

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


void cATSCFilter::Process(u_short Pid, u_char Tid, const u_char *Data, int Length)
{  
	time_t now = time(NULL);
	
	//~~~ MGT: Master Guide Table ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	if (Tid == 0xC7) 
	{
		if (!gotVCT || now - lastScanMGT <= MGT_SCAN_DELAY) return;
  	bool updateTables = false;

  	// Do we have a newer version?
    uint8_t newVersion = PSIPTable::extractVersion(Data);

    if ( ((int)newVersion) != getMGTVersion())
  	{
      dprint(L_MSG, "Received MGT: new version, updating (%d -> %d).", getMGTVersion(), newVersion);
      if (!mgt)
  	    mgt = new MGT(Data);
      else
  		  mgt->update(Data);

      setMGTVersion(newVersion);
  		updateTables = true;
  	}
    else { dprint(L_MSG, "Received MGT: same version, no update (%d).", newVersion); }
  	
  	if (updateTables)
  	{
  	  eitPids.clear();  ettEIDs.clear();  ettPids.clear();
  	  
  	  for (u8 k=0; k<mgt->getNumTables(); k++)
  	  {
  		  Table t = mgt->getTable(k);
  		  
  		  if (t.tid == 0xCC) 
  		  { 
  		  	if (t.table_type == 0x0004) { // Channel ETT
  		  	  dprint(L_DBG, "MGT: Found channel ETT PID"); //TODO: Use Channel ETT.
  		  	}  
  		  	else { // Event ETT 
  		      dprint(L_DBG, "MGT: Found ETT PID: %d", t.pid);
  		      ettPids.push_back(t.pid); // Save these for after we have the EITs
  		      //Add(t.pid, t.tid);
  		    }  
  		  }  
  		  else if (t.tid == 0xCB) // EIT
  		  {
  		    dprint(L_DBG, "MGT: Found EIT PID: %d", t.pid);
  		    
          for (u32 i=0; i<channelSIDs.size(); i++)
  		    {
  	        u32 v = (((u32) channelSIDs[i]) << 16) | t.pid;
  	        eitPids.push_back(v);
  	        Add(t.pid, t.tid);
  	      }
  	    }
  	  }
  	}
  	  
    lastScanMGT = time(NULL);
  	
	}


	//~~~ T/C VCT: Terrestrial/Cable Virtual Channel Table ~~~~~
	else if (Tid == 0xC8 || Tid == 0xC9)	
	{	
  	if (gotVCT) return;
  	dprint(L_MSG, "Received VCT.");

  	VCT vct(Data);
  	vdrInterface.addChannels(vct);
  	
  	for (u32 i=0; i<vct.getNumChannels(); i++) 
  	{
  		channelSIDs.push_back( vct.getChannel(i).source_id );
    }

  	gotVCT = true;
	}	
	

	//~~~ RRT: Rating Region Table ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	else if (Tid == 0xCA) 			
	{
		//if (now - lastScanRRT <= SCAN_DELAY)  return;
		if (gotRRT) return;
  	dprint(L_DBG, "Received RRT: Not yet implemented.");
  	//lastScanRRT = time(NULL);
  	//RRT rrt(Data);
  	gotRRT = true;
	}	
	

	//~~~ STT: System Time Table ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	else if (Tid ==  0xCD)		
	{
	  if (now - lastScanSTT <= STT_SCAN_DELAY) return;
  	dprint(L_MSG, "Received STT.");
  	vdrInterface.updateSTT(Data);	
  	
  	lastScanSTT = time(NULL);	
	}
	

	//~~~ EIT: Event Information Table ~~~~~~~~~~~~~~~~~~~~~~~~~
	else if (Tid ==  0xCB)
	{ 		
    u16 sid = EIT::extractSourceID(Data);
    u32 val = (((u32) sid) << 16) | Pid;
  	            
    std::list<uint32_t>::iterator itr;
    for(itr = eitPids.begin(); itr != eitPids.end() && val != *itr; itr++) { /* do nothing */ } 
    
    if (itr == eitPids.end()) // We have already seen or are not expecting this EIT
    {
      bool found = false;
      for (int i=0; i<channelSIDs.size() && !found; i++) {
        if (channelSIDs[i] == sid)
          found = true;
      }
      if (found)
        dprint(L_DBG, "Received EIT (SID: %d PID: %d) [Already seen]", sid, Pid );
      else  
        dprint(L_DBG, "Received EIT not referred to in MGT (SID: %d PID: %d)", sid, Pid );
    }  
    else // Add events to schedule 
    {
      dprint(L_DBG, "Received EIT (SID: %d PID: %d) [%d left]", sid, Pid , eitPids.size() );
      eitPids.erase(itr);
      Del(Pid, 0xCB);
      
      EIT eit(Data);
      vdrInterface.addEventsToSchedule(eit);
      
      // Now look for ETTs for these events
      for (u32 i=0; i<eit.getNumEvents(); i++)
      {
        Event e = eit.getEvent(i);
        if (e.ETM_location == 0x01 || e.ETM_location == 0x02) // There is an ETT for this event
          ettEIDs.push_back(e.event_id);
      }
    }
    
    if (eitPids.size() == 0) {
      dprint(L_MSG, "Received all EITs.");
      
      // Now start looking for ETTs
      for(std::list<uint16_t>::iterator i = ettPids.begin(); i != ettPids.end(); i++) {
        Add(*i, 0xCC);
      } 
    }  

	}	
	

	//~~~ ETT: Extended Text Table ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	else if (Tid ==  0xCC)			
	{  	
    u16 eid = ETT::extractEventID(Data);
    
    std::list<uint16_t>::iterator itr;
    for(itr = ettEIDs.begin(); itr != ettEIDs.end() && eid != *itr; itr++) { }
    
    if (itr == ettEIDs.end()) 
    {
      dprint(L_DBG, "Unexpected ETT (EID: %d)", eid);
    }
    else
    {
      dprint(L_DBG, "Recevied ETT (EID: %d)", eid);
      ettEIDs.erase(itr);
      // We cannot Del(Pid, Tid) because we do not know how many ETTs 
      // we will get per PID. Or maybe there is a way to know this...
      ETT ett(Data);
      vdrInterface.addDescription(ett);
    }
    
    if (ettEIDs.size() == 0) {
      dprint(L_MSG, "Received all ETTs.");
      dprint(L_MSG, "Got all event information for this transport stream.");
      
      // Stop looking for ETTs
      for(std::list<uint16_t>::iterator i = ettPids.begin(); i != ettPids.end(); i++) {
        Del(*i, 0xCC);
      }
    }
    
	}	
	

	//~~~ DCC ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	else if (Tid ==  0xD3)			
	{
	  dprint(L_DBG, "Received DCC: Not yet implemented.");
	}
	

	//~~~ DCCSCT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	else if (Tid ==  0xD4)			
	{
	  dprint(L_DBG, "Received DCCSCT: Not yet implemented.");
	}
	
}


//----------------------------------------------------------------------------

int cATSCFilter::getMGTVersion(void) const
{
  std::map<int,uint8_t>::const_iterator itr = MGTVersions.find( currentChannel->Tid() );
  if (itr == MGTVersions.end()) // Key not found
    return -1;

  return itr->second;
}


//----------------------------------------------------------------------------

void cATSCFilter::setMGTVersion(uint8_t version)
{
  MGTVersions[currentChannel->Tid()] = version;
}


//////////////////////////////////////////////////////////////////////////////


