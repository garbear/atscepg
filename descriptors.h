#ifndef __ATSCDESCRIPTORS_H
#define __ATSCDESCRIPTORS_H

#include <string>
#include <vector>

#include "tools.h"
#include "structs.h"

//////////////////////////////////////////////////////////////////////////////


class MultipleStringStructure
{
public:
	MultipleStringStructure(const u8* data);
	virtual ~MultipleStringStructure();	
	
	u32 getNumStrings(void) const { return number_strings; }
	std::string getString(u32 i) const;
	virtual void print(void) const;
	
protected:
	u8 number_strings;
	std::vector<std::string> strings;
};


//////////////////////////////////////////////////////////////////////////////


class Descriptor
{
public:
	Descriptor(const u8* data) 
	{
		descriptor_tag    = data[0];
		descriptor_length = data[1];
	}
	
	virtual ~Descriptor() {}
	virtual void print(void)
	{ 
		dprint(L_DAT, "  Descriptor Tag    : 0x%02X", descriptor_tag);
		dprint(L_DAT, "  Descriptor Length : %d", descriptor_length);
	}
	
	u8 getTag(void) { return descriptor_tag; }
	static Descriptor* getDescriptor(const u8* data);
	
protected:
  u8 descriptor_tag;
	u8 descriptor_length;
	
};


//////////////////////////////////////////////////////////////////////////////


class AC3AudioDescriptor : public Descriptor
{
public:	
	AC3AudioDescriptor(const u8* data);
	void print(void);
	
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
	std::string getLongChannelName(void) { return long_channel_name_text; }
private:
	std::string long_channel_name_text;
};


//////////////////////////////////////////////////////////////////////////////


class ServiceLocationDescriptor : public Descriptor
{
public:	
	ServiceLocationDescriptor(const u8* data);
	u32 getNumStreams(void) { return streams.size(); }
	Stream getStream(u32 i) { return streams[i]; }
	u16 getPCR_PID(void) { return PCR_PID; }
	
private:
  u16 PCR_PID;
  std::vector<Stream> streams;
};


//////////////////////////////////////////////////////////////////////////////


class GenreDescriptor : public Descriptor
{
public:	
	GenreDescriptor(const u8* data);
	virtual ~GenreDescriptor() { delete[] attributes; } 
	u8 getNumGenres(void) { return attribute_count; }
	u8 getGenre(u8 i);
	
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
