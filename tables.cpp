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
#include <string.h>

#include <libsi/util.h>

#include "tables.h"
#include "types.h"


//////////////////////////////////////////////////////////////////////////////


PSIPTable::PSIPTable(void)
{ 
  table_id               = 0;
  section_length         = 0;
  table_id_extension     = 0;
  version_number         = 0;
  current_next_indicator = 0;
  section_number         = 0;
  last_section_number    = 0;
  protocol_version       = 0;
  crc_passed             = 0;
}


//----------------------------------------------------------------------------

PSIPTable::~PSIPTable()
{
  DeleteDescriptors();
}


//----------------------------------------------------------------------------

Descriptor* PSIPTable::GetDescriptor(u32 i) const
{
  return (i < descriptors.size()) ? descriptors[i] : NULL;
}


//----------------------------------------------------------------------------

void PSIPTable::AddDescriptors(const u8* data, u16 length)
{ 
  const u8* dc = data;

  while ( dc < data+length )
  { 
    Descriptor* d = Descriptor::CreateDescriptor(dc);
    if (d) descriptors.push_back( d );
    dc += dc[1] + 2;
  }  
}


//----------------------------------------------------------------------------

void PSIPTable::DeleteDescriptors(void)
{
  for (u32 i=0; i<descriptors.size(); i++) {
    delete descriptors[i];
    descriptors[i] = NULL;
  }
  
  descriptors.clear();
}


//----------------------------------------------------------------------------
 
void PSIPTable::Update(const u8* data, int length)
{
  table_id       = data[0];
  section_length = ((data[1] & 0x0F) << 8) | data[2];
  
  if (section_length+3 != length) {
    dprint(L_ERR, "PSIPTable: Insufficient data length.");
    crc_passed = false;
    return;
  }
  
  table_id_extension     = get_u16( data+3 );
  version_number         = (data[5] >> 1) & 0x1F;
  current_next_indicator = data[5] & 0x01;
  section_number         = data[6];
  last_section_number    = data[7];
  protocol_version       = data[8];

  crc_passed = SI::CRC32::isValid((const char*)data, section_length+3);
  if (!crc_passed) {
    dprint(L_ERR, "CRC 32 integrity check failed");    
  }
}


//////////////////////////////////////////////////////////////////////////////


MGT::MGT(const u8* data, int length): PSIPTable(data, length)
{
  Parse(data, length);
}


//----------------------------------------------------------------------------

void MGT::Update(const u8* data, int length)
{
  PSIPTable::Update(data, length);
  delete[] tables;
  Parse(data, length);
}  


//----------------------------------------------------------------------------

void MGT::Parse(const u8* data, int length)
{
  if (!crc_passed) {
    numberOfTables = 0;
    tables = NULL;
    return;
  }
  
  numberOfTables = get_u16(data + 9);
  tables = new Table[numberOfTables];
  
  const uchar* d = data + 11;
  for (u16 i=0; i<numberOfTables; i++)
  {
    u16 tableType = get_u16(d);
    
    tables[i].table_type = tableType;
    tables[i].pid = ((d[2] & 0x1F) << 8) | d[3];
    tables[i].tid = TableTypeToTID(tableType);  
    
    //u8  table_type_version_number = d[4] & 0x1F; 
    //u32 number_bytes              = get_u32( d+5 );
    u16 table_type_descriptors_length = ((d[9] & 0x0F) << 8) | d[10];

    d += 11 + table_type_descriptors_length;
    
    // Inner Loop Descriptors
    // Can only receive stuffing or user private descriptors: ignore
    // AddDescriptors(d+11, table_type_descriptors_length);
  }
  
  // Outer Loop Descriptors
  //u16 descriptors_length = ((d[0] & 0x0F) << 8) | d[1];
  //AddDescriptors(d+2, descriptors_length);

}


//////////////////////////////////////////////////////////////////////////////


STT::STT(const u8* data, int length) : PSIPTable(data, length)
{
  Parse(data, length);
}


//----------------------------------------------------------------------------

void STT::Update(const u8* data, int length)
{
  PSIPTable::Update(data, length);
  Parse(data, length);
}  


//----------------------------------------------------------------------------

void STT::Parse(const u8* data, int length)
{
  if (!crc_passed) {
    return;
  }
  
  // Number of seconds since 00:00:00 UTC Jan 6, 1980
  system_time      = get_u32( data+9 );
  GPS_UTC_offset   = data[13]; // Subtract from GPS time for UTC
  daylight_savings = get_u16( data+14 );
  
  //TODO: Descriptors ?
}


