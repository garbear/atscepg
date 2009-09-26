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
#include "log.h"
#include "setupMenu.h"
#include "scanner.h"


//////////////////////////////////////////////////////////////////////////////


struct LogParameters {
  const int value;
  const char* name;
  int enabled;
};

LogParameters logParameters[] = {
  { L_MSG , "User Messages" },
  { L_ERR , "Errors" },
  { L_DBG , "Debug" },
  { L_DBGV, "Debug (verbose)" },
  { L_DAT , "Data" },
  { L_OTH , "Miscellaneous" },
  { L_VDR , "VDR Interface" },
  { L_MGT , "MGT" },
  { L_VCT , "VCT" },
  { L_EIT , "EIT" },
  { L_ETT , "ETT" },
  { 0 }
};


//////////////////////////////////////////////////////////////////////////////


cATSCSetupMenu::cATSCSetupMenu(void)
{
  //newSetTime = config.setTime;
  newLogConsole = config.logConsole;
  newLogFile    = config.logFile;
  newLogSyslog  = config.logSyslog;
  strncpy(newLogFileName, config.logFileName, sizeof(newLogFileName));
  
  //Add(new cMenuEditBoolItem("Set system time", &newSetTime, "No", "Yes"));
  
  Add(scan = new cOsdItem("Channel Scan..."));
  AddEmptyLine();
/*
  AddCategory("Devices");
  
  AddEmptyLine();
*/  
#ifdef AE_ENABLE_LOG  
  AddCategory("Logging");
  Add(new cMenuEditBoolItem("Log to console", &newLogConsole));
  Add(new cMenuEditBoolItem("Log to syslog", &newLogSyslog));
  Add(new cMenuEditBoolItem("Log to file", &newLogFile));
  Add(new cMenuEditStrItem("Log file", newLogFileName, sizeof(newLogFileName)));
  AddEmptyLine();
  
  LogParameters* lp = logParameters;
  while (lp && lp->value)
  {
    lp->enabled = config.logType & lp->value;
    Add(new cMenuEditBoolItem(lp->name, &(lp->enabled)));
    lp++;
  }
#endif

  SetCols(20);
}


//----------------------------------------------------------------------------

void cATSCSetupMenu::Store(void)
{
  //SetupStore("setTime",  config.setTime   = newSetTime);
#ifdef AE_ENABLE_LOG   
  int newLogType = 0;
  const LogParameters* lp = logParameters;
  while (lp && lp->value)
  {
    if (lp->enabled)
      newLogType |= lp->value;
    lp++;
  }
  
  SetupStore("logType",     config.logType    = newLogType);
  SetupStore("logConsole",  config.logConsole = newLogConsole);
  SetupStore("logFile",     config.logFile    = newLogFile);
  SetupStore("logSyslog",   config.logSyslog  = newLogSyslog);
  SetupStore("logFileName", newLogFileName);
  
  char* oldName = strdup(config.logFileName);
  free(config.logFileName);
  config.logFileName = strdup(newLogFileName);
  if (strcmp(oldName, newLogFileName) != 0 && config.logFile)
    Logger.ResetLogfile();
  free(oldName);
#endif
}


//----------------------------------------------------------------------------

eOSState cATSCSetupMenu::ProcessKey(eKeys key)
{
  eOSState state = cOsdMenu::ProcessKey(key);

  if (state == osUnknown && key == kOk)
  {
    Store();
    if (scan == Get(Current()))
      state = AddSubMenu(new cATSCScanner);
    else
      state = osBack;
  }
   
  return state;  
}


//----------------------------------------------------------------------------

void cATSCSetupMenu::AddCategory(const char *Title) 
{
  char* buffer = NULL;
  asprintf(&buffer, "--- %s -------------------------------------------------"
   		              "---------------", Title );
  cOsdItem* item = new cOsdItem(buffer);
  free(buffer);
	item->SetSelectable(false);
  Add(item);
}


//----------------------------------------------------------------------------

void cATSCSetupMenu::AddEmptyLine(void)
{
  cOsdItem* item = new cOsdItem("");
  item->SetSelectable(false);
  Add(item);
}


//////////////////////////////////////////////////////////////////////////////


