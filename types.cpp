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

#include "types.h"


//////////////////////////////////////////////////////////////////////////////


const char* TableTypeText(u16 type)
{
	if      (type == 0x0000)
		return "Terrestrial VCT with current_next_indicator=’1’";
	else if (type == 0x0001)
		return "Terrestrial VCT with current_next_indicator=’0’";
	else if (type == 0x0002)
		return "Cable VCT with current_next_indicator=’1’";
	else if (type == 0x0003)
		return "Cable VCT with current_next_indicator=’0’";		
	else if (type == 0x0004)
		return "Channel ETT";		
	else if (type == 0x0005)
		return "DCCSCT";
	else if ((type >= 0x0006 && type <= 0x00FF) || (type >= 0x0180 && type <= 0x01FF) ||	
	         (type >= 0x0280 && type <= 0x0300) || (type >= 0x1000 && type <= 0x13FF) ||
	         (type >= 0x1500))
	  return "Reserved for future ATSC use";
	else if (type >= 0x0400 && type <= 0x0FFF)
	  return "User private";
	else if (type >= 0x0100 && type <= 0x017F)
	  return "EIT";	
	else if (type >= 0x0200 && type <= 0x027F)
	  return "ETT";	
	else if (type >= 0x0301 && type <= 0x03FF)
	  return "RRT";	
	else if (type >= 0x1400 && type <= 0x14FF)
	  return "DCCT";					 
 
	return "Unknown";
}

//----------------------------------------------------------------------------

u8 TableTypeToTID(u16 type)
{
	if      (type == 0x0000 || type == 0x0001) return 0xC8;
	else if (type == 0x0002 || type == 0x0003) return 0xC9;	
	else if (type == 0x0004) return 0xCC;		
	else if (type == 0x0005) return 0xD4;
	else if (type >= 0x0100 && type <= 0x017F) return 0xCB;	
	else if (type >= 0x0200 && type <= 0x027F) return 0xCC;	
	else if (type >= 0x0301 && type <= 0x03FF) return 0xCA;	
	else if (type >= 0x1400 && type <= 0x14FF) return 0xD3;					 
 
	return 0x00;
}

//----------------------------------------------------------------------------

const char* ModulationModeText(u8 type)
{
	if      (type == 0x00)	return "Reserved";
	else if (type == 0x01)	return "Analog";
	else if (type == 0x02)	return "SCTE Mode 1 (QAM-64)";
	else if (type == 0x03)	return "SCTE Mode 2 (QAM-256)";
	else if (type == 0x04)	return "ATSC (8-VSB)";
	else if (type == 0x05)	return "ATSC (16-VSB)";
	else if (type >= 0x06 && type <= 0x7F)
													return "Reserved for future use by ATSC";
	else if (type >= 0x80)  return "User Private";
	
	return "Unknown";
}

//----------------------------------------------------------------------------

const char* ETMLocationText(u8 type, u8 tableID)
{
	switch (type)
	{
		case 0x00: return "No ETM";
		case 0x01: return "PTC carrying this PSIP";
		case 0x02: 
			if (tableID == 0xC8 || tableID == 0xC9) // VCT
							 return "PTC specified by the Channel TSID";
			else if (tableID == 0xCB) // EIT
							 return "PTC carrying this event";
			break;
		case 0x03: return "Reserved for future ATSC use";
	};
	
	return "Unknown";
}

//----------------------------------------------------------------------------

const char* ServiceTypeText(u8 type)
{
	if      (type == 0x00)	return "Reserved";
	else if (type == 0x01)	return "Analog Television";
	else if (type == 0x02)	return "ATSC Digital Television";
	else if (type == 0x03)	return "ATSC Audio";
	else if (type == 0x04)	return "ATSC Data Only";	
	else if (type >= 0x05 && type <= 0x3F)
													return "Reserved for future ATSC use";
													
	return "Unknown";													
}

//----------------------------------------------------------------------------

