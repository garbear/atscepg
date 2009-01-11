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
  static cATSCFilter* Instance(void);
  static void Destroy(void);

  static int Frequency(void) { return instance->currentChannel->Frequency(); }
  
  void Attach(cDevice* Device, const cChannel* channel);
  void Detach(void);
  
protected:
  virtual void Process(u_short Pid, u_char Tid, const u_char* Data, int Length);
  virtual void SetStatus(bool On);
  
private:
  cATSCFilter(void);
 ~cATSCFilter(void);

  void ProcessMGT(const uint8_t* data);
  void ProcessVCT(const uint8_t* data);
  void ProcessEIT(const uint8_t* data, uint16_t Pid);
  void ProcessETT(const uint8_t* data);
  
  int GetMGTVersion(void) const;
  void SetMGTVersion(uint8_t version);
  
  std::map<int, uint8_t> MGTVersions;
  
  static cATSCFilter* instance;
  cDevice* attachedDevice;
  const cChannel* currentChannel;

  MGT* mgt;

  time_t lastScanMGT;
  time_t lastScanSTT;
   
  bool gotVCT;
  bool gotRRT;
   
  VDRInterface vdrInterface;
  
  std::vector<u16> channelSIDs;

  std::list<uint32_t> eitPids; // SID << 16 | PID
  std::list<uint16_t> ettEIDs;
  std::list<uint16_t> ettPids;

};


//////////////////////////////////////////////////////////////////////////////


#endif //__ATSCFILTER_H
