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

#ifndef __ATSC_CONFIG_H
#define __ATSC_CONFIG_H


///////////////////////////////////////////////////////////////////////////////


class cATSCConfig
{
public:
  cATSCConfig(void);
 ~cATSCConfig();
 
  bool SetupParse(const char* Name, const char* Value);
  
  //int setTime;
  int logType;
  int logConsole;
  int logFile;
  int logSyslog;
  char* logFileName;
};


///////////////////////////////////////////////////////////////////////////////


extern cATSCConfig config;


///////////////////////////////////////////////////////////////////////////////


#endif //__ATSC_CONFIG_H
