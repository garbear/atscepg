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

#include <stdarg.h>

#include <vdr/channels.h>
#include <vdr/device.h>
#include <vdr/plugin.h>

#include "scanner.h"
#include "devices.h"
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
  asprintf(&numberCmd, "%s/number", dir);
  
  file = NULL;
  device = NULL;
  currentFrequency = 0;
  devSelection = false;
  needsUpdate = false;
  
  if (AtscDevices.NumDevices() > 1) 
  {
    devSelection = true;
    cOsdMenu::Add(new cOsdItem("Select a device:", osUnknown, false));
    int n = AtscDevices.NumDevices();
    for (int i=0; i<n; i++)
      cOsdMenu::Add(new cOsdItem(AtscDevices.GetName(i)));
  }
  else {
    device = AtscDevices.GetDevice(0);
    Start();
  }

  Display();
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

  if (device == NULL) {
    AddLine("No ATSC device found");
  }
  else
  {
    for (uint32_t i=0; i<NUM_FREQ && Running(); i++)
    {
      SetTransponderData(c, ATSCFrequencies[i]);
      currentFrequency = ATSCFrequencies[i];

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
  
  VCT vct(Data, Length);
  
  if (!vct.CheckCRC()) {
    AddLine("\tReceived VCT with errors, check signal.");
    condWait.Signal();
    return;
  }
  
  AddLine("\tReceived VCT: found %d channels.", vct.NumberOfChannels());

  for (u32 i=0; i<vct.NumberOfChannels(); i++)
  {
    AtscChannel* ch = (AtscChannel*) vct.GetChannel(i);
    
    SetTransponderData(ch->VDRChannel(), currentFrequency);
  
    AddLine("\t%d.%d  %s", ch->MajorNumber(), ch->MinorNumber(), ch->ShortName());
    
    if (file)
    {
      int chanNum = Number(ch->MajorNumber(), ch->MinorNumber());
      if (chanNum == -1)
        continue;

      cString chanText = ch->VDRChannel()->ToText();
      if (chanNum)
        fprintf(file, ":@%d\n%s", chanNum, *chanText);
      else
        fprintf(file, "%s", *chanText);
    }
  }  

  condWait.Signal(); // We got the VCT don't let the thread wait for nothing!
}


//----------------------------------------------------------------------------

void cATSCScanner::SetTransponderData(cChannel* c, int frequency)
{
#if VDRVERSNUM < 10700      
  c->SetTerrTransponderData(cSource::stTerr, frequency, 999, 7, 999, 999, 999, 999, 999);
#elif VDRVERSNUM < 10702
  c->SetTerrTransponderData(cSource::stTerr, frequency, 999, MapToDriver(10, ModulationValues), 999, 999, 999, 999, 999, 0, 0);
#else
  c->SetTerrTransponderData(cSource::stTerr, frequency, 6000000, MapToDriver(10, ModulationValues), 999, 999, 999, 999, 999);
#endif 
}


//----------------------------------------------------------------------------

eOSState cATSCScanner::ProcessKey(eKeys Key)
{
  eOSState state = cOsdMenu::ProcessKey(Key);

  if (state == osUnknown)
  {
    state = osContinue;

    switch (Key)
    {
      case kBack:
        state = osBack;
        break;
        
      case kOk:
        if (devSelection) {
          device = AtscDevices.GetDevice(Current()-1);
          devSelection = false;
          Clear();
          Display();
          Start();
        }
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
        
      case kNone:
        if (needsUpdate) {
          Display();
          needsUpdate = false;
        }
      break;
       
      default:
        break;    
    }
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
  needsUpdate = true;
}


//----------------------------------------------------------------------------

void cATSCScanner::UpdateLastLine(const char* Text)
{
  char* buffer = NULL;
  asprintf(&buffer, "%s\t%s", Last()->Text(), Text);
  Last()->SetText(buffer, false); // false, so we don't need to free(buffer)
  needsUpdate = true;
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
    p.Close();
    
    if (result) {
      result[l] = 0;
      num = atoi(result);
    }
    else
      num = 0;
      
    free(result);
  }
  else
    dprint(L_ERR, "ERROR: can't open pipe for command '%s'", cmd);
 
  free(cmd);
  
  return num;
}


//////////////////////////////////////////////////////////////////////////////


