#include <string>
#include <vector>

#include "descriptors.h"
#include "huffman.h"
#include "types.h"


//////////////////////////////////////////////////////////////////////////////

  
MultipleStringStructure::MultipleStringStructure(const u8* data)
{
  number_strings = data[0];

  for (u8 i=0; i<number_strings; i++)
  {
    //TODO: use ISO_639_language_code
    //u32 ISO_639_language_code = (data[1] << 16) | (data[2] << 8) | data[3];
    u8  number_segments = data[4];
    
    std::string str;
    const uchar* d = data + 5;
    for (u8 j=0; j<number_segments; j++) 
    {
      u8 compression_type = d[0];
      u8 mode             = d[1];
      u8 number_bytes     = d[2];
      
      std::string dec;
      switch (compression_type)
      {
        case 0x00: // No compression
          dec = Uncompressed( d+3 , number_bytes, mode); 
          break;
          
        case 0x01: // Huffman - Tables C.4 & C.5
        case 0x02: // Huffman - Tables C.6 & C.7
          dec = ATSCHuffman1toString(d+3, number_bytes, compression_type);
          break;
          
        // 0x03 to 0xAF: reserved
        // 0xB0 to 0xFF: Used in other systems
        
        default:
          dec = "-E-";
      };
      
      str += dec; // Add decompressed segment
       
    }
    
    strings.push_back(str);
   }
}


//----------------------------------------------------------------------------

MultipleStringStructure::~MultipleStringStructure()
{

}


//----------------------------------------------------------------------------

void MultipleStringStructure::Print(void) const
{
  for (u8 i=0; i<number_strings; i++)
  {
    dprint(L_DAT, "%s", strings[i].c_str() );
  }
}


//----------------------------------------------------------------------------

std::string MultipleStringStructure::GetString(u32 i) const 
{ 
  if (i >= strings.size()) return "Out of range";
  
  return strings[i]; 
} 


//////////////////////////////////////////////////////////////////////////////


Descriptor::Descriptor(const u8* data) 
{
  descriptor_tag    = data[0];
  descriptor_length = data[1];
}


//----------------------------------------------------------------------------

void Descriptor::Print(void)
{ 
  dprint(L_DAT, "Descriptor Tag    : 0x%02X", descriptor_tag);
  dprint(L_DAT, "Descriptor Type   : %s", DescriptorText(descriptor_tag));
  dprint(L_DAT, "Descriptor Length : %d", descriptor_length);
}
  
  
//----------------------------------------------------------------------------
  
Descriptor* Descriptor::CreateDescriptor(const u8* data)
{
  //dprint(L_DBG, "Got %s", DescriptorText(data[0]));
  
  if      (data[0] == 0x80) // Stuffing Descriptor
    return NULL;
    
  else if (data[0] == 0x81) // AC-3 audio Descriptor
    return new AC3AudioDescriptor(data); 
              
  else if (data[0] == 0x86) // Caption Service Descriptor
    return new CaptionServiceDescriptor(data); 
      
  else if (data[0] == 0x87) // Content Advisory Descriptor
    return new ContentAdvisoryDescriptor(data); 
    
  else if (data[0] == 0xA0) // Extended Channel Name Descriptor
    return new ExtendedChannelNameDescriptor(data);
    
  else if (data[0] == 0xA1) // Service Location Descriptor
    return new ServiceLocationDescriptor(data);
    
  else if (data[0] == 0xA2) // Time-Shifted Service Descriptor
    return NULL;
    
  else if (data[0] == 0xA3) // Component Name Descriptor
    return NULL;
    
  else if (data[0] == 0xA8) // DCC Departing Request Descriptor
    return NULL;
    
  else if (data[0] == 0xA9) // DCC Arriving Request Descriptor 
    return NULL;
               
  else if (data[0] == 0xAA) // Redistribution Control Descriptor
    return NULL;
              
  else if (data[0] == 0xAD) // ATSC Private Information Descriptor
    return NULL;
          
  else if (data[0] == 0xB6) // Content Identifier Descriptor
    return NULL;
    
  else if (data[0] == 0xAB) // Genre Descriptor
    return new GenreDescriptor(data);
  
  else {
    dprint(L_ERR, "Unknown descriptor type: 0x%02X", data[0]);
    return NULL;
  }
}

//////////////////////////////////////////////////////////////////////////////

// TESTED - ok
// TODO: Not all fields are always specified, find appropriate default values
AC3AudioDescriptor::AC3AudioDescriptor(const u8* data) : Descriptor(data)
{
  sample_rate_code = (data[2] & 0xE0) >> 5;
  bsid             = (data[2] & 0x1F); 
  bit_rate_code    = (data[3] & 0xFC) >> 2;
  surround_mode    = (data[3] & 0x03);
  bsmod            = (data[4] & 0xE0) >> 5;
  num_channels     = (data[4] & 0x1E) >> 1;
  full_svc         =  data[4] & 0x01;
  langcod          =  data[5];

  const u8* d = data + 6;
  //-------
  if(num_channels == 0) // 1+1 mode
  {
    langcod2 = d[0];
    d++;
  }
  else
    langcod2 = 0xFF;
  //-------
  if (bsmod < 2) 
  {
    mainid   = (d[0] & 0xE0) >> 5;
    priority = (d[0] & 0x18) >> 3;
    //reserved    3    ‘111’
    d++;
  }
  else 
  {
    asvcflags = d[0];
    d++;
  }
  //-------
  u8 textlen   = (d[0] & 0xFE) >> 1;
  u1 text_code = (d[0] & 0x01);
  
  text = "";
  if (text_code) // ISO Latin-1
    for (u8 i=0; i<textlen; i++)
      text += d[1 + i];  
  else           // UTF-16
    for (u8 i=0; i<textlen; i+=2)
      text += get_u16( d+1+i );
  
  d += 1+textlen;      
  //-------    

  language_flag   = (d[0] & 0x80) >> 7;
  language_flag_2 = (d[0] & 0x40) >> 6;
  // reserved    6    ‘111111’
  
  if (language_flag) 
  {
    language = get_u24( d + 1 );
    d += 3;
  }
  if(language_flag_2) 
  {
    language_2 = get_u24( d + 1 );
    d += 3;
  }
  
  d++;
  
  //-------
  /*
    for(i=0; i<N; i++) {                         
          additional_info[i]  = d[i];
    }
  */

}