//----------------------------------------------------------------------------

time_t STT::GetGPSTime(void) const 
{ 
  return (system_time - GPS_UTC_offset);
}


//////////////////////////////////////////////////////////////////////////////


EIT::EIT(const u8* data, int length) : PSIPTable(data, length)
{
  if (!crc_passed) {
    source_id = 0;
    numberOfEvents = 0;
    events = NULL;
    return;
  }
  
  source_id = table_id_extension;
  numberOfEvents  = data[9];
  events = new Event[numberOfEvents];
  
  const uchar* d = data + 10;
  for (u8 i = 0; i < numberOfEvents; i++)
  {
    events[i].version_number    = version_number;
    events[i].table_id          = table_id;
    events[i].event_id = ((d[0] & 0x3F) << 8) | d[1];
    events[i].start_time = get_u32(d + 2);  
    events[i].ETM_location      = (d[6] & 0x30) >> 4;
    events[i].length_in_seconds = ((d[6] & 0x0F) << 16) | (d[7] << 8) | d[8];
    
    u8 title_length = d[9];
    
    if (title_length > 0) {
      MultipleStringStructure title_text(d + 10);
      events[i].SetTitleText( title_text.GetString(0).c_str() ); // Assume single string title
    }
    else
      events[i].SetTitleText("No Title");

    u16 descriptors_length = ((d[10 + title_length] & 0x0F) << 8) | d[11 + title_length];
  
    // Deal with and remove these descriptors right here...
    // AddDescriptors(d+12+title_length, descriptors_length);

    d += 12 + title_length + descriptors_length;
  } 
  
}


//////////////////////////////////////////////////////////////////////////////


VCT::VCT(const u8* data, int length) : PSIPTable(data, length)
{
  if (!crc_passed) {
    transport_stream_id = 0;
    numberOfChannels = 0;
    channels = NULL;
    return;
  }
  
  transport_stream_id = table_id_extension;
  numberOfChannels    = data[9];
  channels = new AtscChannel*[numberOfChannels];

  const uchar* d = data + 10;
  for (u8 i = 0; i < numberOfChannels; i++)
  { 
    channels[i] = new AtscChannel();
    
    //TODO: Proper conversion from UTF-16
    std::string short_name = "";
    for (int k=0; k<7; k++) short_name += (d[2*k] << 8) | d[2*k+1]; 
    channels[i]->SetShortName( short_name.c_str() );
    
    channels[i]->SetMajorNumber( (((d[14] & 0x0F) << 8) | (d[15] & 0xFC)) >> 2 );
    channels[i]->SetMinorNumber(  ((d[15] & 0x03) << 8) | d[16] );
    /*
    u8  modulation_mode      = d[17];
    u32 carrier_frequency    = get_u32( d+18 );
    u16 channel_TSID         = get_u16( d+22 );
    u16 program_number       = get_u16( d+24 );

    u8  ETM_location         = (d[26] & 0xC0) >> 6;
    u1  access_controlled    = (d[26] & 0x20) >> 5;
    u1  hidden               = (d[26] & 0x10) >> 4;
    
    // Only used for cable (table_id == 0xC9)
    u1  path_select = (d[26] & 0x08) >> 3;
    u1  out_of_band = (d[26] & 0x04) >> 2;
    u16 one_part_number = (major_channel_number & 0x00F) << 10 + minor_channel_number;
    //XXX: one_part_number only appies under certain conditions...
         
    u1  hide_guide           = (d[26] & 0x02) >> 1;    
    u8  service_type         = (d[27] & 0x3F);
    */
    
    channels[i]->SetId(transport_stream_id, get_u16(d+28));
     
    u16 descriptors_length = ((d[30] & 0x03) << 8) | d[31];
    AddDescriptors(d+32, descriptors_length);
    
    int Vpid = 0;
    int Vtype = 0;
    int Ppid = 0;
  
    int Dpids[MAXDPIDS + 1] = { 0 };
    char DLangs[MAXDPIDS][MAXLANGCODE2] = { "" };
    int NumDpids = 0;  
  
    //TODO: More in depth descriptor handling
    for (size_t j=0; j<descriptors.size(); j++)
    {
      Descriptor* d = descriptors[j]; 
      if (d->GetTag() == 0xA1) // Service Location Descriptor
      {
        ServiceLocationDescriptor* sld = dynamic_cast<ServiceLocationDescriptor*>(d);

        for (u8 k = 0; k < sld->NumberOfStreams(); k++)
        { 
          const Stream* s = sld->GetStream(k);
          switch(s->stream_type)
          {
            case 0x02: // ITU-T Rec. H.262 | ISO/IEC 13818-2 Video or ISO/IEC 11172-2 constrained parameter video stream
            case 0x1B: // H.264/MPEG-4 AVC (ISO/IEC 14496-10)
              Vpid = s->elementary_PID;
              Ppid = sld->GetPCR_PID();
              Vtype = s->stream_type;
            break;
            
            case 0x81: // Audio per ATSC A/53E Annex B
              if (NumDpids < MAXDPIDS) {
                Dpids[NumDpids] = s->elementary_PID;
                strncpy(DLangs[NumDpids], s->ISO_639_language_code, 3);
                NumDpids++;
              }
            break;
            
            case 0x05: // Software download data service (A/97)
            case 0x0B: // DSM-CC sections containing A/90 asynchronous data
              // Some PBS stations use UpdateTV based on ATSC A/97 (updatelogic.com)
            break;  
            
            case 0x06: // PES packets containing A/90 streaming, synchronized data
            case 0x0D: // DSM-CC addressable sections per A/90
            case 0x14: // DSM-CC sections containing non-streaming, synchronized data per A/90
            case 0x95: // Sections conveying A/90 Data Service Table, Network Resources
            case 0xC2: // PES packets containing A/90 streaming, synchronous data
              // Ignored for now...
            break;

            default:
              dprint(L_ERR, "Found unknown stream type 0x%02X", s->stream_type);
          }
        }
      } 
      else if (d->GetTag() == 0xA0) // Extended Channel Name Descriptor
      {
        ExtendedChannelNameDescriptor* ecnd = dynamic_cast<ExtendedChannelNameDescriptor*>(d);
        channels[i]->SetLongName( ecnd->GetLongChannelName().c_str() );
      }
      else
        dprint(L_ERR, "Unhandled VCT descriptor 0x%02X",  d->GetTag());  
    }
    
    DeleteDescriptors();
    
    channels[i]->SetPids(Vpid, Ppid, Vtype, Dpids, DLangs);     
    
    d += 32 + descriptors_length;
  }
   
  /*
  u16 additional_descriptors_length = ((d[0] & 0x03) << 8) | d[1];
  
  for (j=0; j<N; j++) {
     additional_descriptor()
  }
  */     
}


