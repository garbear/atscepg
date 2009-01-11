#ifndef __CHANNELID_H
#define __CHANNELID_H

#include <string>

#include "ATSCTools.h"


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
	
  Channel(u16 ts_id, u16 s_id, std::string s="") { transport_stream_id = ts_id; source_id = s_id; short_name=s;}
  
/*   // Make this class STL "safe"
  
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


struct PTS
{
	u16 pid;  u8 tid;  u16 sid; u16 eid;
	
	PTS(u16 p, u8 t, u16 s, u16 e) { pid=p; tid=t; sid=s; eid=e; }
	
	bool operator== (const PTS& arg) const 
  { 
    return pid == arg.pid && tid == arg.tid && sid == arg.sid && eid == arg.eid;
  }
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


#endif //__CHANNELID_H
