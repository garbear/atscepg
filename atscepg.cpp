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

#include <vdr/plugin.h>
#include <vdr/status.h>

#include "devices.h"
#include "config.h"
#include "setupMenu.h"


#if VDRVERSNUM < 10714
#error This version of ATSC EPG requires VDR 1.7.14 or above.
#endif


//////////////////////////////////////////////////////////////////////////////


static const char* VERSION        = "0.3.0-hg";
static const char* DESCRIPTION    = "Adds event info for ATSC broadcasts";
static const char* MAINMENUENTRY  =  NULL; 


//////////////////////////////////////////////////////////////////////////////


class cPluginAtscepg : public cPlugin
{
private:


public:
  cPluginAtscepg(void);
  virtual ~cPluginAtscepg();
  virtual const char* Version(void) { return VERSION; }
  virtual const char* Description(void) { return DESCRIPTION; }
  virtual const char* CommandLineHelp(void);
  virtual bool ProcessArgs(int argc, char *argv[]);
  virtual bool Initialize(void);
  virtual bool Start(void);
  virtual void Stop(void);
  virtual void Housekeeping(void);
  virtual void MainThreadHook(void);
  virtual cString Active(void);
  virtual const char* MainMenuEntry(void) { return MAINMENUENTRY; }
  virtual cOsdObject* MainMenuAction(void);
  virtual cMenuSetupPage* SetupMenu(void);
  virtual bool SetupParse(const char* Name, const char* Value);
  virtual bool Service(const char* Id, void* Data = NULL);
  virtual const char** SVDRPHelpPages(void);
  virtual cString SVDRPCommand(const char* Command, const char* Option, int& ReplyCode);
};


//////////////////////////////////////////////////////////////////////////////


cPluginAtscepg::cPluginAtscepg(void)
{
  // Initialize any member variables here.
  // DON'T DO ANYTHING ELSE THAT MAY HAVE SIDE EFFECTS, REQUIRE GLOBAL
  // VDR OBJECTS TO EXIST OR PRODUCE ANY OUTPUT!
}


//----------------------------------------------------------------------------

cPluginAtscepg::~cPluginAtscepg()
{
  // Clean up after yourself!
}


//----------------------------------------------------------------------------

const char *cPluginAtscepg::CommandLineHelp(void)
{
  // Return a string that describes all known command line options.
  return NULL;
}


//----------------------------------------------------------------------------

bool cPluginAtscepg::ProcessArgs(int argc, char *argv[])
{
  // Implement command line argument processing here if applicable.
  return true;
}


//----------------------------------------------------------------------------

bool cPluginAtscepg::Initialize(void)
{
  // Initialize any background activities the plugin shall perform.
  return true;
}


//----------------------------------------------------------------------------

bool cPluginAtscepg::Start(void)
{
  // Start any background activities the plugin shall perform.
  AtscDevices.Initialize();
  AtscDevices.StartFilters();
  return true;
}


//----------------------------------------------------------------------------

void cPluginAtscepg::Stop(void)
{
  // Stop any background activities the plugin shall perform.
  AtscDevices.StopFilters();
}


//----------------------------------------------------------------------------

void cPluginAtscepg::Housekeeping(void)
{
  // Perform any cleanup or other regular tasks.
}


//----------------------------------------------------------------------------

void cPluginAtscepg::MainThreadHook(void)
{
  // Perform actions in the context of the main program thread.
  // WARNING: Use with great care - see PLUGINS.html!
}


//----------------------------------------------------------------------------

cString cPluginAtscepg::Active(void)
{
  // Return a message string if shutdown should be postponed
  return NULL;
}


//----------------------------------------------------------------------------

cOsdObject *cPluginAtscepg::MainMenuAction(void)
{
  // Perform the action when selected from the main VDR menu.
  return NULL;
}


//----------------------------------------------------------------------------

cMenuSetupPage *cPluginAtscepg::SetupMenu(void)
{
  // Return a setup menu in case the plugin supports one.
  return new cATSCSetupMenu;
}


//----------------------------------------------------------------------------

bool cPluginAtscepg::SetupParse(const char *Name, const char *Value)
{
  // Parse your own setup parameters and store their values.
  return config.SetupParse(Name, Value);
}


//----------------------------------------------------------------------------

bool cPluginAtscepg::Service(const char *Id, void *Data)
{
  // Handle custom service requests from other plugins
  return false;
}


//----------------------------------------------------------------------------

const char **cPluginAtscepg::SVDRPHelpPages(void)
{
  // Return help text for SVDRP commands this plugin implements
  return NULL;
}


//----------------------------------------------------------------------------

cString cPluginAtscepg::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode)
{
  // Process SVDRP commands this plugin implements
  return NULL;
}

  
//////////////////////////////////////////////////////////////////////////////


VDRPLUGINCREATOR(cPluginAtscepg); // Don't touch this!


