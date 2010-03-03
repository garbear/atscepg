/*
 * Copyright (C) 2006-2010 Alex Lasnier <alex@fepg.org>
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
  
protected:
  virtual void Process(u_short Pid, u_char Tid, const u_char* Data, int length);
  virtual void SetStatus(bool On);
  
private:
  bool ProcessPAT(const uint8_t* data, int length);
  bool ProcessPMT(const uint8_t* data, int length);
  bool ProcessMGT(const uint8_t* data, int length);
  bool ProcessVCT(const uint8_t* data, int length);
  bool ProcessEIT(const uint8_t* data, int length, uint16_t Pid);
  bool ProcessETT(const uint8_t* data, int length);
  
  int GetMGTVersion(void);
  void SetMGTVersion(uint8_t version);
  cChannel* GetChannel(uint16_t sid) const;
  
  void ResetFilter(void);
  
  std::map<int, uint8_t> MGTVersions; // Should be shared between instances?
  uint8_t newMGTVersion;

  time_t lastScanMGT;
  time_t lastScanSTT;
  
  bool gotMGT;
  bool gotVCT;
  bool gotRRT;
  
  int fNum;
  int prevTransponder;
  
  std::list<uint16_t> channelSIDs;

  std::list<uint32_t> eitPids; // SID << 16 | PID
  std::list<uint16_t> ettEIDs;
  std::list<uint16_t> ettPids;
  
  SidTranslator sidTranslator;
  uint16_t currentTID;
};


//////////////////////////////////////////////////////////////////////////////


#endif //__ATSCFILTER_H
