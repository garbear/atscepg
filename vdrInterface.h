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

#ifndef __VDRINTERFACE_H
#define __VDRINTERFACE_H


#include  <vdr/channels.h>

#include "tables.h"
#include "tools.h"
#include "structs.h"


//////////////////////////////////////////////////////////////////////////////


class VDRInterface
{
public:
  VDRInterface() { stt = NULL; }
 ~VDRInterface() { delete stt; }
  
	bool AddEventsToSchedule(const EIT& eit);
	void AddChannels(const VCT& vct);
	bool AddDescription(const ETT& ett);
	void UpdateSTT(const u8* data); 
	
private:
  cChannel* GetChannel(u16 s_id, u8 table_id) const;
	cEvent* CreateVDREvent(const Event& event) const;
  void DisplayChannelInfo(const Channel* ch, u8 table_id) const;
	time_t GPStoSystem(time_t gps) const;

	u16 currentTID;	
	STT* stt;
};


//////////////////////////////////////////////////////////////////////////////


#endif //__VDRINTERFACE_H
