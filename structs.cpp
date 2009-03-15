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

#include <string.h>
#include <stdlib.h>

#include "structs.h"


//////////////////////////////////////////////////////////////////////////////


Table::Table(void) 
{ 
  pid = 0; 
  tid = 0; 
  table_type = 0;
}


//----------------------------------------------------------------------------

Table::Table(u16 p, u8 t, u16 tt) 
{ 
  pid = p; 
  tid = t; 
  table_type = tt;
}


//----------------------------------------------------------------------------

Table::Table(const Table& arg) 
{ 
  pid = arg.pid; 
  tid = arg.tid; 
  table_type = arg.table_type; 
}


//----------------------------------------------------------------------------

const Table& Table::operator= (const Table& arg) 
{ 
  pid = arg.pid; 
  tid = arg.tid; 
  table_type = arg.table_type; 
  return *this;
}
  

//////////////////////////////////////////////////////////////////////////////


Event::Event(void)
{
  event_id          = 0;
  start_time        = 0;
  length_in_seconds = 0;
  version_number    = 0;
  table_id          = 0;
  ETM_location      = 0;
  title_text        = NULL;
}


//----------------------------------------------------------------------------

Event::~Event(void)
{
  free(title_text);
}


//----------------------------------------------------------------------------

Event::Event(const Event& arg)
{
  event_id          = arg.event_id;
  start_time        = arg.start_time;
  length_in_seconds = arg.length_in_seconds;
  version_number    = arg.version_number;
  table_id          = arg.table_id;
  ETM_location      = arg.ETM_location;
  title_text        = strdup(arg.title_text);
}


//----------------------------------------------------------------------------
  
const Event& Event::operator= (const Event& arg)
{
  event_id          = arg.event_id;
  start_time        = arg.start_time;
  length_in_seconds = arg.length_in_seconds;
  version_number    = arg.version_number;
  table_id          = arg.table_id;
  ETM_location      = arg.ETM_location;
  free(title_text);
  title_text        = strdup(arg.title_text);
  return *this;
}


//----------------------------------------------------------------------------

void Event::SetTitleText(const char* text)
{
  free(title_text);
  title_text = strdup(text);
}


//////////////////////////////////////////////////////////////////////////////


AtscChannel::AtscChannel(void)
{
  majorChannelNumber = 0;
  minorChannelNumber = 0;
}


void AtscChannel::SetPids(int Vpid, int Ppid, int Vtype, int *Dpids, char DLangs[][MAXLANGCODE2])
{
  int Apids[1] = { 0 };
  int Spids[1] = { 0 };
  char ALangs[1][MAXLANGCODE2] = { "" };
  char SLangs[1][MAXLANGCODE2] = { "" };
#if VDRVERSNUM < 10701
  channel.SetPids(Vpid, Ppid, Apids, ALangs, Dpids, DLangs, Spids, SLangs, 0);
#else  
  channel.SetPids(Vpid, Ppid, Vtype, Apids, ALangs, Dpids, DLangs, Spids, SLangs, 0);
#endif
}

#if 0
Channel::Channel(void)
{
  transport_stream_id = 0;
  source_id           = 0;
  aPID                = 0;
  vPID                = 0;
  PCR_PID             = 0;
  majorChannelNumber  = 0;
  minorChannelNumber  = 0;
  short_name          = strdup("");
  long_name           = strdup("");
}


//----------------------------------------------------------------------------

Channel::~Channel(void)
{
  free(short_name);
  free(long_name);
}


//----------------------------------------------------------------------------

Channel::Channel(const Channel& arg)
{
  transport_stream_id = arg.transport_stream_id;
  source_id           = arg.source_id;
  aPID                = arg.aPID;
  vPID                = arg.vPID;
  PCR_PID             = arg.PCR_PID ;
  majorChannelNumber  = arg.majorChannelNumber;
  minorChannelNumber  = arg.minorChannelNumber;
  short_name          = strdup(arg.short_name);
  long_name           = strdup(arg.long_name);
}


//----------------------------------------------------------------------------

const Channel& Channel::operator= (const Channel& arg)
{
  transport_stream_id = arg.transport_stream_id;
  source_id           = arg.source_id;
  aPID                = arg.aPID;
  vPID                = arg.vPID;
  PCR_PID             = arg.PCR_PID ;
  majorChannelNumber  = arg.majorChannelNumber;
  minorChannelNumber  = arg.minorChannelNumber;
  free(short_name);
  free(long_name);
  short_name          = strdup(arg.short_name);
  long_name           = strdup(arg.long_name);
}


//----------------------------------------------------------------------------
 
void Channel::SetShortName(const char* shortName)
{
  free(short_name);
  short_name = strdup(shortName);
}


//----------------------------------------------------------------------------

void Channel::SetLongName(const char* longName) 
{
  free(long_name);
  long_name  = strdup(longName);
}
#endif

//////////////////////////////////////////////////////////////////////////////  
