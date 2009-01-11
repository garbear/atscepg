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

#include <vdr/plugin.h>
#include <vdr/interface.h>

#include "config.h"
#include "setupMenu.h"
#include "scanner.h"

//////////////////////////////////////////////////////////////////////////////


cATSCSetupMenu::cATSCSetupMenu(void)
{
  newTimeZone  = config.timeZone;
  //newSetTime = config.setTime;
   
  Add(new cMenuEditIntItem("Time Zone",    &newTimeZone, -12, 13));  
  //Add(new cMenuEditBoolItem("Set system time", &newSetTime, "No", "Yes"));
  
  scan = new cOsdItem("Channel Scan...");
  Add(scan); 
}


//----------------------------------------------------------------------------

void cATSCSetupMenu::Store(void)
{
  SetupStore("timeZone", config.timeZone  = newTimeZone);
  //SetupStore("setTime",  config.setTime   = newSetTime);
}


//----------------------------------------------------------------------------

eOSState cATSCSetupMenu::ProcessKey(eKeys key)
{
  eOSState state = cOsdMenu::ProcessKey(key);
 
  if (key == kOk) {
    Store();
  }
    
  if (state == osUnknown && scan == Get(Current()) && key == kOk) {
    state = AddSubMenu(new cATSCScanner);
  } 
    
  return state;  
}


//////////////////////////////////////////////////////////////////////////////


