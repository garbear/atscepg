#ifndef __VDRINTERFACE_H
#define __VDRINTERFACE_H


#include  <vdr/channels.h>

#include "tables.h"
#include "tools.h"
#include "structs.h"


//////////////////////////////////////////////////////////////////////////////


class VDRInterface
{
public:
  VDRInterface() { stt = NULL; }
 ~VDRInterface() { delete stt; }
  
	bool AddEventsToSchedule(const EIT& eit);
	void AddChannels(const VCT& vct);
	bool AddDescription(const ETT& ett);
	void UpdateSTT(const u8* data); 
	
private:
  cChannel* GetChannel(u16 s_id, u8 table_id) const;
	cEvent* CreateVDREvent(const Event& event) const;
  void DisplayChannelInfo(const Channel* ch, u8 table_id) const;
	time_t GPStoSystem(time_t gps) const;

	u16 currentTID;	
	STT* stt;
};


//////////////////////////////////////////////////////////////////////////////


#endif //__VDRINTERFACE_H
