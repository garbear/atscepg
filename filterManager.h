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

#ifndef __ATSC_FILTER_MANAGER_H
#define __ATSC_FILTER_MANAGER_H

#include <map> 

#include <vdr/device.h>
#include <vdr/thread.h>


///////////////////////////////////////////////////////////////////////////////

class cATSCFilter;

///////////////////////////////////////////////////////////////////////////////


class cFilterManager
{
public:
  cFilterManager(void);
 ~cFilterManager();
 
  void AddFilter(const cATSCFilter* filter);
  
  bool Set(const cATSCFilter* filter, int transponder);
  void Reset(const cATSCFilter* filter);
  
  int  GetMgtVersion(int transponder);
  void SetMgtVersion(int transponder, uint8_t version);
  
private:
  std::map<int, uint8_t> MGTVersions;
  
  struct FilterPair {
    const cATSCFilter* filter;
    int ts;
  };
  
  FilterPair map[MAXDEVICES];
  int numFilters;
  
  cMutex mutex;
};


///////////////////////////////////////////////////////////////////////////////


extern cFilterManager FilterManager;


///////////////////////////////////////////////////////////////////////////////


#endif //__ATSC_FILTER_MANAGER_H
