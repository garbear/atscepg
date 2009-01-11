#include <stdarg.h>

#include <vdr/channels.h>
#include <vdr/device.h>
#include <vdr/plugin.h>

#include "scanner.h"
#include "tools.h"
#include "tables.h"
#include "structs.h"


//////////////////////////////////////////////////////////////////////////////


#define TIMEOUT     1000
#define VCT_TIMEOUT 5000
#define FILE_NAME   "channels-atsc.conf"


//////////////////////////////////////////////////////////////////////////////


cATSCScanner::cATSCScanner(void) : cOsdMenu("ATSC Channel Scan", 10, 16, 10), 
                                   cThread("ATSC Scanner")
{
  dprint(L_DBGV, "ATSC Scanner Created.");
  SetHelp("Cancel");
  
	Set(0x1FFB, 0xC8); // VCT-T
	Set(0x1FFB, 0xC9); // VCT-C
	
	file = NULL;
	currentFrequency = 0;
	
  Display();
  Start(); 
}


//----------------------------------------------------------------------------

cATSCScanner::~cATSCScanner(void) 
{
  Cancel(TIMEOUT+500);
  dprint(L_DBGV, "ATSC Scanner Destroyed.");
}


//----------------------------------------------------------------------------

void cATSCScanner::Action(void)
{
  dprint(L_DBGV, "ATSC Scanner thread started.");
  
  char* fn = NULL;
  asprintf(&fn, "%s/%s", cPlugin::ConfigDirectory(), FILE_NAME);
  file = fopen(fn, "w");
  free(fn);
  if (file == NULL) AddLine("Could not open output file.");
   
  cChannel* c = new cChannel();
  c->SetTerrTransponderData(cSource::stTerr, 0, 999, 7, 999, 999,999, 999, 999);

#if VDRVERSNUM < 10500  
  cDevice* device = cDevice::GetDevice(c);
#else
  cDevice* device = cDevice::GetDevice(c, -1, false);
#endif

  if (device == NULL) {
    AddLine("No ATSC device found");
  }
  else
  {
    for (uint32_t i=0; i<NUM_FREQ && Running(); i++)
    {
      delete c; 
      c = new cChannel();
      currentFrequency = ATSCFrequencies[i];
      c->SetTerrTransponderData(cSource::stTerr, ATSCFrequencies[i], 999, 7, 999, 999, 999, 999, 999);

      AddLine("Tunning:\t%d Hz", ATSCFrequencies[i]);
      
      
      device->SwitchChannel(c, false);
      bool lock = device->HasLock(TIMEOUT); 
      if (!lock) {
        UpdateLastLine("Failed");
        continue; 
      }
  
      UpdateLastLine("Success");
      
      gotVCT = false; 
      device->AttachFilter(this);
      condWait.Wait(VCT_TIMEOUT); // Let the filter do its thing
      device->Detach(this);
      if (!gotVCT) AddLine("\tNo channels found");
    }
  }
  
  delete c;
  if (file) {
    AddLine("Saved to file: %s", FILE_NAME);  
    fclose(file);
  }  
  dprint(L_DBGV, "ATSC Scanner thread ended.");
}


//----------------------------------------------------------------------------

void cATSCScanner::Process(u_short Pid, u_char Tid, const u_char* Data, int Length)
{ 
  if (gotVCT) return;
  
  gotVCT = true;
  
  VCT vct(Data);
  AddLine("\tReceived VCT: found %d channels.", vct.getNumChannels());

  for (u32 i=0; i<vct.getNumChannels(); i++)
	{
	  ::Channel ch( vct.getChannel(i) );

		const char* name = ch.short_name.c_str(); 
		AddLine("\t%d.%d  %s", ch.majorChannelNumber, ch.minorChannelNumber, name);
		
		if (file) {
		  char c = (Tid == 0xC9) ? 'C' : 'T';
      fprintf(file, "%s:%d:M8:%c:0:", name, currentFrequency, c);
      if (ch.PCR_PID && ch.PCR_PID != ch.vPID)
        fprintf(file, "%d+%d:", ch.vPID, ch.PCR_PID);
      else
        fprintf(file, "%d:", ch.vPID);
        
      fprintf(file, "0;%d:0:0:%d:0:%d:0\n", ch.aPID, ch.source_id, vct.getTID());  
		}
  }	
  
  condWait.Signal(); // We got the VCT don't let the thread wait for nothing!

}


//----------------------------------------------------------------------------

eOSState cATSCScanner::ProcessKey(eKeys Key)
{
  eOSState state = cOsdMenu::ProcessKey(Key);

  if (state == osUnknown)
    switch (Key)
    {
      case kBack:
        state = osBack;
        break;
      case kRed:
        if (Running()) {
          condWait.Signal(); // If we are waiting for a VCT, stop.
          Cancel(TIMEOUT+500);
          AddLine("Scan canceled");
          state = osContinue;
        }
        else
          state = osBack;
        break;
      default:
        break;    
    }
    
  return state;  
}


//----------------------------------------------------------------------------

void cATSCScanner::AddLine(const char* Text, ...) 
{
  char* buffer = NULL;
  va_list ap;
  va_start(ap, Text);
  vasprintf(&buffer, Text, ap);
  va_end(ap);

  cOsdItem* item = new cOsdItem(buffer);
  free(buffer);

  cOsdMenu::Add(item);
  CursorDown();
  Display();
}


//----------------------------------------------------------------------------

void cATSCScanner::UpdateLastLine(const char* Text)
{
  cOsdItem* last = Last();
  char* buffer = NULL;
  asprintf(&buffer, "%s\t%s", last->Text(), Text);
  cOsdMenu::Del(last->Index()); last = NULL;
  
  cOsdItem* item = new cOsdItem(buffer);
  free(buffer);

  cOsdMenu::Add(item);
  CursorDown();
  Display();
}


//////////////////////////////////////////////////////////////////////////////


