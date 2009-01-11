#include <vdr/plugin.h>
#include <vdr/filter.h>
#include  <vdr/device.h>

#include "ATSCFilter.h"
#include "ATSCTables.h"
#include "ATSCTools.h"


//////////////////////////////////////////////////////////////////////////////


cATSCFilter::cATSCFilter()
{
  DEBUG_MSG("ATSCFilter Created");
  
  mgt = NULL;

  gotVCT = false;
  gotRRT = false;
  
  lastScanMGT = 0;
  lastScanSTT = 0;
   
  attachedDevice = NULL;
  
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
	if (mgt) delete mgt;
}

//----------------------------------------------------------------------------

void cATSCFilter::Attach(const cDevice* Device)
{
  if (!attachedDevice) 
  {
    attachedDevice = (cDevice*) (Device ? Device : getDevice());
    if (attachedDevice) 
    {
      attachedDevice->AttachFilter(this);
      DEBUG_MSG("ATSCFilter Attached.");
    }
  }
  
}

//----------------------------------------------------------------------------

void cATSCFilter::Detach(const cDevice* Device)
{
  if (attachedDevice) 
  {
    if (Device == attachedDevice)
    {
      attachedDevice->Detach(this);
      attachedDevice = NULL;
      DEBUG_MSG("ATSCFilter Detached.");
    }
  }
  
}
  
//----------------------------------------------------------------------------

void cATSCFilter::SetStatus(bool On)
{ 
	if (On) 
	{
    gotVCT = false;
    gotRRT = false;
    
    lastScanMGT = 0;
    lastScanSTT = 0;
    
    channelSIDs.clear();
    pts.clear();
    eitPids.clear();
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
  	DEBUG_MSG("Received MGT.");
  	
  	bool updateTables = false;

  	if (!mgt) {
  	  mgt = new MGT(Data);
  	  updateTables = true;
  	}

  	// Do we have a newer version?
  	if ( PSIPTable::extractVersion(Data) > mgt->getVersion() )
  	{
  	  DEBUG_MSG("New MGT Version: updating event info.");
  		mgt->update(Data);
  		updateTables = true;
  	}
  	
  	if (updateTables)
  	{
  	  eitPids.clear();  ettPids.clear();
  	  for (u8 k=0; k<mgt->getNumTables(); k++)
  	  {
  		  Table t = mgt->getTable(k);
  		  
  		  if (t.tid == 0xCC) 
  		  { 
  		  	if (t.table_type == 0x0004) // Channel ETT
  		  	  ; //TODO: Use Channel ETT.
  		  	else // Event ETT 
  		      ettPids.push_back(t.pid);
  		  }  
  		  else if (t.tid == 0xCB) // EIT
  		  {
  		    eitPids.push_back(t.pid);
  		    
          for (u32 i=0; i<channelSIDs.size(); i++)
  		    {
  	        addE_T(t.pid, t.tid, channelSIDs[i]);
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
  	DEBUG_MSG("Received VCT.");
  	
  	VCT vct(Data);
  	vdrInterface.addChannels(vct);
  	
  	for (u32 i=0; i<vct.getNumChannels(); i++)
  		channelSIDs.push_back( vct.getChannel(i).source_id );

  	gotVCT = true;
	}	
	

	//~~~ RRT: Rating Region Table ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	else if (Tid == 0xCA) 			
	{
		//if (now - lastScanRRT <= SCAN_DELAY)  return;
		if (gotRRT) return;
    DEBUG_MSG("Received RRT: Not yet implemented.");
  	
  	//lastScanRRT = time(NULL);
  	gotRRT = true;
	}	
	

	//~~~ STT: System Time Table ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	else if (Tid ==  0xCD)		
	{
	  if (now - lastScanSTT <= STT_SCAN_DELAY) return;
  	DEBUG_MSG("Received STT.");
  	vdrInterface.updateSTT(Data);	
  	
  	lastScanSTT = time(NULL);	
	}
	

	//~~~ EIT: Event Information Table ~~~~~~~~~~~~~~~~~~~~~~~~~
	else if (Tid ==  0xCB)
	{ 			
    DEBUG_MSG("Received EIT.");
  	
  	EIT eit(Data);

    if ( vdrInterface.addEventsToSchedule(eit) ) 
    {
      if ( delE_T(Pid, Tid, eit.getSourceID() ) )
      {
        // Now look for ETTs for these events
        for (u32 i=0; i<eit.getNumEvents(); i++)
        {
          Event e = eit.getEvent(i);
          if (e.ETM_location != 0x00)
            addE_T(getETTPidFromEIT(Pid), 0xCC, eit.getSourceID(), e.event_id );
        }
      }
    }
	}	
	

	//~~~ ETT: Extended Text Table ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	else if (Tid ==  0xCC)			
	{
  	DEBUG_MSG("Received ETT.");
  	
  	ETT ett(Data);
  	if ( vdrInterface.addDescription(ett) )
  	{
  	  delE_T(Pid, Tid, ett.getSourceID(), ett.getEventID() );
  	}
	}	
	

	//~~~ DCC ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	else if (Tid ==  0xD3)			
	{
	  DEBUG_MSG("Received DCC: Not yet implemented.");
	}
	

	//~~~ DCCSCT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	else if (Tid ==  0xD4)			
	{
	  DEBUG_MSG("Received DCCSCT: Not yet implemented.");
	}
	
}

//----------------------------------------------------------------------------

void cATSCFilter::addE_T(u16 pid, u8 tid, u16 sid, u16 eid)
{
	PTS nPts(pid, tid, sid, eid);
	
	for (u32 i=0; i<pts.size(); i++)
	  if (pts[i] == nPts) return;
	 
	pts.push_back(nPts);  
	cFilter::Add(pid, tid);
}

//----------------------------------------------------------------------------

bool cATSCFilter::delE_T(u16 pid, u8 tid, u16 sid, u16 eid)
{
	PTS nPts(pid, tid, sid, eid);
	
	std::vector<PTS>::iterator itr = pts.begin();

  for (; itr != pts.end(); itr++)
  {
	  if (*itr == nPts)
	  {
	  	cFilter::Del(pid, tid);
	  	pts.erase(itr);
	  	if (pts.empty() ) { DEBUG_MSG("Got all epg info for this transport stream."); }
	  	return true;
	  }
	}
	
	return false;
}
	
//----------------------------------------------------------------------------

u16 cATSCFilter::getETTPidFromEIT(u16 eit_pid)
{
	for (u32 i=0; i<eitPids.size(); i++)
	{
		if (eitPids[i] == eit_pid && i<ettPids.size() )
		  return ettPids[i];
	}
	
	return 0;
}

//----------------------------------------------------------------------------

cDevice* cATSCFilter::getDevice(void)
{
  cChannel* c = Channels.GetByNumber(cDevice::CurrentChannel());
  if (c) {
    return cDevice::GetDevice(c, 0);
  }
  else
  {
    return cDevice::ActualDevice();
  }
}


//////////////////////////////////////////////////////////////////////////////
