#ifndef ___ATSCSETUPMENU_H
#define ___ATSCSETUPMENU_H

#include <vdr/plugin.h>


class cATSCSetupMenu : public cMenuSetupPage 
{
private:
  int  newTimeZone;
  //int  newSetTime;
  
protected:
  virtual void Store(void);
  
public:
  cATSCSetupMenu(void);
};


#endif //___ATSCSETUPMENU_H
