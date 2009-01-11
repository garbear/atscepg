#ifndef __ATSCFILTER_H
#define __ATSCFILTER_H

#include <vector>

#include <vdr/filter.h>
#include <vdr/device.h>

#include "vdrInterface.h"
   

//////////////////////////////////////////////////////////////////////////////


class cATSCFilter : public cFilter
{
private: 
  MGT* mgt;

  time_t lastScanMGT;
  time_t lastScanSTT;
   
  bool gotVCT;
  bool gotRRT;
            
  cDevice* attachedDevice;
  cDevice* getDevice(void);
   
  VDRInterface vdrInterface;
  
  std::vector<u16> channelSIDs;
  std::vector<PTS> pts;
  std::vector<u16> eitPids;
  std::vector<u16> ettPids; 
  
public:
  cATSCFilter();
 ~cATSCFilter();
  void Attach(const cDevice* Device = NULL);
  void Detach(const cDevice* Device);
  
protected:
  virtual void Process(u_short Pid, u_char Tid, const u_char *Data, int Length);
  virtual void SetStatus(bool On);
  
private:
	void addE_T(u16 pid, u8 tid, u16 sid, u16 eid=0);
	bool delE_T(u16 pid, u8 tid, u16 sid, u16 eid=0);
	
	u16 getETTPidFromEIT(u16 eit_pid);
};


//////////////////////////////////////////////////////////////////////////////


#endif //__ATSCFILTER_H
