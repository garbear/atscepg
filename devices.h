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

#ifndef __ATSC_DEVICES_H
#define __ATSC_DEVICES_H

#include <vdr/device.h>

#include "filter.h"


///////////////////////////////////////////////////////////////////////////////


class cAtscDevices
{
public:
  cAtscDevices(void);
 ~cAtscDevices();
  void Initialize(void);
  void StartFilters(void);
  void StopFilters(void);
  int NumDevices(void) const { return numDevices; }
  cDevice* GetDevice(int i) { return (i>=0 && i<numDevices) ? devices[i]->device : NULL; }
  const char* GetName(int i) { return (i>=0 && i<numDevices) ? devices[i]->name : NULL; }

private:  
  class DeviceInfo {
  public:
    DeviceInfo(int numDev, cDevice* dev, const char* nam);
    ~DeviceInfo();

    cATSCFilter* filter;
    cDevice* device;
    char* name;
  };
  DeviceInfo* devices[MAXDEVICES];
  int numDevices;
};


///////////////////////////////////////////////////////////////////////////////


extern cAtscDevices AtscDevices;


///////////////////////////////////////////////////////////////////////////////


#endif //__ATSC_DEVICES_H
