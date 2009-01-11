#include "tools.h"


#ifdef AE_DEBUG

#include <stdarg.h>
#include <stdlib.h> 

static uint8_t logType = 0;

void setLogType(uint8_t type) { logType = type; }

void dprint(uint8_t type, const char* msg, ...)
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



