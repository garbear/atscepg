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

#include <time.h>
#include <string>

#include "vdrInterface.h"


//////////////////////////////////////////////////////////////////////////////


bool VDRInterface::AddEvents(cChannel* channel, const EIT& eit)
{
  if (!channel)
    return false;

  bool modified = false;
    
  cSchedulesLock SchedulesLock;
  const cSchedules* Schedules = cSchedules::Schedules(SchedulesLock);
  
  cSchedule* s = (cSchedule*) Schedules->GetSchedule(channel, true);

  for (u32 i=0; i<eit.NumberOfEvents(); i++)
  {
    const Event* e = eit.GetEvent(i);

    // Check if event already exit
    cEvent* pEvent = (cEvent*) s->GetEvent(e->event_id, GPStoLocal(e->start_time));
    if (!pEvent) {
      dprint(L_VDR, "New event: id %d (sid: %d, tid: %d, ver: %d)", e->event_id, eit.SourceID() , eit.TableID(), e->version_number);
      s->AddEvent( CreateVDREvent(e) );
      modified = true;
    } 
    else {
      dprint(L_VDR, "Old event: id %d (sid: %d, tid: %d, ver: %d)", e->event_id, eit.SourceID() , eit.TableID(), e->version_number);
      dprint(L_VDR, "      was: id %d (sid: ?, tid: %d, ver: %d)", pEvent->EventID(), pEvent->TableID(), pEvent->Version());
        
      pEvent->SetEventID(e->event_id);
      pEvent->SetSeen();
        
      if (pEvent->Version() != e->version_number) {
        dprint(L_VDR, "           new version!");
        ToVDREvent(e, pEvent);
        modified = true;  
      }
    }
  }
    
  if (modified) 
  {
    s->Sort();
    Schedules->SetModified(s);  
  } 

  return true; 
}


//----------------------------------------------------------------------------

bool VDRInterface::AddDescription(cChannel* channel, const ETT& ett)
{
  if (!channel)
    return false;

  //TODO: Other languages...
  std::string desc = ett.GetString(0);
  //std::string desc = "";
  //for (u32 i=0; i<ett.getNumStrings(); i++)
  //  desc += ett.getString(i);
  
  uint16_t eid = ett.EventID();
  if (eid) // Event ETM
  {
    cSchedulesLock SchedulesLock;
    const cSchedules* Schedules = cSchedules::Schedules(SchedulesLock);
    cSchedule* s = (cSchedule*) Schedules->GetSchedule(channel, true);
      
    cEvent* event = (cEvent*) s->GetEvent(eid);
    if (event)         
      event->SetDescription( desc.c_str() );
    else
      return false;
  }
  else // Channel ETM
    dprint(L_DBG, "Got Channel ETM, ignored.");

  return true;
}


//----------------------------------------------------------------------------

static time_t UtcOffset(void)
{
  struct tm gps = { 0,0,0,6,0,80,0,0,0 }; // Jan 6, 1980 00:00:00 UTC
  time_t t = mktime(&gps);
  return (t + gps.tm_gmtoff);
}


//----------------------------------------------------------------------------

time_t VDRInterface::GPStoLocal(time_t gps)
{
  static time_t utc_offset = UtcOffset();
  gps += utc_offset;
  gps -= gps%60;
    
  return gps;
}


//----------------------------------------------------------------------------

cEvent* VDRInterface::CreateVDREvent(const Event* event)
{
  cEvent* vdrEvent = new cEvent(event->event_id);
  ToVDREvent(event, vdrEvent);
  return vdrEvent;
}


//----------------------------------------------------------------------------

void VDRInterface::ToVDREvent(const Event* event, cEvent* vdrEvent)
{
  if (!event || !vdrEvent) return;
  
  vdrEvent->SetEventID(event->event_id);
  vdrEvent->SetStartTime( GPStoLocal(event->start_time) );
  vdrEvent->SetDuration(event->length_in_seconds);
  vdrEvent->SetTitle( event->TitleText() );
  vdrEvent->SetVersion(event->version_number);
  vdrEvent->SetTableID(event->table_id);
  
  if (event->ETM_location == 0x00) // There is no description for this event
    vdrEvent->SetDescription("No description provided for this event.");
}


//////////////////////////////////////////////////////////////////////////////
