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

#include <sys/ioctl.h>
#include <linux/dvb/frontend.h>

#include "devices.h"


///////////////////////////////////////////////////////////////////////////////


cAtscDevices AtscDevices;


///////////////////////////////////////////////////////////////////////////////


cAtscDevices::cAtscDevices(void)
{
  numDevices = 0;
  for (int i=0; i<MAXDEVICES; i++)
    devices[i] = NULL;
}


//----------------------------------------------------------------------------

cAtscDevices::~cAtscDevices()
{
  for (int i=0; i<numDevices; i++)
    delete devices[i];
}


//----------------------------------------------------------------------------

void cAtscDevices::Initialize(void)
{
  int n = cDevice::NumDevices();
  for (int i=0; i<n; i++) 
  {
    cString dev = cString::sprintf("/dev/dvb/adapter%d/frontend%d", i, 0);
    int fe = open(dev, O_RDONLY | O_NONBLOCK);
    if (fe < 0)
      continue;

    struct dvb_frontend_info frontendInfo;
    if (ioctl(fe, FE_GET_INFO, &frontendInfo) >= 0)
      if (frontendInfo.type == FE_ATSC) 
      {
        dprint(L_MSG, "Found ATSC device (#%d) %s", i, frontendInfo.name);
        devices[numDevices] = new DeviceInfo(numDevices+1, cDevice::GetDevice(i), frontendInfo.name);
        numDevices++;
      }
  }
  
  dprint(L_MSG, "Found %d ATSC device%s", numDevices, numDevices==1?"":"s");
}


//----------------------------------------------------------------------------

void cAtscDevices::StartFilters(void)
{
  for (int i=0; i<numDevices; i++)
    devices[i]->filter->Attach(devices[i]->device);
}


//----------------------------------------------------------------------------

void cAtscDevices::StopFilters(void)
{
  for (int i=0; i<numDevices; i++)
    devices[i]->filter->Detach(); 
}


//----------------------------------------------------------------------------
 
cAtscDevices::DeviceInfo::DeviceInfo(int numDev, cDevice* dev, const char* nam)
{
  filter = new cATSCFilter(numDev);
  device = dev;
  name = strdup(nam);
}


//----------------------------------------------------------------------------

cAtscDevices::DeviceInfo::~DeviceInfo()
{
  delete filter;
  free(name);
}


///////////////////////////////////////////////////////////////////////////////
