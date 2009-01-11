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
 
  if (state == osUnknown && scan == Get(Current()) && key == kOk) {
    state = AddSubMenu(new cATSCScanner);
  }  
    
  return state;  
}


//////////////////////////////////////////////////////////////////////////////


