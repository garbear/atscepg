#ifndef __ATSCTABLES_H
#define __ATSCTABLES_H

#include <vector>
#include <string>

#include "ATSCTools.h"
#include "ATSCDescriptors.h"
#include "structs.h"

//////////////////////////////////////////////////////////////////////////////


class PSIPTable
{
public:
	PSIPTable(const u8* data) { update(data); }
	PSIPTable(void) { }
  virtual ~PSIPTable() { }
	virtual void print(void) const;
	virtual void update(const u8* data);
	u8 getVersion(void) { return version_number; }
	
	static u8 extractVersion(const u8* data) { return (data[5] >> 1) & 0x1F; }
	 
protected:
  u8  table_id;
  //  section_syntax_indicator ‘1’
  //  private_indicator        ‘1’
  //  reserved                 ‘11’
  u16 section_length;
  u16 table_id_extension;
  //  reserved                 ‘11’
  u8  version_number;
  u1  current_next_indicator;
  u8  section_number;
  u8  last_section_number;
  u8  protocol_version;
  // PSIP_table_data()
  u32 CRC_32;

	//TODO: Descriptors ?
};



//////////////////////////////////////////////////////////////////////////////


class MGT : public PSIPTable
{
public:
	MGT(const u8* data);
  virtual ~MGT() { }	
  void print(void) const;
  u32 getNumTables(void) const { return tables.size(); }
  Table getTable(int i) const  { return tables[i]; }
  void update(const u8* data);
  
private:
  inline void parse(const u8* data);
	std::vector<Table> tables;
};


//////////////////////////////////////////////////////////////////////////////


// Seconds between start of GPS time and the start of UNIX time.
#define	 secs_Between_1Jan1970_6Jan1980	 315982800

class STT : public PSIPTable
{
public:
	STT(const u8* data);
	virtual ~STT() { }
	time_t getTime(void) const;
	void print(void) const;
	void update(const u8* data);
	time_t UTCtoLocal(time_t utc) const;
	
private:
  inline void parse(const u8* data);
	u32 system_time;
	u8  GPS_UTC_offset;
	u16 daylight_savings;	
};


//////////////////////////////////////////////////////////////////////////////


class EIT : public PSIPTable
{
public:
	EIT(const u8* data);
  virtual ~EIT() { }	
  //void print(void);
  u32 getNumEvents(void) const { return events.size(); }
  Event getEvent(int i)  const { return events[i]; }
  u16 getSourceID(void)  const { return source_id; }
  
private:
	u16 source_id;
	std::vector<Event> events;
};


//////////////////////////////////////////////////////////////////////////////


class VCT : public PSIPTable
{
public:
	VCT(const u8* data);
  virtual ~VCT() { }	
  u32 getNumChannels(void)  const { return channels.size(); }
  Channel getChannel(int i) const { return channels[i]; }
  u16 getTID(void) const { return transport_stream_id; }
  
private:
	u16 transport_stream_id;
	std::vector<Channel> channels;
};


//////////////////////////////////////////////////////////////////////////////


class RTT : public PSIPTable
{
public:
	RTT(const u8* data);
  virtual ~RTT() { }	

private:
	
};


//////////////////////////////////////////////////////////////////////////////


class ETT : public PSIPTable, public MultipleStringStructure
{
public:
	ETT(const u8* data);
  virtual ~ETT() { }	
	u16 getSourceID(void) const {return source_id; }
	u16 getEventID(void)  const {return event_id; }
	
private:
  u16 source_id;
	u16 event_id;
};


//////////////////////////////////////////////////////////////////////////////


#endif // __ATSCTABLES_H
