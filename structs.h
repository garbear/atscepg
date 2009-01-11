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

#ifndef __ATSC_STRUCTS_H
#define __ATSC_STRUCTS_H

#include <string>

#include "tools.h"


//////////////////////////////////////////////////////////////////////////////


struct Table 
{
  Table(void);
  Table(u16 p, u8 t, u16 tt);
  Table(const Table& arg);
  const Table& operator= (const Table& arg);
  
	u16 pid;
	u8  tid;
	u16 table_type;
};


//////////////////////////////////////////////////////////////////////////////


struct Event
{
  Event(void);
 ~Event(void); 
  Event(const Event& arg);
  
  const Event& operator= (const Event& arg);
  
  void SetTitleText(const char* text);
  const char* TitleText(void) const { return title_text; }
  
  u16 event_id;
  u32 start_time;
  u32 length_in_seconds;
  u8 version_number;
  u8 table_id;
  u8 ETM_location;
  
private:
  char* title_text;  
};


//////////////////////////////////////////////////////////////////////////////


struct Channel
{ 
  Channel(void);
 ~Channel(void);
  Channel(const Channel& arg);

  const Channel& operator= (const Channel& arg);
  
  void SetName(const char* text);
  const char* Name(void) const { return short_name; }
  
  u16 transport_stream_id;
  u16 source_id;
	u16 aPID;
	u16 vPID;
	u16 PCR_PID;
	u16 majorChannelNumber;
	u16 minorChannelNumber;
		
private:
  char* short_name;
};


//////////////////////////////////////////////////////////////////////////////


struct Stream
{
  Stream(void) { stream_type = 0; elementary_PID=0; ISO_639_language_code=0; }
	Stream(u8 s, u16 p, u32 l) { stream_type=s; elementary_PID=p; ISO_639_language_code=l; }
	
	u8  stream_type;
  u16 elementary_PID;
	u32 ISO_639_language_code;
};

//////////////////////////////////////////////////////////////////////////////


#endif //__ATSC_STRUCTS_H
