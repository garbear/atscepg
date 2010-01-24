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
 
#ifndef __ATSC_LOG_H
#define __ATSC_LOG_H

#include <stdint.h>

#include <vdr/tools.h>
#include <vdr/thread.h>


// Remove this to disable debug messages
#define AE_ENABLE_LOG


enum LogType {
  L_NONE = 0x0000,
  L_MSG  = 0x0001,
  L_ERR  = 0x0002,
  L_DBG  = 0x0004,
  L_DBGV = 0x0008,
  L_DAT  = 0x0010,
  L_OTH  = 0x0020,
  L_VDR  = 0x0040,
        // 0x0080
  L_MGT  = 0x0100,
  L_VCT  = 0x0200,
  L_EIT  = 0x0400,
  L_ETT  = 0x0800,

  L_ALL  = 0xFFFF
};
 
#define L_DEFAULT (L_MSG | L_ERR)


//////////////////////////////////////////////////////////////////////////////


class Log
{
public:
  Log(void);
  ~Log();

  void Printf(LogType, const char* msg, ...);
  void ResetLogfile(void);
  
private:
  void LogPrint(LogType type, const char* msg);
  void ConsolePrint(LogType type, const char* msg);
  void FilePrint(LogType type, const char* msg);
  void SyslogPrint(LogType type, const char* msg);

  char* lastMsg;
  int lastCount;
  LogType lastType;
  cMutex lastMutex;
  
  FILE* logfile;
  char timeStamp[32];
  long long logfileSize;
  bool disableLogfile;
  cMutex logfileMutex;
};


#ifdef AE_ENABLE_LOG

extern Log Logger;

#define dprint(t,...)  Logger.Printf((t),__VA_ARGS__)

#else

#define dprint(type, msg, ...) ;

#endif


//////////////////////////////////////////////////////////////////////////////


#endif //__ATSC_LOG_H