//----------------------------------------------------------------------------

void AC3AudioDescriptor::Print(void)
{
  dprint(L_DAT, "========== AC-3 Descriptor ==========");
  dprint(L_DAT, "  Descriptor Length : %d", descriptor_length);
  dprint(L_DAT, "  Sample Rate Code  : %s", SampleRateText(sample_rate_code) );
  //bsid; 
  dprint(L_DAT, "  Bit Rate Code     : %s", BitRateText(bit_rate_code) );
  dprint(L_DAT, "  Surround Mode     : %s", SurroundModeText(surround_mode) );
  //bsmod;
  dprint(L_DAT, "  Number Channels   : %s", NumberOfChannelsText(num_channels) );
  dprint(L_DAT, "=====================================");
  //full_svc;
  //langcod;
  //langcod2;
  //mainid;
  //priority;
  //asvcflags;
  //std::string text;
  //language_flag;
  //language_flag_2;
  //language;
  //language_2;
}


//////////////////////////////////////////////////////////////////////////////

// TESTED - partial test: ok
CaptionServiceDescriptor::CaptionServiceDescriptor(const u8* data) : Descriptor(data)
{
  u8 number_of_services = (data[2] & 0x1F);
  
  const u8* d = data + 3;
  for (u8 i=0; i<number_of_services; i++)
  {
    u32 ISO_639_language_code = get_u24(d);
    
    u1  digital_cc = (d[3] & 0x80) >> 7;
    
    if (!digital_cc) 
      u1 line21_field = (d[3] & 0x01); // Deprecated
    else
      u8 caption_service_number = (d[3] & 0x3F);
      
    u1 easy_reader       = (d[4] & 0x80) >> 7;
    u1 wide_aspect_ratio = (d[4] & 0x40) >> 6;
    
    d += 6;
  }
}


//////////////////////////////////////////////////////////////////////////////

// UNTESTED
ExtendedChannelNameDescriptor::ExtendedChannelNameDescriptor(const u8* data): Descriptor(data)
{
  MultipleStringStructure long_channel_name( data + 3 );
  
  // Assume we get a single string
  long_channel_name_text = long_channel_name.GetString(0);
}


//////////////////////////////////////////////////////////////////////////////

// TESTED - partial test: ok
ServiceLocationDescriptor::ServiceLocationDescriptor(const u8* data) : Descriptor(data)
{
  PCR_PID = ((data[2] & 0x1F) << 8) | data[3];
  
  numberOfStreams = data[4];
  streams = new Stream[numberOfStreams];
  
  const u8* d = data + 5;
  for (u8 i=0; i< numberOfStreams; i++) 
  {
    streams[i].stream_type = d[0];
    streams[i].elementary_PID = ((d[1] & 0x1F) << 8) | d[2];
    streams[i].ISO_639_language_code = get_u24(d+3);  

    d += 6;
  }
}


//////////////////////////////////////////////////////////////////////////////

// UNTESTED
GenreDescriptor::GenreDescriptor(const u8* data): Descriptor(data)
{
  attribute_count = data[2] & 0x1F;
  attributes = new u8[attribute_count];
  
  for (u8 i=0; i<attribute_count; i++) 
  {
    attributes[i] = data[3 + i];
  }
}


//----------------------------------------------------------------------------

u8 GenreDescriptor::GetGenre(u8 i)
{
  return (i < attribute_count) ? attributes[i] : 0xFF;
}


//////////////////////////////////////////////////////////////////////////////

// TESTED - ok
ContentAdvisoryDescriptor::ContentAdvisoryDescriptor(const u8* data): Descriptor(data)
{
  u8 rating_region_count = data[2] & 0x3F;
  
  const u8* d = data + 3;
  for (u8 i=0; i<rating_region_count; i++)
  {    
    u8 rating_region    = d[0];
    u8 rated_dimensions = d[1];

    for (u8 j=0; j< rated_dimensions; j++)
    {
      u8 rating_dimension_j = d[2 + 2*j];
      u8 rating_value       = d[2 + 2*j + 1] & 0x0F;
    }
    
    u8 rating_description_length = d[2 + 2*rated_dimensions];
    
    if (rating_description_length > 0)
    {
      MultipleStringStructure rating_description_text( d + 2 + 2*rated_dimensions + 1 );
      rating_description_text.Print(); 
    }
     
    d += 2 + 2*rated_dimensions + 1 + rating_description_length;

  }
}


//////////////////////////////////////////////////////////////////////////////

