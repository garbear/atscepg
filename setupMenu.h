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

#ifndef ___ATSCSETUPMENU_H
#define ___ATSCSETUPMENU_H

#include <vdr/plugin.h>


class cATSCSetupMenu : public cMenuSetupPage 
{
public:
  cATSCSetupMenu(void);

protected:
  virtual void Store(void);
  virtual eOSState ProcessKey(eKeys key);
  
  void AddCategory(const char *Title);
  void AddEmptyLine(void);  

private:
  cOsdItem* scan;
  //int  newSetTime;
  int newLogConsole;
  int newLogFile;
  int newLogSyslog;
  char newLogFileName[128];
};


#endif //___ATSCSETUPMENU_H