//----------------------------------------------------------------------------

VCT::~VCT()
{ 
  for (int i=0; i<numberOfChannels; i++) {
    delete channels[i];
    channels[i] = NULL;
  }
  
  delete[] channels;
}


//////////////////////////////////////////////////////////////////////////////


RRT::RRT(const u8* data, int length) : PSIPTable(data, length)
{
  if (!crc_passed) {
    return;
  }
  
  u8 rating_region_name_length =  data[9]; 
  
  MultipleStringStructure rating_region_name_text( data + 10 );
  rating_region_name_text.Print(); 

  u8 dimensions_defined = data[10 + rating_region_name_length];
  
  const uchar* d = data + 11 + rating_region_name_length;
  for (u8 i=0; i<dimensions_defined; i++) 
  {
    u8 dimension_name_length = d[0];

    MultipleStringStructure dimension_name_text( d + 1 );
    dimension_name_text.Print();

    //u1  graduated_scale = (d[1+dimension_name_length] & 0x10) >> 4;
    u8  values_defined  = (d[1+dimension_name_length] & 0x0F);
    
    d += 2 + dimension_name_length;
    
    for (u8 j=0; j< values_defined; j ++) 
    {
      u8 abbrev_rating_value_length = d[0];

      MultipleStringStructure abbrev_rating_value_text( d + 1 );
      abbrev_rating_value_text.Print();
                 
      u8 rating_value_length = d[1+abbrev_rating_value_length];
      MultipleStringStructure rating_value_text( d + 2 + abbrev_rating_value_length );          
      abbrev_rating_value_text.Print();
      
      d += 2 + abbrev_rating_value_length + rating_value_length;
    }
  }

  //u16 descriptors_length = ((d[0] & 0x03) << 8) | d[1]; 
 /* 
  for (i=0; i<N; i++) {
    descriptor()
  }
  */
}


//////////////////////////////////////////////////////////////////////////////


ETT::ETT(const u8* data, int length) : PSIPTable(data, length)
{
  if (!crc_passed) {
    source_id = event_id = 0;
    mss = NULL;
    return;
  }
  
  source_id = get_u16( data + 9 );
  event_id  = (data[11] << 6) | ((data[12] & 0xFC) >> 2);
  
  mss = new MultipleStringStructure(data + 13);
}


//////////////////////////////////////////////////////////////////////////////


