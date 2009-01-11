#include <time.h>
#include <string>

#include  <vdr/epg.h>
#include  <vdr/channels.h>

#include "vdrInterface.h"


//////////////////////////////////////////////////////////////////////////////


bool VDRInterface::addEventsToSchedule(EIT& eit)
{
	if (!stt) return false; 

	cChannel* c = getChannel( eit.getSourceID() );

	if (c)
	{
		bool modified = false;
		
    cSchedulesLock SchedulesLock;
    const cSchedules* Schedules = cSchedules::Schedules(SchedulesLock);
  
	  cSchedule* s = (cSchedule*) Schedules->GetSchedule(c, true);

	  for (u32 i=0; i<eit.getNumEvents(); i++)
	  {
	  	Event* e = &( eit.getEvent(i) );

	    // Check if event already exit
	    cEvent* pEvent = (cEvent*) s->GetEvent(e->event_id/*, e->start_time*/);
	    if (!pEvent)
	    {
	      DEBUG_MSG("Added Event with ID %d", e->event_id );
	      s->AddEvent( createVDREvent(*e) );
	      modified = true;
	    } 
	    else
	    {
	      DEBUG_MSG("Updated Event with ID %d", e->event_id );
	      pEvent->SetSeen();
	      //TODO: Check version info
	    }
    }
    
    if (modified) 
    {
      s->Sort();
      Schedules->SetModified(s);  
    } 
    
  } 
  else return false;


  return true; 
}

//----------------------------------------------------------------------------

void VDRInterface::addChannels(VCT& vct)
{
	currentTID = vct.getTID();
	
	for (u32 i=0; i<vct.getNumChannels(); i++)
	{
		Channel ch = vct.getChannel(i); 
    
    cChannel* c = getChannel(ch.source_id );
    
    if (c)
    {
			//TODO: Add/Update channels
    }
    else
    {
    	fprintf(stderr, "\n");
      DEBUG_MSG("Does your channels.conf have correct values?");
      DEBUG_MSG("Channel: %s should have", ch.short_name.c_str() );
      DEBUG_MSG("TID: %d \t SID: %d\n", currentTID, ch.source_id);
    }
	}
}

//----------------------------------------------------------------------------

bool VDRInterface::addDescription(ETT& ett)
{
	u16 eid = ett.getEventID();
	//u16 sid = ett.getSourceID();
	
	cChannel* c = getChannel( ett.getSourceID() );

	if (c)
	{
		//TODO: Other languages...
		//std::string desc = ett.getString(0);
		
		std::string desc = "";
		for (u32 i=0; i<ett.getNumStrings(); i++)
		  desc += ett.getString(i);
		
	  if (eid) // Event ETM
	  {
			cSchedulesLock SchedulesLock;
      const cSchedules* Schedules = cSchedules::Schedules(SchedulesLock);
  
	    cSchedule* s = (cSchedule*) Schedules->GetSchedule(c, true);
	    
	    cEvent* e = (cEvent*) s->GetEvent( eid );
	    if (e) 
	    {	    	
	      e->SetDescription( desc.c_str() );
        DEBUG_MSG("Added description for event %d", eid);
      }
      else return false;
	  }
	  else // Channel ETM
	  {
			//TODO: Use this as long channel name?
	  }
	}
	else return false;
	
	return true;
}

//----------------------------------------------------------------------------

cChannel* VDRInterface::getChannel(u16 source_id) const
{
	tChannelID channelID_search(0x4000, 0x00, currentTID, source_id);
   
  return Channels.GetByChannelID(channelID_search, true, true, true);
}

//----------------------------------------------------------------------------

void VDRInterface::updateSTT(const u8* data)
{
	if (!stt) 
	  stt = new STT(data);
	else
	  stt->update(data);
}

//----------------------------------------------------------------------------

time_t VDRInterface::GPStoSystem(time_t gps)
{
  //TODO: Remove dependence on STTs to calculate local time
	return 0;
}

//----------------------------------------------------------------------------

cEvent* VDRInterface::createVDREvent(Event& event)
{
	cEvent* vdrEvent = new cEvent(event.event_id);
	
  vdrEvent->SetStartTime( stt->UTCtoLocal( event.start_time ) );
  //vdrEvent->SetStartTime( GPStoSystem( event.start_time ) );
  vdrEvent->SetDuration(event.length_in_seconds);
  vdrEvent->SetTitle( event.title_text.c_str() );
  vdrEvent->SetVersion(event.version_number);
  vdrEvent->SetTableID(event.table_id);
  
  vdrEvent->SetShortText("");
  
  if (event.ETM_location == 0x00) // There is no description for this event
    vdrEvent->SetDescription("No description provided for this event.");
  else
    vdrEvent->SetDescription("");
    
  vdrEvent->SetRunningStatus(0);
    
  return vdrEvent; 
}

//////////////////////////////////////////////////////////////////////////////
