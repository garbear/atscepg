#include <time.h>
#include <string>

#include <libsi/util.h>

#include "ATSCTables.h"
#include "ATSCTypes.h"


//////////////////////////////////////////////////////////////////////////////


PSIPTable::~PSIPTable()
{
	for (u32 i=0; i<descriptors.size(); i++)
	  delete descriptors[i];
}

//----------------------------------------------------------------------------

Descriptor* PSIPTable::getDescriptor(u32 i)
{
	if (i < descriptors.size()) return descriptors[i];
	
	return NULL;
}

//----------------------------------------------------------------------------

void PSIPTable::addDescriptors(const u8* data, u16 length)
{ 
  const u8* dc = data;

	while ( dc < data+length )
	{ 
	  Descriptor* d = Descriptor::getDescriptor(dc);
	  if (d) descriptors.push_back( d );
		dc += dc[1] + 2;
  }	
}

//----------------------------------------------------------------------------
	
void PSIPTable::update(const u8* data)
{
	table_id               = data[0];
	section_length         = ((data[1] & 0x0F) << 8) | data[2];
	table_id_extension     = (data[3] << 8) | data[4];
	version_number         = (data[5] >> 1) & 0x1F;
	current_next_indicator = data[5] & 0x01;
	section_number         = data[6];
	last_section_number    = data[7];
	protocol_version       = data[8];
	
	CRC_32 = get_u32(data+ section_length-1);
#if 0
	for (u32 i=0; i<section_length; i++) {
    const char* start = (const char*) data + i;
    u16 length = section_length-1- i;
 
  
  if ( SI::CRC32::isValid(start, length, CRC_32) )
	{ DEBUG_MSG("~~~ CRC OK %d ~~~~~~~~~~~~~", i); } else { /*DEBUG_MSG("CRC FAILED");*/ }
	}
#endif	
}

//----------------------------------------------------------------------------

void PSIPTable::print() const
{
	tPrint("   Table ID       : 0x%02X\n", table_id);
	tPrint("   Section Length : %u\n",     section_length); 
	tPrint("   Table ID Ext   : 0x%04X\n", table_id_extension);
	tPrint("   Version Number : %u\n",     version_number);
	tPrint("   Cur/Next ind.  : %u\n",		 current_next_indicator);
	tPrint("   Section Number : 0x%02X\n", section_number);
	tPrint("   Last Section # : 0x%02X\n", last_section_number);
	tPrint("   Protocol Vers. : %u\n",     protocol_version);
	tPrint("   CRC 32         : 0x%08X\n", CRC_32);
}


//////////////////////////////////////////////////////////////////////////////


MGT::MGT(const u8* data): PSIPTable(data)
{
	parse(data);
}

//----------------------------------------------------------------------------

void MGT::update(const u8* data)
{
	PSIPTable::update(data);
	tables.clear();
  parse(data);
}	

//----------------------------------------------------------------------------

inline void MGT::parse(const u8* data)
{
	u16 tables_defined      = (data[9] << 8) | data[10];
	
	const uchar* d = &(data[11]);
	for (u16 i=0; i<tables_defined; i++)
	{
		u16 table_type                = (d[0] << 8) | d[1];
	  u16 table_type_pid            = ((d[2] & 0x1F) << 8) | d[3];
	  //u8  table_type_version_number = d[4] & 0x1F; 
	  //u32 number_bytes              = (d[5] << 24) | (d[6] << 16) | (d[7] << 8) | d[8];
    u16 table_type_descriptors_length = ((d[9] & 0x0F) << 8) | d[10];
	
		Table t( table_type_pid, tableID(table_type), table_type);
		tables.push_back(t);

		d = &(d[11+table_type_descriptors_length]);
		
		// Can only receive stuffing or user private descriptors: ignore
		/*// Inner Loop Descriptors
		const u8* dc = &(d[11]);
		while ( dc < d )
		{ 
			dc = &( dc[dc[1] + 2] );
		}*/ 
	}
	
	
	// u16 descriptors_length = ((d[0] & 0x0F) << 8) | d[1];
	
	/*// Outer Loop Descriptors
	const u8* dc = &(d[2]);
	while ( dc < d +2 + descriptors_length )
	{ 
		dc = &( dc[dc[1] + 2] );
	} */

}

//----------------------------------------------------------------------------

