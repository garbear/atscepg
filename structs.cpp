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


Channel::Channel(void)
{
  transport_stream_id = 0;
  source_id           = 0;
  aPID                = 0;
  vPID                = 0;
  PCR_PID             = 0;
  majorChannelNumber  = 0;
  minorChannelNumber  = 0;
  short_name          = NULL;
}


//----------------------------------------------------------------------------

Channel::~Channel(void)
{
  free(short_name);
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
  short_name          = strdup(arg.short_name);
}


//----------------------------------------------------------------------------
 
void Channel::SetName(const char* text)
{
  free(short_name);
  short_name = strdup(text);
}

  
//////////////////////////////////////////////////////////////////////////////  
