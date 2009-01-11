#ifndef __ATSCDESCRIPTORS_H
#define __ATSCDESCRIPTORS_H

#include <string>

#include "tools.h"
#include "structs.h"


//////////////////////////////////////////////////////////////////////////////


class MultipleStringStructure
{
public:
  MultipleStringStructure(const u8* data);
  virtual ~MultipleStringStructure();  
  
  u8 NumberOfStrings(void) const { return number_strings; }
  std::string GetString(u32 i) const;
  virtual void Print(void) const;
  
protected:
  u8 number_strings;
  std::vector<std::string> strings;
};


//////////////////////////////////////////////////////////////////////////////


class Descriptor
{
public:
  Descriptor(const u8* data); 
  virtual ~Descriptor() { }
  
  virtual void Print(void);
  u8 GetTag(void) { return descriptor_tag; }
  
  static Descriptor* CreateDescriptor(const u8* data);
  
protected:
  u8 descriptor_tag;
  u8 descriptor_length;
  
};


//////////////////////////////////////////////////////////////////////////////


class AC3AudioDescriptor : public Descriptor
{
public:  
  AC3AudioDescriptor(const u8* data);
  virtual void Print(void);
  
private:
  u8  sample_rate_code;
  u8  bsid; 
  u8  bit_rate_code;
  u8  surround_mode;
  u8  bsmod;
  u8  num_channels;
  u1  full_svc;
  u8  langcod;
  u8  langcod2;
  u8 mainid;
  u8 priority;
  u8 asvcflags;
  std::string text;
  u1 language_flag;
  u1 language_flag_2;
  u32 language;
  u32 language_2;
};


//////////////////////////////////////////////////////////////////////////////


class CaptionServiceDescriptor : public Descriptor
{
public:  
  CaptionServiceDescriptor(const u8* data);
  
private:

};


//////////////////////////////////////////////////////////////////////////////


class ExtendedChannelNameDescriptor : public Descriptor
{
public:  
  ExtendedChannelNameDescriptor(const u8* data);
  
  std::string GetLongChannelName(void) { return long_channel_name_text; }
private:
  std::string long_channel_name_text;
};


//////////////////////////////////////////////////////////////////////////////


class ServiceLocationDescriptor : public Descriptor
{
public:  
  ServiceLocationDescriptor(const u8* data);
  virtual ~ServiceLocationDescriptor(void) { delete[] streams; }
  
  u8 NumberOfStreams(void) const  { return numberOfStreams; }
  const Stream* GetStream(u8 i)   { return (i<numberOfStreams) ? &streams[i] : NULL; }
  u16 GetPCR_PID(void) const      { return PCR_PID; }
  
private:
  u16 PCR_PID;
  u8 numberOfStreams;
  Stream* streams;
};


//////////////////////////////////////////////////////////////////////////////


class GenreDescriptor : public Descriptor
{
public:  
  GenreDescriptor(const u8* data);
  virtual ~GenreDescriptor() { delete[] attributes; }
   
  u8 NumberOfGenres(void) { return attribute_count; }
  u8 GetGenre(u8 i);
  
private:
  u8  attribute_count;
  u8* attributes;
};


//////////////////////////////////////////////////////////////////////////////


class ContentAdvisoryDescriptor : public Descriptor
{
public:  
  ContentAdvisoryDescriptor(const u8* data);
  
private:
};


//////////////////////////////////////////////////////////////////////////////

// Non-implemented descriptors

// class StuffingDescriptor
// class TimeShiftedServiceDescriptor : public Descriptor
// class ComponentNameDescriptor : public Descriptor
// class DCCDepartingRequestDescriptor : public Descriptor
// class DCCArrivingRequestDescriptor : public Descriptor
// class RedistributionControlDescriptor : public Descriptor
// class ATSCPrivateInformationDescriptor : public Descriptor
// class ContentIdentifierDescriptor : public Descriptor



//////////////////////////////////////////////////////////////////////////////


#endif //__ATSCDESCRIPTORS_H
