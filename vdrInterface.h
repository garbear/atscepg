#ifndef __VDRINTERFACE_H
#define __VDRINTERFACE_H


#include  <vdr/channels.h>

#include "ATSCTables.h"
#include "ATSCTools.h"
#include "structs.h"


//////////////////////////////////////////////////////////////////////////////


class VDRInterface
{
public:
  VDRInterface() { stt = NULL; }
 ~VDRInterface() { if (stt) delete stt; }
  
	bool addEventsToSchedule(EIT& eit);
	void addChannels(VCT& vct);
	bool addDescription(ETT& ett);
	void updateSTT(const u8* data); 
	
private:
	u16 currentTID;
  cChannel* getChannel(u16 s_id, u8 table_id) const;
	cEvent* createVDREvent(Event& event);
  void displayChannelInfo(Channel& ch, u8 table_id) const;
	time_t GPStoSystem(time_t gps);
	
	STT* stt;
};


//////////////////////////////////////////////////////////////////////////////


#endif //__VDRINTERFACE_H
