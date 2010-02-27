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

#ifndef __ATSC_SCANNER_H
#define __ATSC_SCANNER_H


#include <vdr/device.h>
#include <vdr/filter.h>
#include <vdr/osdbase.h>


//////////////////////////////////////////////////////////////////////////////


class cATSCScanner : public cOsdMenu, public cThread, public cFilter
{
public:
  cATSCScanner(void);
  virtual ~cATSCScanner();
 
  virtual eOSState ProcessKey(eKeys Key);
 
protected:
  virtual void Action(void);

private:
  void AddLine(const char* Text, ...); 
  void UpdateLastLine(const char* Text);
  int Number(uint16_t major, uint16_t minor);
  void SetTransponderData(cChannel* c, int frequency);
  
  virtual void Process(u_short Pid, u_char Tid, const u_char *Data, int Length);
  
  cCondWait condWait;
  bool gotVCT;
  bool devSelection;
  int currentFrequency;
  const char* dir;
  char* numberCmd;
  FILE* file;
  int deviceNum;
  int modulation;
  const char* deviceNames[MAXDEVICES];
};


//////////////////////////////////////////////////////////////////////////////


#endif //__ATSC_SCANNER_H
