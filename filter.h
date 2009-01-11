#ifndef __ATSCFILTER_H
#define __ATSCFILTER_H

#include <vector>
#include <map> 
#include <list>

#include <vdr/filter.h>
#include <vdr/device.h>

#include "vdrInterface.h"
 

//////////////////////////////////////////////////////////////////////////////


class cATSCFilter : public cFilter
{  
public:
  static cATSCFilter* Instance(void);
  static void Destroy(void);

  static int Frequency(void) { return instance->currentChannel->Frequency(); }
  
  void Attach(cDevice* Device, const cChannel* channel);
  void Detach(void);
  
protected:
  virtual void Process(u_short Pid, u_char Tid, const u_char* Data, int Length);
  virtual void SetStatus(bool On);
  
private:
  cATSCFilter(void);
 ~cATSCFilter(void);

  static cATSCFilter* instance;
  cDevice* attachedDevice;
  const cChannel* currentChannel;

  std::map<int, uint8_t> MGTVersions;
  int getMGTVersion(void) const;
  void setMGTVersion(uint8_t version);

  MGT* mgt;

  time_t lastScanMGT;
  time_t lastScanSTT;
   
  bool gotVCT;
  bool gotRRT;
   
  VDRInterface vdrInterface;
  
  std::vector<u16> channelSIDs;

  std::list<uint32_t> eitPids; // SID << 16 | PID
  std::list<uint16_t> ettEIDs;
  std::list<uint16_t> ettPids;

};


//////////////////////////////////////////////////////////////////////////////


#endif //__ATSCFILTER_H