const char* DescriptorText(u8 type)
{
	switch (type)
	{
		case 0x80:	return "Stuffing Descriptor";
		case 0x81:	return "AC-3 Audio Descriptor";
		case 0x86:	return "Caption Service Descriptor";
		case 0x87:	return "Content Advisory Descriptor";
		case 0xA0:	return "Extended Channel Name Descriptor";
		case 0xA1:	return "Service Location Descriptor";
		case 0xA2:	return "Time-Shifted Service Descriptor";
		case 0xA3:	return "Component Name Descriptor";
		case 0xA8:	return "DCC Departing Request Descriptor";
		case 0xA9:	return "DCC Arriving Request Descriptor";
		case 0xAA:	return "Redistribution Control Descriptor";
		case 0xAD:	return "ATSC Private Information Descriptor";
		case 0xB6:	return "Content Identifier Descriptor";
		case 0xAB:	return "Genre Descriptor";
	}
	
	return "Unknown";
}

//----------------------------------------------------------------------------

const char* StreamTypeText(u8 type)
{
	switch (type)
	{
    case 0x02: return "ITU-T Rec. H.262";
    case 0x06: return "PES packets containing A/90 streaming, synchronized data";
    case 0x0B: return "DSM-CC sections containing A/90 asynchronous data";
    case 0x0D: return "DSM-CC addressable sections per A/90";
    case 0x14: return "DSM-CC sections containing non-streaming, synchronized data per A/90";
    case 0x81: return "Audio per ATSC A/53E";
    case 0x95: return "Sections conveying A/90 Data Service Table, Network Resources Table";
    case 0xC2: return "PES packets containing A/90 streaming, synchronous data";
	}
	
	return "Unknown";
}

//----------------------------------------------------------------------------

const char* SampleRateText(u8 type)
{
	switch (type)
	{
	  case  0: return "48 kbps";
	  case  1: return "44.1 kbps";
	  case  2: return "32 kbps";
	  case  3: return "Reserved";
	  case  4: return "48 kbps or 44.1 kbps";
	  case  5: return "48 kbps or 32 kbps";
	  case  6: return "44.1 kbps or 32 kbps";
	  case  7: return "48 kbps or 44.1 kbps or 32 kbps";
	}
	
	return "Unknown";
}

//----------------------------------------------------------------------------

static const char* const exactBitRate[19] = {
  "32 kbps",  "40 kbps",  "48 kbps",  "56 kbps",  "64 kbps",
  "80 kbps",  "96 kbps",  "112 kbps", "128 kbps", "160 kbps",
  "192 kbps", "224 kbps", "256 kbps", "320 kbps", "384 kbps",
  "448 kbps", "512 kbps", "576 kbps", "640 kbps"
};

static const char* const upperBitRate[19] = {
  "<= 32 kbps",  "<= 40 kbps", "<= 48 kbps",  "<= 56 kbps",  "<= 64 kbps",
  "<= 80 kbps",  "<= 96 kbps", "<= 112 kbps", "<= 128 kbps", "<= 160 kbps",
  "<= 192 kbps","<= 224 kbps", "<= 256 kbps", "<= 320 kbps", "<= 384 kbps",
  "<= 448 kbps","<= 512 kbps", "<= 576 kbps", "<= 640 kbps"
};

const char* BitRateText(u8 type)
{
  if (type <= 18)  
    return exactBitRate[type];
  else if (type >= 32 && type <= 50)
    return upperBitRate[type-32];

  return "Unknown";
}


//----------------------------------------------------------------------------

const char* SurroundModeText(u8 type)
{
  switch (type)
	{
	  case 0: return "Not indicated";
	  case 1: return "Not Dolby surround encoded";
	  case 2: return "Dolby surround encoded";
	  case 3: return "Reserved";
	}
	
  return "Unknown"; 
}


//----------------------------------------------------------------------------
   
static const char* const numberOfChannelsStrings[] = {
  "1 + 1",    "1/0",      "2/0",      "3/0",
  "2/1",      "3/1",      "2/2 ",     "3/2",
  "1",        "<= 2",     "<= 3",     "<= 4",
  "<= 5",     "<= 6",     "Reserved", "Reserved"
};

