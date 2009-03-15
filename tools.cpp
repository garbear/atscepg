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

#include "tools.h"


#ifdef AE_DEBUG

#include <stdarg.h>


#include <stdlib.h> 

static uint16_t logType = 0;

void SetLogType(uint16_t type) { logType = type; }

void dprint(uint16_t type, const char* msg, ...)
{
  if (!msg) return;
  
  if (!(type & logType)) return; // no logging for this type
  
  char* output = NULL;
  va_list ap;
  va_start(ap, msg);
  vasprintf(&output, msg, ap);
  va_end(ap);
   
  if (type == L_ERR)  
    fprintf(stderr, "[ATSC] \033[31m%s\n\033[0m", output);
  else if (type == L_DAT)
    fprintf(stderr, "%s\n", output);   
  else 
    fprintf(stderr, "[ATSC] %s\n", output);
     
  fflush(stderr);
  free(output);  
}

#endif // AE_DEBUG



