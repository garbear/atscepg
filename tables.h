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

#ifndef __ATSCTABLES_H
#define __ATSCTABLES_H

#include <vector>
#include <string>

#include "tools.h"
#include "descriptors.h"
#include "structs.h"


//////////////////////////////////////////////////////////////////////////////


class PSIPTable
{
public:
  PSIPTable(const u8* data) { Update(data); }
  PSIPTable(void);
  virtual ~PSIPTable();
  
  virtual void Print(void) const;
  virtual void Update(const u8* data);
  bool CheckCRC(void) { return crc_passed; }
  u8 Version(void) const { return version_number; }
  u8 TableID(void) const { return table_id; }
  u32 NumberOfDescriptors(void) const { return descriptors.size(); }
  Descriptor* GetDescriptor(u32 i) const;
  static u8 ExtractVersion(const u8* data) { return (data[5] >> 1) & 0x1F; }
   
protected:
  void AddDescriptors(const u8* data, u16 length);
  
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
  //u32 CRC_32;
  bool crc_passed;
  
  std::vector<Descriptor*> descriptors; //XXX: replace vector with something else!
};



//////////////////////////////////////////////////////////////////////////////


class MGT : public PSIPTable
{
public:
  MGT(const u8* data);
  virtual ~MGT() { delete[] tables; }
    
  virtual void Print(void) const;
  u16 NumberOfTables(void) const { return numberOfTables; }
  const Table* GetTable(int i) const { return (i<numberOfTables) ? &tables[i] : NULL; }
  void Update(const u8* data);
  
private:
  u16 numberOfTables;
  void Parse(const u8* data);
  Table* tables;
};


//////////////////////////////////////////////////////////////////////////////


// Seconds between start of GPS time and the start of UNIX time.
#define   secs_Between_1Jan1970_6Jan1980   315982800

class STT : public PSIPTable
{
public:
  STT(const u8* data);
  //virtual ~STT() { }
  time_t GetTime(void) const;
  void Print(void) const;
  void Update(const u8* data);
  time_t UTCtoLocal(time_t utc) const;
  
private:
  void Parse(const u8* data);
  
  u32 system_time;
  u8  GPS_UTC_offset;
  u16 daylight_savings;  
};


//////////////////////////////////////////////////////////////////////////////


class EIT : public PSIPTable
{
public:
  EIT(const u8* data);
  virtual ~EIT() { delete[] events; }  
  //void print(void);
  u8 NumberOfEvents(void) const { return numberOfEvents; }
  const Event* GetEvent(int i) const { return (i<numberOfEvents) ? &events[i] : NULL; }
  u16 SourceID(void) const { return source_id; }
  
  static u16 ExtractSourceID(const u8* data) { return get_u16( data+3 ); }
  
private:
  u8 numberOfEvents;
  u16 source_id;
  Event* events;
};


//////////////////////////////////////////////////////////////////////////////


class VCT : public PSIPTable
{
public:
  VCT(const u8* data);
  virtual ~VCT();
  
  u8 NumberOfChannels(void)  const { return numberOfChannels; }
  const AtscChannel* GetChannel(int i) const { return (i<numberOfChannels) ? channels[i] : NULL; }
  u16 TID(void) const { return transport_stream_id; }
  
private:
  u8 numberOfChannels;
  u16 transport_stream_id;

  AtscChannel* (*channels);
};


//////////////////////////////////////////////////////////////////////////////


class RRT : public PSIPTable
{
public:
  RRT(const u8* data);
  //virtual ~RRT() { }  

private:
  
};


//////////////////////////////////////////////////////////////////////////////


class ETT : public PSIPTable, public MultipleStringStructure
{
public:
  ETT(const u8* data);
  //virtual ~ETT() { }  
  u16 SourceID(void) const { return source_id; }
  u16 EventID(void)  const { return event_id; }
  
  static u16 ExtractEventID(const u8* data) { 
    return (data[11] << 6) | ((data[12] & 0xFC) >> 2); 
  }
  
private:
  u16 source_id;
  u16 event_id;
};


//////////////////////////////////////////////////////////////////////////////


#endif // __ATSCTABLES_H
