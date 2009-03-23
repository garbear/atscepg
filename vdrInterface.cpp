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

#include <time.h>
#include <string>

#include  <vdr/epg.h>
#include  <vdr/channels.h>

#include "vdrInterface.h"
#include "filter.h"
#include "config.h"


//////////////////////////////////////////////////////////////////////////////


bool VDRInterface::AddEventsToSchedule(const EIT& eit)
{
	cChannel* c = GetChannel( eit.SourceID() , eit.TableID() );

	if (c)
	{
		bool modified = false;
		
    cSchedulesLock SchedulesLock;
    const cSchedules* Schedules = cSchedules::Schedules(SchedulesLock);
  
	  cSchedule* s = (cSchedule*) Schedules->GetSchedule(c, true);

	  for (u32 i=0; i<eit.NumberOfEvents(); i++)
	  {
	  	const Event* e = eit.GetEvent(i);

	    // Check if event already exit
	    cEvent* pEvent = (cEvent*) s->GetEvent(e->event_id, GPStoLocal(e->start_time) );
	    if (!pEvent)
	    {
	      dprint(L_VDR, "New event: id %d (sid: %d, tid: %d, ver: %d)", e->event_id, eit.SourceID() , eit.TableID(), e->version_number);
	      s->AddEvent( CreateVDREvent(e) );
	      modified = true;
	    } 
	    else
	    {
	      dprint(L_VDR, "Old event: id %d (sid: %d, tid: %d, ver: %d)", e->event_id, eit.SourceID() , eit.TableID(), e->version_number);
	      dprint(L_VDR, "      was: id %d (sid: ?, tid: %d, ver: %d)", pEvent->EventID(), pEvent->TableID(), pEvent->Version());
	      pEvent->SetSeen();
	      
	      if (pEvent->Version() < e->version_number)
	      {
	        dprint(L_VDR, "           new version!");
	        ToVDREvent(e, pEvent, true);
	        modified = true;  
	      }
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

void VDRInterface::AddChannels(const VCT& vct)
{
	currentTID = vct.TID(); //XXX: Still need currentTID???
	
	for (u8 i=0; i<vct.NumberOfChannels(); i++)
	{
		const AtscChannel* ch = vct.GetChannel(i); 
    
    cChannel* c = GetChannel(ch->Sid(), vct.TableID());
    
    if (c) {
			//TODO: Add/Update channels
    }
    else
    {
    	//fprintf(stderr, "\n[ATSC] Does your channels.conf have correct values?");
    	//DisplayChannelInfo(ch, vct.TableID());
    }
	}
}

//----------------------------------------------------------------------------

bool VDRInterface::AddDescription(const ETT& ett)
{
	u16 eid = ett.EventID();

	cChannel* c = GetChannel( ett.SourceID() , ett.TableID() );

	if (c)
	{
		//TODO: Other languages...
		std::string desc = ett.GetString(0);
		
		//std::string desc = "";
		//for (u32 i=0; i<ett.getNumStrings(); i++)
		//  desc += ett.getString(i);
		
	  if (eid) // Event ETM
	  {
			cSchedulesLock SchedulesLock;
      const cSchedules* Schedules = cSchedules::Schedules(SchedulesLock);
  
	    cSchedule* s = (cSchedule*) Schedules->GetSchedule(c, true);
	    
	    cEvent* e = (cEvent*) s->GetEvent( eid );
	    if (e) 
	    {	    	
	      e->SetDescription( desc.c_str() );
      }
      else return false;
	  }
	  else // Channel ETM
	  {
			dprint(L_DBG, "Got Channel ETM");
	  }
	}
	else return false;
	
	return true;
}

//----------------------------------------------------------------------------

cChannel* VDRInterface::GetChannel(u16 source_id, u8 table_id) const
{
  u32 source = (table_id == 0xC9) ? 0x4000 : 0xC000;
	tChannelID channelIDSearch(source, 0x00, currentTID, source_id);
   
  return Channels.GetByChannelID(channelIDSearch, true, true);
}

//----------------------------------------------------------------------------

void VDRInterface::UpdateSTT(const u8* data)
{
	if (!stt) 
	  stt = new STT(data);
	else
	  stt->Update(data);
}

//----------------------------------------------------------------------------
#define	 secs_Between_1Jan1970_6Jan1980	 315982800

time_t VDRInterface::GPStoLocal(time_t gps) const
{
  gps += secs_Between_1Jan1970_6Jan1980;
  // time_t localTime = timegm(localtime(&gps));

  time_t now = time(NULL);
  time_t utc = timelocal(gmtime(&now));

  time_t localTime = gps - (utc - now);
  localTime -= localTime % 60;

  return localTime;
}

//----------------------------------------------------------------------------

cEvent* VDRInterface::CreateVDREvent(const Event* event) const
{
	cEvent* vdrEvent = new cEvent(event->event_id);
	
  ToVDREvent(event, vdrEvent, false);
    
  return vdrEvent; 
}


//----------------------------------------------------------------------------

void VDRInterface::ToVDREvent(const Event* event, cEvent* vdrEvent, bool setId) const
{
  if (!event || !vdrEvent) return;
  
  if (setId)
    vdrEvent->SetEventID(event->event_id);

  vdrEvent->SetStartTime( GPStoLocal(event->start_time) );
  
  vdrEvent->SetDuration(event->length_in_seconds);
  vdrEvent->SetTitle( event->TitleText() );
  vdrEvent->SetVersion(event->version_number);
  vdrEvent->SetTableID(event->table_id);
  
  if (event->ETM_location == 0x00) // There is no description for this event
    vdrEvent->SetDescription("No description provided for this event.");
}


//----------------------------------------------------------------------------

void VDRInterface::DisplayChannelInfo(const AtscChannel* ch, u8 table_id) const
{ //TODO: Update for VDR 1.7.x. Do we really need this anyway?
/*
  char c = (table_id == 0xC9) ? 'C' : 'T';
  int freq = cATSCFilter::Frequency();
  fprintf(stderr, "\n%s:%d:M8:%c:0:", ch->Name(), freq, c);

  if (ch->PCR_PID && ch->PCR_PID != ch->vPID)
    fprintf(stderr, "%d+%d:", ch->vPID, ch->PCR_PID);
  else
    fprintf(stderr, "%d:", ch->vPID);
    
  fprintf(stderr, "0;%d:0:0:%d:0:%d:0\n\n", ch->aPID, ch->source_id, currentTID);
  */ 
}


//////////////////////////////////////////////////////////////////////////////
