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

#include <stdarg.h>
#include <stdlib.h> 
#include <string.h>

#include "config.h"
#include "log.h"


#ifdef AE_ENABLE_LOG
Log Logger;
#endif

#define PREFIX "[ATSC] "
#define MAX_LOG_SIZE (256*1024)


//////////////////////////////////////////////////////////////////////////////


Log::Log(void)
{
  lastMsg = NULL;
  lastCount = 0;
  lastType = L_NONE;
  logfile = NULL;
  logfileSize = 0;
  disableLogfile = false;
}


//----------------------------------------------------------------------------

Log::~Log()
{
  free(lastMsg);

  if (logfile)
   fclose(logfile);
}


//----------------------------------------------------------------------------

void Log::Printf(LogType type, const char* msg, ...)
{
  if (!msg || !(type & config.logType)) return;
  
  char* output = NULL;
  va_list ap;
  va_start(ap, msg);
  vasprintf(&output, msg, ap);
  va_end(ap);
  
  lastMutex.Lock();
  
  if (lastMsg && strcmp(lastMsg, output)==0) // Repeated message
  {
    lastCount++;
  }
  else
  {
    if (lastCount) {
      char buff[64];
      snprintf(buff, sizeof(buff), "Last message repeated %d time%s.", lastCount, lastCount==1?"":"s");
      LogPrint(lastType, buff);
      lastCount = 0;
    }

    LogPrint(type, output);
    free(lastMsg);
    lastMsg = strdup(output);
    lastType = type;
  }
  
  lastMutex.Unlock();
  free(output); 
}


//----------------------------------------------------------------------------

void Log::LogPrint(LogType type, const char* msg)
{
  if (config.logConsole)
    ConsolePrint(type, msg);
  if (config.logSyslog)
    SyslogPrint(type, msg);
  if (config.logFile)
    FilePrint(type, msg);
}


//----------------------------------------------------------------------------

void Log::ConsolePrint(LogType type, const char* msg)
{
  if (type & L_ERR) 
    printf(PREFIX "\033[31m%s\033[0m\n", msg);
  else
    printf(PREFIX "%s\n", msg);
}

 
//---------------------------------------------------------------------------- 

void Log::SyslogPrint(LogType type, const char* msg)
{
  int pri=-1;
  switch (type)
  {
    case L_MSG : if (SysLogLevel > 1) pri = LOG_INFO; break;
    case L_ERR : if (SysLogLevel > 0) pri = LOG_ERR;  break; 
    case L_DBG : 
    case L_DBGV: 
    case L_DAT :
    case L_OTH :
    case L_VDR :
    case L_MGT :
    case L_VCT :
    case L_EIT :
    case L_ETT : if (SysLogLevel > 2) pri = LOG_DEBUG; break;
  }
  
  if(pri>=0)
    syslog(pri,"[%d] " PREFIX "%s", cThread::ThreadId(), msg);
}


//----------------------------------------------------------------------------

void Log::FilePrint(LogType type, const char* msg)
{
  logfileMutex.Lock();    
      
  if (!logfile && !disableLogfile)
  {
    logfile = fopen(config.logFileName, "a");
    if (logfile)
    {
      setlinebuf(logfile);
      logfileSize = ftell(logfile);
      if (logfileSize < 0) {
        logfileSize=0;
        fprintf(stderr, PREFIX "Cannot determine size of logfile '%s', assuming zero.\n", config.logFileName);
      }
      fprintf(stderr, PREFIX "Logfile '%s' opened.\n", config.logFileName);
    }
    else {
      disableLogfile = true;
      fprintf(stderr, PREFIX "Failed to open logfile '%s': %s.\n", config.logFileName, strerror(errno));
    }
  }
    
  if (logfile) 
  {
    time_t now = time(NULL);
    struct tm tm_r;
    strftime(timeStamp, sizeof(timeStamp), "%b %e %T", localtime_r(&now, &tm_r));
    
    int q = fprintf(logfile, "[%s] %s\n", timeStamp, msg);
    if(q > 0) logfileSize += q;

    if (logfileSize > MAX_LOG_SIZE)
    {
      fprintf(logfile,"[%s] %s\n", timeStamp, "Logfile closed, filesize limit reached.\n");
      fclose(logfile);
      logfile = NULL; 
      disableLogfile = false;
      char* newName;
      asprintf(&newName,"%s.old", config.logFileName);
      if(rename(config.logFileName, newName))
      {
        disableLogfile = true;
        fprintf(stderr, PREFIX "Failed to rotate logfile: %s.\n", strerror(errno));
        fprintf(stderr, PREFIX "Logging to file disabled!\n");
      }
      free(newName);
    }
  }
      
  logfileMutex.Unlock();
}


//----------------------------------------------------------------------------

void Log::ResetLogfile(void)
{
  logfileMutex.Lock();

  disableLogfile = false;
  if (logfile)
  {
    fclose(logfile);
    logfile = NULL;
  }
  
  logfileMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////////


