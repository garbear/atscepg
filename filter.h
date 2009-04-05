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

#ifndef __ATSCFILTER_H
#define __ATSCFILTER_H

#include <vector>
#include <map> 
#include <list>

#include <vdr/filter.h>
#include <vdr/device.h>

#include "vdrInterface.h"
 

//////////////////////////////////////////////////////////////////////////////


class cATSCFilter : public cFilter
{  
public:
  cATSCFilter(int num);
  virtual ~cATSCFilter();
  
  void Attach(cDevice* Device);
  void Detach(void);
  
protected:
  virtual void Process(u_short Pid, u_char Tid, const u_char* Data, int Length);
  virtual void SetStatus(bool On);
  
private:
  bool ProcessPAT(const uint8_t* data);
  bool ProcessPMT(const uint8_t* data);
  
  bool ProcessMGT(const uint8_t* data);
  bool ProcessVCT(const uint8_t* data);
  bool ProcessEIT(const uint8_t* data, uint16_t Pid);
  bool ProcessETT(const uint8_t* data);
  
  int GetMGTVersion(void);
  void SetMGTVersion(uint8_t version);

  std::map<int, uint8_t> MGTVersions; // Should be shared between instances?
  
  cDevice* attachedDevice;

  MGT* mgt;

  time_t lastScanMGT;
  time_t lastScanSTT;
   
  bool gotVCT;
  bool gotRRT;
  int fNum;
  
  VDRInterface vdrInterface;
  
  std::vector<u16> channelSIDs;

  std::list<uint32_t> eitPids; // SID << 16 | PID
  std::list<uint16_t> ettEIDs;
  std::list<uint16_t> ettPids;
  
#ifdef AE_DEBUG
  void dfprint(uint16_t type, const char* msg, ...);
#else
  void dfprint(uint16_t type, const char* msg, ...) { }
#endif
};


//////////////////////////////////////////////////////////////////////////////


#endif //__ATSCFILTER_H
