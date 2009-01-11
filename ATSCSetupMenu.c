#include <vdr/plugin.h>
#include <vdr/interface.h>

#include "config.h"
#include "ATSCSetupMenu.h"


cATSCSetupMenu::cATSCSetupMenu(void)
{
  newTimeZone  = config.timeZone;
  //newSetTime = config.setTime;
   
  Add(new cMenuEditIntItem("Time Zone",    &newTimeZone, -12, 13));  
  //Add(new cMenuEditBoolItem("Set system time", &newSetTime, "No", "Yes"));
}


void cATSCSetupMenu::Store(void)
{
  SetupStore("timeZone", config.timeZone  = newTimeZone);
  //SetupStore("setTime",  config.setTime   = newSetTime);
}
