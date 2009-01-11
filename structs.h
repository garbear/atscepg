#ifndef __ATSC_STRUCTS_H
#define __ATSC_STRUCTS_H

#include <string>

#include "tools.h"


//////////////////////////////////////////////////////////////////////////////


struct Table {
  Table(u16 p, u8 t, u16 tt) { pid = p; tid = t; table_type = tt;}
	u16 pid;
	u8  tid;
	u16 table_type;
};


//////////////////////////////////////////////////////////////////////////////


struct Event
{
  u16 event_id;
  u32 start_time;
  u32 length_in_seconds;
  std::string title_text;
  u8 version_number;
  u8 table_id;
  u8 ETM_location;
  
 /* 
  Event(const Event& arg)
  {
    event_id = arg.event_id;                   start_time = arg.start_time;
    length_in_seconds = arg.length_in_seconds; title_text = arg.title_text;
  }
 
  const Event& operator= (const Event& arg)
  {
    event_id = arg.event_id;                   start_time = arg.start_time;
    length_in_seconds = arg.length_in_seconds; title_text = arg.title_text;  
  	return *this;  
  } 
	*/ 
};


//////////////////////////////////////////////////////////////////////////////


struct Channel
{ 
  u16 transport_stream_id;
  u16 source_id;
  std::string short_name;

	u16 aPID;
	u16 vPID;
	u16 PCR_PID;
	
	u16 majorChannelNumber;
	u16 minorChannelNumber;
		
  Channel(u16 ts_id, u16 s_id, std::string s="", u16 ap = 0, u16 vp = 0, u16 pp = 0) 
  { 
    transport_stream_id = ts_id; 
    source_id = s_id; 
    short_name=s;
    aPID = ap;
    vPID = vp;
    PCR_PID = 0; 
    majorChannelNumber = minorChannelNumber = 0;
  }
  


/*   // Make this class STL "safe" 
//XXX: these methods are no longer valid due to new data, update before use!
  Channel(const Channel& arg) 
  {
  	transport_stream_id = arg.transport_stream_id;
  	source_id = arg.source_id;
  	short_name = arg.short_name;
  }
  
  const Channel& operator= (const Channel& arg)
  {
  	transport_stream_id = arg.transport_stream_id;
  	source_id = arg.source_id;
  	short_name = arg.short_name;
  	return *this;
  }
     
  bool operator== (const Channel& arg) const 
  { 
    return transport_stream_id == arg.transport_stream_id && source_id == arg.source_id;
  }
  
  bool operator!= (const Channel& arg) const
  {
  	return transport_stream_id != arg.transport_stream_id || source_id != arg.source_id;
  } 
  
  bool operator< (const Channel& arg) const
  {
  	return (transport_stream_id < arg.transport_stream_id) || 
  	       ((transport_stream_id == arg.transport_stream_id) && (source_id < arg.source_id));
  }
  
  bool operator> (const Channel& arg) const 
  {
  	return (transport_stream_id > arg.transport_stream_id) || 
  	       ((transport_stream_id == arg.transport_stream_id) && (source_id > arg.source_id));
  }
*/ 
};


//////////////////////////////////////////////////////////////////////////////


struct Stream
{
	Stream(u8 s, u16 p, u32 l) { stream_type=s; elementary_PID=p; ISO_639_language_code=l; }
	
	u8  stream_type;
  u16 elementary_PID;
	u32 ISO_639_language_code;
};

//////////////////////////////////////////////////////////////////////////////


#endif //__ATSC_STRUCTS_H