void MGT::print() const {}


//////////////////////////////////////////////////////////////////////////////


STT::STT(const u8* data) : PSIPTable(data)
{
	parse(data);
}

//----------------------------------------------------------------------------

void STT::update(const u8* data)
{
	PSIPTable::update(data);
  parse(data);
}	

//----------------------------------------------------------------------------

inline void STT::parse(const u8* data)
{
  // Number of seconds since 00:00:00 UTC Jan 6, 1980
	system_time      = (data[9] << 24) | (data[10] << 16) | (data[11] << 8) | data[12];
	GPS_UTC_offset   =  data[13]; // Subtract from GPS time for UTC
	daylight_savings = (data[14] << 8) | data[15];
	
	//TODO: Descriptors ?
}

//----------------------------------------------------------------------------

// Does not work - but no longer used anyway
time_t STT::UTCtoLocal(time_t utcTime) const
{
	//TODO: Is there a better way to do this?.
	time_t utc = utcTime - GPS_UTC_offset 
	           + secs_Between_1Jan1970_6Jan1980;

   struct tm* utc_tm = gmtime( &utc );
   utc_tm->tm_isdst = -1;

	 if ( (daylight_savings & 0x8000) >> 15 )
	   utc_tm->tm_hour += 1; // opposite because of -
	   
   return ( 2 * utc ) - mktime( utc_tm );
}

//----------------------------------------------------------------------------

time_t STT::getTime(void) const 
{ 
  return UTCtoLocal(system_time);
}

//----------------------------------------------------------------------------

void STT::print(void) const
{
	time_t now = getTime();
	tPrint("\nCurrent Time: %s\n", ctime( &now) );
}


//////////////////////////////////////////////////////////////////////////////


EIT::EIT(const u8* data) : PSIPTable(data)
{	
  source_id = table_id_extension;
	u8  num_events_in_section  = data[9];

	const uchar* d = &(data[10]);
	for (u8 j=0; j<num_events_in_section; j++)
	{
		// reserved 		2   ‘11’
		u16 event_id      = ((d[0] & 0x3F) << 8) | d[1];
		
    // number of GPS seconds since 00:00:00 UTC, January 6, 1980
		u32 start_time    = (d[2] << 24) | (d[3] << 16) | (d[4] << 8) | d[5];
		
		// reserved 		2   ‘11’
		u8  ETM_location      = (d[6] & 0x30) >> 4;
		u32 length_in_seconds = ((d[6] & 0x0F) << 16) | (d[7] << 8) | d[8];
		u8  title_length      = d[9];
		
		MultipleStringStructure title_text( &(d[10]) );
		
		Event event;

		event.event_id          = event_id;
    event.start_time        = start_time;
    event.length_in_seconds = length_in_seconds;
    event.version_number    = version_number;
    event.table_id          = table_id;
    event.ETM_location      = ETM_location; 
    
		// Assume single string title
    if (title_text.getNumStrings() > 0)
      event.title_text = title_text.getString(0);
    else
    	event.title_text = "No Title";
		
		// reserved     4   ‘1111’
		u16 descriptors_length = ((d[10 + title_length] & 0x0F) << 8) | d[11 + title_length];
	
		// Deal with and remove these descriptors right here...
		//addDescriptors(d+12+title_length, descriptors_length);
	 	
	 	d = &(d[12 + title_length + descriptors_length]);

    events.push_back( event );
	} 
	

}


//////////////////////////////////////////////////////////////////////////////


