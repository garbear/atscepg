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

#ifndef __VDRINTERFACE_H
#define __VDRINTERFACE_H

#include  <vdr/channels.h>
#include  <vdr/epg.h>

#include "tables.h"
#include "tools.h"
#include "structs.h"


//////////////////////////////////////////////////////////////////////////////


class VDRInterface
{
public:
  static bool AddEvents(cChannel* channel, const EIT& eit);
  static bool AddDescription(cChannel* channel, const ETT& ett);

private:
  static void ToVDREvent(const Event* event, cEvent* vdrEvent);
  static cEvent* CreateVDREvent(const Event* event);
  static time_t GPStoLocal(time_t gps); 
};


///////////////////////////////////////////////////////////////////////////////


#endif //__VDRINTERFACE_H
