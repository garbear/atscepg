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

#include <string.h>
#include <vdr/tools.h>

#include "config.h"
#include "log.h"


///////////////////////////////////////////////////////////////////////////////


#define DEFAULT_LOG_FILE "/var/tmp/atscepg.log"

cATSCConfig config;


///////////////////////////////////////////////////////////////////////////////


cATSCConfig::cATSCConfig(void)
{
  //setTime  = 0;
  logType = L_DEFAULT;
  logConsole = true;
  logFile = false;
  logSyslog = false;
  logFileName = strdup(DEFAULT_LOG_FILE);
}


//----------------------------------------------------------------------------

cATSCConfig::~cATSCConfig()
{
  free(logFileName);
}


//----------------------------------------------------------------------------

bool cATSCConfig::SetupParse(const char* Name, const char* Value)
{
  //if (!strcasecmp(Name, "setTime"))  config.setTime = atoi(Value);
  if      (!strcasecmp(Name, "logType"))    logType    = atoi(Value);
  else if (!strcasecmp(Name, "logConsole")) logConsole = atoi(Value);
  else if (!strcasecmp(Name, "logFile"))    logFile    = atoi(Value);
  else if (!strcasecmp(Name, "logSyslog"))  logSyslog  = atoi(Value);
  else if (!strcasecmp(Name, "logFileName")) { free(logFileName); logFileName = strdup(Value); }
  else return false;
  
  return true;
}


///////////////////////////////////////////////////////////////////////////////



