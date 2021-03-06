/*
 * Copyright (C) 2006-2010 Alex Lasnier <alex@fepg.org>
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
#include <algorithm>

#include <linux/dvb/frontend.h>
#include <libsi/section.h>
#include <libsi/descriptor.h>
#include <vdr/channels.h>
#include <vdr/device.h>
#include <vdr/plugin.h>

#include "scanner.h"
#include "devices.h"
#include "tools.h"
#include "tables.h"
#include "structs.h"
#include "frequencies.h"


//////////////////////////////////////////////////////////////////////////////


#define TIMEOUT        1000
#define FILTER_TIMEOUT 5000
#define FILE_NAME      "channels.conf"

static const char* const ignoreEncStrings[2] = { "Include encrypted", "Ignore encrypted" };


//////////////////////////////////////////////////////////////////////////////


cATSCScanner::cATSCScanner(void) : cOsdMenu("ATSC Channel Scan", 12),
                                   cThread("ATSC Scanner")
{
  dprint(L_DBGV, "ATSC Scanner Created.");
  SetHelp("Cancel");
  
  Set(0x1FFB, 0xC8, 0xFE); // VCT-T/C
  
  dir = cPlugin::ConfigDirectory("atscepg");
  asprintf(&numberCmd, "%s/number", dir);
  
  file = NULL;
  deviceNum = 0;
  modulation = 0;
  currentFrequency = 0;
  devSelection = true;
  ignoreEncrypted = true;
  
  if (AtscDevices.NumDevices() == 0) {
    devSelection = false;
    AddLine("No ATSC device found");
  }
  else
  {
    int n = AtscDevices.NumDevices();
    for (int i=0; i<n; i++)
      deviceNames[i] = AtscDevices.GetName(i);
    cOsdMenu::Add(new cMenuEditStraItem("Device", &deviceNum, n, deviceNames));
    cOsdMenu::Add(new cMenuEditStraItem("Modulation", &modulation, NUM_FREQ_TYPES, Frequencies_Name));
    cOsdMenu::Add(new cMenuEditStraItem("Channels", &ignoreEncrypted, 2, ignoreEncStrings));
    cOsdMenu::Add(new cOsdItem("Start scan..."));
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
  dprint(L_DBG, "ATSC Scanner thread started.");
  
  char* fn = NULL;
  asprintf(&fn, "%s/%s", dir, FILE_NAME);
  file = fopen(fn, "w");
  free(fn);
  if (!file) {
    AddLine("Could not open output file.");
    return;
  }
   
  cChannel* c = new cChannel();
  cDevice* device = AtscDevices.GetDevice(deviceNum);
  const int* frequencies = Frequencies_List[modulation];
  unsigned int frequenciesNum = Frequencies_Size[modulation];
  
  bool doPMTscan = (modulation == 1);
  
  if (doPMTscan)
    Set(0x0000, 0x00); // Do PAT/PMT scan for cable
  
  int prevChan = -1;
  if (device == cDevice::ActualDevice())
    prevChan = cDevice::CurrentChannel();

  for (uint32_t i=0; i<frequenciesNum && Running(); i++)
  {
    currentFrequency = frequencies[i];
    SetTransponderData(c);
    
    AddLine("Tuning:\t%d Hz", frequencies[i]);
    
    device->SwitchChannel(c, false);
    bool lock = device->HasLock(TIMEOUT); 
    if (!lock) {
      UpdateLastLine("Failed");
      dprint(L_DBG, "Tuning: %d Hz (Failed)", frequencies[i]);
      continue; 
    }

    UpdateLastLine("Success");
    dprint(L_DBG, "Tuning: %d Hz (Success)", frequencies[i]);
     
    gotVCT = false;
    gotPAT = false;
    gotPMT = doPMTscan ? false : true;
    
    device->AttachFilter(this);
    condWait.Wait(FILTER_TIMEOUT); // Let the filter do its thing
    device->Detach(this);
    
    if ((!gotVCT && !doPMTscan) || (!gotVCT && !gotPMT)) {
      AddLine("\tNo channels found");
      dprint(L_DBG, "No channels found");
    }
  }  
  delete c;

  AddLine("Saved to file: %s", FILE_NAME);  
  fclose(file);

  if (prevChan > 0)
    Channels.SwitchTo(prevChan);
  
  dprint(L_DBG, "ATSC Scanner thread ended.");
}


//----------------------------------------------------------------------------

void cATSCScanner::Process(u_short Pid, u_char Tid, const u_char* Data, int Length)
{
  switch (Tid)
  {
    case 0x00:
      if (!gotPAT)
        ProcessPAT(Data, Length);
      break;
    case 0x02:
      if (ProcessPMT(Data, Length))
        cFilter::Del(Pid, 0x02);
      break;
    case 0xC8:
    case 0xC9:
      if (!gotVCT)
        ProcessVCT(Tid, Data, Length);
      break;
  }

  if (gotVCT && gotPMT)
    condWait.Signal(); // We got the VCT don't let the thread wait for nothing!
}


//----------------------------------------------------------------------------

void cATSCScanner::ProcessVCT(u_char Tid, const u_char* Data, int Length)
{  
  gotVCT = true;
  
  VCT vct(Data, Length);
  
  if (!vct.CheckCRC()) {
    AddLine("\tReceived VCT with errors, check signal.");
    dprint(L_DBG, "Received VCT-%c with errors", Tid==0xC8?'T':'C');
    condWait.Signal();
    return;
  }
  
  AddLine("\tReceived VCT: found %d channels.", vct.NumberOfChannels());
  dprint(L_DBG, "Received VCT-%c found %d channels.", Tid==0xC8?'T':'C', vct.NumberOfChannels());

  for (u32 i=0; i<vct.NumberOfChannels(); i++)
  {
    AtscChannel* ch = (AtscChannel*) vct.GetChannel(i);
    
    SetTransponderData(ch->VDRChannel());
  
    AddLine("\t%d.%d  %s", ch->MajorNumber(), ch->MinorNumber(), ch->ShortName());
    dprint(L_DBG, "  %d.%d  %s", ch->MajorNumber(), ch->MinorNumber(), ch->ShortName());
    
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
}


//----------------------------------------------------------------------------

void cATSCScanner::ProcessPAT(const u_char* data, int Length)
{
  SI::PAT pat(data, false);
  if (!pat.CheckCRCAndParse())
    return;
  
  tsid = pat.getTransportStreamId();
  
  pmtSIDs.clear();
  int numChan = 0;
  SI::PAT::Association assoc;
  for (SI::Loop::Iterator it; pat.associationLoop.getNext(assoc, it); )
    if (!assoc.isNITPid()) {
      pmtSIDs.push_back(assoc.getServiceId());
      cFilter::Add(assoc.getPid(), 0x02);
      numChan++;
    }
  
  AddLine("\tReceived PAT: found %d channels.", numChan);
  dprint(L_DBG, "Received PAT: found %d channels.", numChan);
  
  gotPAT = true;
}

//----------------------------------------------------------------------------

bool cATSCScanner::ProcessPMT(const u_char* data, int Length)
{
  SI::PMT pmt(data, false);
  if (!pmt.CheckCRCAndParse())
    return false;
  
  std::list<uint16_t>::iterator itr = find(pmtSIDs.begin(), pmtSIDs.end(), pmt.getServiceId());
  if (itr == pmtSIDs.end()) // Not found
    return false;
  //dprint(L_DBG, "Received PMT for SID %d", pmt.getServiceId());
  
  int Vpid  = 0;
  int Vtype = 0;
  int Ppid  = 0;
  int Apids[1] = { 0 };
  int Atypes[1] = { 0 };
  char ALangs[1][MAXLANGCODE2] = { "" };
  int Spids[1] = { 0 };
  char SLangs[1][MAXLANGCODE2] = { "" };
  int Dpids[MAXDPIDS + 1] = { 0 };
  int Dtypes[MAXDPIDS + 1] = { 0 };
  char DLangs[MAXDPIDS][MAXLANGCODE2] = { "" };
  int NumDpids = 0;
  int NumCaIds = 0;
  int CaIds[MAXCAIDS + 1] = { 0 };
   
  SI::PMT::Stream stream;
  for (SI::Loop::Iterator it; pmt.streamLoop.getNext(stream, it); ) 
  {
    int esPid = stream.getPid();
    switch (stream.getStreamType())
    {
      case 0x02:
      case 0x1B:
        Vpid = esPid;
        Ppid = pmt.getPCRPid();
        Vtype = stream.getStreamType();
      break;
      
      case 0x05:
      case 0x06:
      break;
        
      case 0x81: {
        char lang[MAXLANGCODE1] = { 0 };
        SI::Descriptor *d;
        for (SI::Loop::Iterator it; (d = stream.streamDescriptors.getNext(it)); ) {
          switch (d->getDescriptorTag()) {
            case SI::ISO639LanguageDescriptorTag: {
              SI::ISO639LanguageDescriptor *ld = (SI::ISO639LanguageDescriptor *)d;
              strn0cpy(lang, I18nNormalizeLanguageCode(ld->languageCode), MAXLANGCODE1);
            }
            break;
            default: ;
          }
          delete d;
        }
        if (NumDpids < MAXDPIDS) {
          Dpids[NumDpids] = esPid;
          Dtypes[NumDpids] = SI::AC3DescriptorTag;
          strn0cpy(DLangs[NumDpids], lang, MAXLANGCODE1);
          NumDpids++;
        }
      }
      break;
        
      default:
        dprint(L_DBG, "PMT unhandled stream type 0x%02X.", stream.getStreamType());
    }
          
    SI::CaDescriptor* d;
    for (SI::Loop::Iterator it; (d = (SI::CaDescriptor*) stream.streamDescriptors.getNext(it, SI::CaDescriptorTag)); ) 
    {
      if (NumCaIds < MAXCAIDS) {
        int caId = d->getCaType();
        bool add = true;
        for (int i = 0; add && i < NumCaIds; i++)
          if (CaIds[i] == caId)
            add = false;
        if (add)    
          CaIds[NumCaIds++] = caId;
      }        
      delete d;
    }
  } 
  
  if (!ignoreEncrypted || NumCaIds == 0)
  {
    cChannel channel;
    SetTransponderData(&channel);
    channel.SetId(0, tsid, pmt.getServiceId());
#if VDRVERSNUM < 10715
    channel.SetPids(Vpid, Ppid, Vtype, Apids, ALangs, Dpids, DLangs, Spids, SLangs, 0);
#else
    channel.SetPids(Vpid, Ppid, Vtype, Apids, Atypes, ALangs, Dpids, Dtypes, DLangs, Spids, SLangs, 0);
#endif
    channel.SetCaIds(CaIds);
    if (file)
      fprintf(file, "%s", *(channel.ToText()));
  }
    
  pmtSIDs.erase(itr);
  if (pmtSIDs.empty()) {
    //dprint(L_DBG, "Got all PMTs");
    gotPMT = true;
  }
  return true;
}


//----------------------------------------------------------------------------

void cATSCScanner::SetTransponderData(cChannel* c)
{
  cDvbTransponderParameters dtp;
  dtp.SetModulation((modulation == 0) ? VSB_8 : QAM_256);
  dtp.SetInversion(INVERSION_AUTO);    
  c->SetTransponderData(cSource::stAtsc, currentFrequency, 0, dtp.ToString('A'));
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
        if (devSelection && Current() == 3) {
          devSelection = false;
          Clear();
          SetCols(10, 16, 10);
          Display();
          Start();
        }
        break;        
        
      case kRed:
        if (Running()) {
          condWait.Signal(); // If we are waiting for a VCT, stop.
          Cancel(TIMEOUT+500);
          AddLine("Scan cancelled");
          dprint(L_DBG, "Scan cancelled");
          state = osContinue;
        }
        else
          state = osBack;
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
}


//----------------------------------------------------------------------------

void cATSCScanner::UpdateLastLine(const char* Text)
{
  if (!Last())
    return;

  char* buffer = NULL;
  asprintf(&buffer, "%s\t%s", Last()->Text(), Text);
  Last()->SetText(buffer, false); // false, so we don't need to free(buffer)
  DisplayCurrent(true);
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