const char* NumberOfChannelsText(u8 type)
{
  if (type < 16) return numberOfChannelsStrings[type]; 
  
  return "Unknown"; 
}

//----------------------------------------------------------------------------

static const char* const genres[] = {
"Not Available",    "Reserved (Basic)", "Reserved (Basic)", "Reserved (Basic)",
"Reserved (Basic)", "Reserved (Basic)", "Reserved (Basic)", "Reserved (Basic)",
"Reserved (Basic)", "Reserved (Basic)", "Reserved (Basic)", "Reserved (Basic)",
"Reserved (Basic)", "Reserved (Basic)", "Reserved (Basic)", "Reserved (Basic)",
"Reserved (Basic)", "Reserved (Basic)", "Reserved (Basic)", "Reserved (Basic)",
"Reserved (Basic)", "Reserved (Basic)", "Reserved (Basic)", "Reserved (Basic)",
"Reserved (Basic)", "Reserved (Basic)", "Reserved (Basic)", "Reserved (Basic)",
"Reserved (Basic)", "Reserved (Basic)", "Reserved (Basic)", "Reserved (Basic)",
"Education",        "Entertainment",    "Movie ",           "News", 
"Religious",        "Sports",           "Other",            "Action", 
"Advertisement",    "Animated",         "Anthology",        "Automobile", 
"Awards ",          "Baseball",         "Basketball",       "Bulletin", 
"Business",         "Classical",        "College",          "Combat", 
"Comedy",           "Commentary",       "Concert",          "Consumer", 
"Contemporary",     "Crime",            "Dance ",           "Documentary", 
"Drama",            "Elementary",       "Erotica",          "Exercise", 
"Fantasy",          "Farm",             "Fashion",          "Fiction", 
"Food",             "Football",         "Foreign",          "Fund Raiser", 
"Game/Quiz",        "Garden",           "Golf ",            "Government", 
"Health",           "High School",      "History",          "Hobby ", 
"Hockey",           "Home ",            "Horror",           "Information", 
"Instruction",      "International",    "Interview",        "Language", 
"Legal",            "Live",             "Local",            "Math", 
"Medical",          "Meeting",          "Military",         "Miniseries",  
"Music",            "Mystery",          "National",         "Nature", 
"Police",           "Politics",         "Premier",          "Prerecorded", 
"Product",          "Professional",     "Public",           "Racing", 
"Reading",          "Repair",           "Repeat",           "Review",   
"Romance",          "Science",          "Series",           "Service",  
"Shopping",         "Soap Opera",       "Special",          "Suspense",  
"Talk",             "Technical",        "Tennis",           "Travel",  
"Variety",          "Video",            "Weather",          "Western",   
"Art",              "Auto Racing",      "Aviation",         "Biography",  
"Boating",          "Bowling",          "Boxing",           "Cartoon",  
"Children",         "Classic Film",     "Community",        "Computers", 
"Country Music",    "Court",            "Extreme Sports",   "Family", 
"Financial",        "Gymnastics",       "Headlines",        "Horse Racing", 
"Hunting/Fishing/Outdoors", "Independent", "Jazz",          "Magazine", 
"Motorcycle Racing","Music/Film/Books", "News-International", "News-Local",
"News-National",    "News-Regional",    "Olympics",         "Original",   
"Performing Arts",  "Pets/Animals",     "Pop",              "Rock & Roll", 
"Sci-Fi",           "Self Improvement", "Sitcom",           "Skating", 
"Skiing",           "Soccer",           "Track/Field",      "True",  
"Volleyball",       "Wrestling" 
};


const char* GenreText(u8 type)
{
	if (type == 0xFF)
	  return "Not a Category";
	if (type >= 0xAE && type <= 0xFE)
	  return "Reserved (Detailed)";
	 
	return genres[type];
}


//////////////////////////////////////////////////////////////////////////////
