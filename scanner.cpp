#include <stdarg.h>
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
#define FILE_NAME   "channels.conf"


//////////////////////////////////////////////////////////////////////////////


cATSCScanner::cATSCScanner(void) : cOsdMenu("ATSC Channel Scan", 10, 16, 10), 
                                   cThread("ATSC Scanner")
{
  dprint(L_DBGV, "ATSC Scanner Created.");
  SetHelp("Cancel");
  
	Set(0x1FFB, 0xC8); // VCT-T
	Set(0x1FFB, 0xC9); // VCT-C
	
	dir = cPlugin::ConfigDirectory("atscepg");
	asprintf(&numberCmd, "%s/number", cPlugin::ConfigDirectory("atscepg"));
	
	file = NULL;
	currentFrequency = 0;
	
  Display();
  Start(); 
}


//----------------------------------------------------------------------------

cATSCScanner::~cATSCScanner(void) 
{
  Cancel(TIMEOUT+500);
  free(numberCmd);
  
  dprint(L_DBGV, "ATSC Scanner Destroyed.");
}


//----------------------------------------------------------------------------

void cATSCScanner::Action(void)
{
  dprint(L_DBGV, "ATSC Scanner thread started.");
  
  char* fn = NULL;
  asprintf(&fn, "%s/%s", dir, FILE_NAME);
  file = fopen(fn, "w");
  free(fn);
  if (file == NULL) AddLine("Could not open output file.");
   
  cChannel* c = new cChannel();
  
#if VDRVERSNUM < 10700    
  c->SetTerrTransponderData(cSource::stTerr, 0, 999, 7, 999, 999,999, 999, 999);
#elif VDRVERSNUM < 10702
  c->SetTerrTransponderData(cSource::stTerr, 0, 999, MapToDriver(10, ModulationValues), 999, 999,999, 999, 999, 0, 0);
#else
  c->SetTerrTransponderData(cSource::stTerr, 0, 999, MapToDriver(10, ModulationValues), 999, 999,999, 999, 999);
#endif 

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

#if VDRVERSNUM < 10700      
      c->SetTerrTransponderData(cSource::stTerr, ATSCFrequencies[i], 999, 7, 999, 999, 999, 999, 999);
#elif VDRVERSNUM < 10702
      c->SetTerrTransponderData(cSource::stTerr, ATSCFrequencies[i], 999, MapToDriver(10, ModulationValues), 999, 999, 999, 999, 999, 0, 0);
#else
      c->SetTerrTransponderData(cSource::stTerr, ATSCFrequencies[i], 999, MapToDriver(10, ModulationValues), 999, 999, 999, 999, 999);
#endif  
 

      AddLine("Tuning:\t%d Hz", ATSCFrequencies[i]);
      
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
  AddLine("\tReceived VCT: found %d channels.", vct.NumberOfChannels());

  for (u32 i=0; i<vct.NumberOfChannels(); i++)
	{
	  const ::Channel* ch = vct.GetChannel(i);

		const char* name = ch->Name(); 
		AddLine("\t%d.%d  %s", ch->majorChannelNumber, ch->minorChannelNumber, name);
		
		if (file) {
		  int chanNum = Number(ch->majorChannelNumber, ch->minorChannelNumber);
		  if (chanNum == -1)
		    continue;
		  else if (chanNum != 0) {
		    fprintf(file, ":@%d\n", chanNum);
		  }
		  
		  char c = (Tid == 0xC9) ? 'C' : 'T';
#if VDRVERSNUM < 10700 		  
      fprintf(file, "%s:%d:M8:%c:0:", name, currentFrequency, c);
#else
      fprintf(file, "%s:%d:A0M10P0:%c:0:", name, currentFrequency, c);
#endif
      if (ch->PCR_PID && ch->PCR_PID != ch->vPID)
        fprintf(file, "%d+%d:", ch->vPID, ch->PCR_PID);
      else
        fprintf(file, "%d:", ch->vPID);
        
      fprintf(file, "0;%d:0:0:%d:0:%d:0\n", ch->aPID, ch->source_id, vct.TID());  
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
          AddLine("Scan cancelled");
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
  char* buffer = NULL;
  asprintf(&buffer, "%s\t%s", Last()->Text(), Text);
  Last()->SetText(buffer, false); // false, so we don't need to free(buffer)
  Display();
}


//----------------------------------------------------------------------------

int cATSCScanner::Number(uint16_t major, uint16_t minor)
{  
  char* result = NULL;
  char* cmd = NULL;
  int num = 0;
  
  asprintf(&cmd, "%s %d %d", numberCmd, major, minor);
  
  cPipe p;
  if (p.Open(cmd, "r"))
  {
    int l = 0;
    int c;
    while ((c = fgetc(p)) != EOF) {
      if (l % 20 == 0)
        result = (char*) realloc(result, l + 21);
      result[l++] = c;
    }
    if (result)
      result[l] = 0;
    p.Close();
    
    num = atoi(result);
    free(result);
  }
  else
    dprint(L_ERR, "ERROR: can't open pipe for command '%s'", cmd);
 
  free(cmd);
  
  return num;
}


//////////////////////////////////////////////////////////////////////////////