VCT::VCT(const u8* data) : PSIPTable(data)
{
	transport_stream_id = table_id_extension;
	u8  num_channels_in_section = data[9];
	
	const uchar* d = &(data[10]);
	for (u8 i=0; i<num_channels_in_section; i++)
	{
		
	  //TODO: Proper conversion from UTF-16 (or is this good enough?)
		std::string short_name = "";
		for (int k=0; k<7; k++) short_name += (d[2*k] << 8) | d[2*k+1]; 
		
		/*
		u16 major_channel_number = (((d[14] & 0x0F) << 8) | (d[15] & 0xFC)) >> 2;
		u16 minor_channel_number = ((d[15] & 0x03) << 8) | d[16];
		u8  modulation_mode      = d[17];
    u32 carrier_frequency    = (d[18] << 24) | (d[19] << 16) | (d[20] << 8) | d[21]; // Deprecated
    u16 channel_TSID         = (d[22] << 8) | d[23];
    u16 program_number       = (d[24] << 8) | d[25];

    u8  ETM_location         = (d[26] & 0xC0) >> 6;
    u1  access_controlled    = (d[26] & 0x20) >> 5;
    u1  hidden               = (d[26] & 0x10) >> 4;
    
    // Only used for cable (table_id == 0xC9)
		u1  path_select = (d[26] & 0x08) >> 3;
		u1  out_of_band = (d[26] & 0x04) >> 2;
		u16 one_part_number = (major_channel_number & 0x00F) << 10 + minor_channel_number;
		//TODO: one_part_number only appies under certain conditions...
		
						
    u1  hide_guide           = (d[26] & 0x02) >> 1;    
    u8  service_type         = (d[27] & 0x3F);
    */
    u16 source_id            = (d[28] << 8) | d[29];
    
    Channel ch(transport_stream_id, source_id, short_name); 
		
		u16 descriptors_length   = ((d[30] & 0x03) << 8) | d[31];
    
    addDescriptors(d+32, descriptors_length);
    
    //TODO: More in depth descriptor handling
    for (u32 i=0; i<descriptors.size(); i++)
    {
    	Descriptor* d = descriptors[i];
    	if (d->getTag() == 0xA1) // Service Location Descriptor
    	{
    		ServiceLocationDescriptor* sld = dynamic_cast<ServiceLocationDescriptor*>(d);
    		ch.PCR_PID = sld->getPCR_PID();
    		
    		for (u32 j=0; j<sld->getNumStreams(); j++)
    		{
    		  Stream s = sld->getStream(j);
    		  if (s.stream_type == 0x02) ch.vPID = s.elementary_PID;
    		  else if (s.stream_type == 0x81) ch.aPID = s.elementary_PID;
    		}
    	}
    }
    
	 	d = &(d[32 + descriptors_length]);

      
    
    channels.push_back( ch ); 
	
	}
	 
	/*
	u16 additional_descriptors_length = ((d[0] & 0x03) << 8) | d[1];
	
  for (j=0; j<N; j++) {
   	additional_descriptor()
  }
  */	   
}


//////////////////////////////////////////////////////////////////////////////


RTT::RTT(const u8* data) : PSIPTable(data)
{
  u8  rating_region_name_length =  data[9]; 
  
	MultipleStringStructure rating_region_name_text( &(data[10]) );
	//rating_region_name_text.print(); 

  u8  dimensions_defined        = data[10+rating_region_name_length];
  
  const uchar* d = &(data[11+rating_region_name_length]);
  for (u8 i=0; i<dimensions_defined; i++) 
  {
  	u8 dimension_name_length = d[0];

  	MultipleStringStructure dimension_name_text( &(d[1]) );
  	//dimension_name_text.print();
  	                 
		// reserved		3   ‘111’
		//u1  graduated_scale = (d[1+dimension_name_length] & 0x10) >> 4;
		u8  values_defined  = (d[1+dimension_name_length] & 0x0F);
		
		d = &(d[2+dimension_name_length]); 
		for (u8 j=0; j< values_defined; j ++) 
		{
			u8 abbrev_rating_value_length = d[0];

      MultipleStringStructure abbrev_rating_value_text( &(d[1]) );
      //abbrev_rating_value_text.print();
                 
      u8 rating_value_length = d[1+abbrev_rating_value_length];
      MultipleStringStructure rating_value_text( &(d[2+abbrev_rating_value_length]) );          
      //abbrev_rating_value_text.print();
      
      d = &(d[2 + abbrev_rating_value_length + rating_value_length]);  
		}
  }
  
  // reserved		6   ‘111111’
  //u16 descriptors_length = ((d[0] & 0x03) << 8) | d[1]; 
 /* 
  for (i=0; i<N; i++) {
  	descriptor()
  }
  */
}


//////////////////////////////////////////////////////////////////////////////


ETT::ETT(const u8* data) : PSIPTable(data), MultipleStringStructure( &(data[13]) )
{
	//u32 ETM_id = get_u32( &(data[9] );
	source_id = get_u16( &(data[9]) );
	event_id = (data[11] << 6) | ((data[12] & 0xFC) >> 2);
	
}


//////////////////////////////////////////////////////////////////////////////
