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
 
#include "filterManager.h"
#include "filter.h"


///////////////////////////////////////////////////////////////////////////////


cFilterManager FilterManager;


///////////////////////////////////////////////////////////////////////////////


cFilterManager::cFilterManager(void)
{
  numFilters = 0;
}


//----------------------------------------------------------------------------

cFilterManager::~cFilterManager()
{

}


//----------------------------------------------------------------------------

void cFilterManager::AddFilter(const cATSCFilter* filter)
{
  if (numFilters < MAXDEVICES)
  {
    map[numFilters].filter = filter;
    map[numFilters].ts     = -1;
    numFilters++;
  }
}


//----------------------------------------------------------------------------

bool cFilterManager::Set(const cATSCFilter* filter, int transponder)
{
  cMutexLock lock(&mutex);
  
  for (int i=0; i<numFilters; i++)
    if (map[i].ts == transponder && map[i].filter != filter)
    { // Another filter is updating this transponder
      return false;
    }
    
  for (int i=0; i<numFilters; i++)
    if (map[i].filter == filter) {
      map[i].ts = transponder;
      break;
    }
    
  return true;
}


//----------------------------------------------------------------------------

void cFilterManager::Reset(const cATSCFilter* filter)
{
  cMutexLock lock(&mutex);
  
  for (int i=0; i<numFilters; i++)
    if (map[i].filter == filter) {
      map[i].ts = -1;
      return;
    }
}


//----------------------------------------------------------------------------

int cFilterManager::GetMgtVersion(int transponder)
{
  cMutexLock lock(&mutex);
  
  std::map<int,uint8_t>::const_iterator itr = MGTVersions.find(transponder);
  if (itr == MGTVersions.end()) // Key not found
    return -1;

  return itr->second;
}


//----------------------------------------------------------------------------

void cFilterManager::SetMgtVersion(int transponder, uint8_t version)
{
  cMutexLock lock(&mutex);
  MGTVersions[transponder] = version;
}


///////////////////////////////////////////////////////////////////////////////



