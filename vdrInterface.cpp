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


VDRInterface::VDRInterface()
{
  stt = NULL;
  currentTID = 0;
}


//----------------------------------------------------------------------------

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
        
        pEvent->SetEventID(e->event_id);
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
  SidTranslation.clear();
  
  for (u8 i=0; i<vct.NumberOfChannels(); i++)
  {
    const AtscChannel* ch = vct.GetChannel(i);
    if (GetVDRSid(ch->Sid()) == -1)
      SidTranslation[ch->Sid()] = ch->ProgramNumber();
    else
      dprint(L_ERR, "Channel has non unique SID");
/*
    cChannel* c = GetChannel(ch->ProgramNumber(), vct.TableID());
    
    if (c) {
      //TODO: Add/Update channels
    }
    else
    {

    }
*/
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
  tChannelID channelIDSearch(source, 0x00, currentTID, GetVDRSid(source_id));
   
  return Channels.GetByChannelID(channelIDSearch, true, true);
}


//----------------------------------------------------------------------------

int VDRInterface::GetVDRSid(u16 sourceId) const
{
  std::map<uint16_t,uint16_t>::const_iterator itr = SidTranslation.find(sourceId);
  if (itr == SidTranslation.end()) // Key not found
    return -1;

  return itr->second;
}


//----------------------------------------------------------------------------

time_t UtcOffset(void)
{
  struct tm gps = { 0,0,0,6,0,80,0,0,0 }; // Jan 6, 1980 00:00:00 UTC
  time_t t = mktime(&gps);
  return (t + gps.tm_gmtoff);
}


//----------------------------------------------------------------------------

void VDRInterface::UpdateSTT(const u8* data, int length)
{
  if (!stt) 
    stt = new STT(data, length);
  else
    stt->Update(data, length);
  
  // time_t local = stt->GetGPSTime() + UtcOffset();
}


//----------------------------------------------------------------------------

time_t VDRInterface::GPStoLocal(time_t gps) const
{
  static time_t utc_offset = UtcOffset();
  gps += utc_offset;
  gps -= gps%60;
    
  return gps;
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


//////////////////////////////////////////////////////////////////////////////
