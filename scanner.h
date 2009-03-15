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

#ifndef __ATSC_SCANNER_H
#define __ATSC_SCANNER_H


#include <vdr/osdbase.h>
#include <vdr/filter.h>


//////////////////////////////////////////////////////////////////////////////


class cATSCScanner : public cOsdMenu, public cThread, public cFilter
{
public:
  cATSCScanner(void);
  virtual ~cATSCScanner();
 
  virtual eOSState ProcessKey(eKeys Key);
 
protected:
  virtual void Action(void);

private:
  void AddLine(const char* Text, ...); 
  void UpdateLastLine(const char* Text);
  int Number(uint16_t major, uint16_t minor);
  void SetTransponderData(cChannel* c, int frequency);
  
  virtual void Process(u_short Pid, u_char Tid, const u_char *Data, int Length);
  
  cCondWait condWait;
  bool gotVCT;
  int currentFrequency;
  const char* dir;
  char* numberCmd;
  FILE* file;
};


//////////////////////////////////////////////////////////////////////////////


static const int ATSCFrequencies[] = 
{
 57028615,
 63028615,
 69028615,
 79028615,
 85028615,
177028615,
183028615,
189028615,
195028615,
201028615,
207028615,
213028615,
473028615,
479028615,
485028615,
491028615,
497028615,
503028615,
509028615,
515028615,
521028615,
527028615,
533028615,
539028615,
545028615,
551028615,
557028615,
563028615,
569028615,
575028615,
581028615,
587028615,
593028615,
599028615,
605028615,
611028615,
617028615,
623028615,
629028615,
635028615,
641028615,
647028615,
653028615,
659028615,
665028615,
671028615,
677028615,
683028615,
689028615,
695028615,
701028615,
707028615,
713028615,
719028615,
725028615,
731028615,
737028615,
743028615,
749028615,
755028615,
761028615,
767028615,
773028615,
779028615,
785028615,
791028615,
797028615,
803028615
};


#define NUM_FREQ (sizeof(ATSCFrequencies)/sizeof(int))


//////////////////////////////////////////////////////////////////////////////


#endif //__ATSC_SCANNER_H
