/*
 * atscepg.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <vdr/plugin.h>
#include <vdr/status.h>

#include "ATSCFilter.h"
#include "config.h"
#include "ATSCSetupMenu.h"

static const char *VERSION        = "0.0.2";
static const char *DESCRIPTION    = "Adds event info for ATSC broadcasts";
static const char *MAINMENUENTRY  =  NULL; 


//////////////////////////////////////////////////////////////////////////////


class cPluginAtscepg : public cPlugin, cStatus {
private:
  // Add any member variables or functions you may need here.
  cATSCFilter* myATSCFilter;
  int lastChannel;
  
public:
  cPluginAtscepg(void);
  virtual ~cPluginAtscepg();
  virtual const char *Version(void) { return VERSION; }
  virtual const char *Description(void) { return DESCRIPTION; }
  virtual const char *CommandLineHelp(void);
  virtual bool ProcessArgs(int argc, char *argv[]);
  virtual bool Initialize(void);
  virtual bool Start(void);
  virtual void Stop(void);
  virtual void Housekeeping(void);
  virtual void MainThreadHook(void);
  virtual cString Active(void);
  virtual const char *MainMenuEntry(void) { return MAINMENUENTRY; }
  virtual cOsdObject *MainMenuAction(void);
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
  virtual bool Service(const char *Id, void *Data = NULL);
  virtual const char **SVDRPHelpPages(void);
  virtual cString SVDRPCommand(const char *Command, const char *Option, int &ReplyCode);
  
protected:
  virtual void ChannelSwitch(const cDevice* Device, int ChannelNumber);
  };



cPluginAtscepg::cPluginAtscepg(void)
{
  // Initialize any member variables here.
  // DON'T DO ANYTHING ELSE THAT MAY HAVE SIDE EFFECTS, REQUIRE GLOBAL
  // VDR OBJECTS TO EXIST OR PRODUCE ANY OUTPUT!
  
  myATSCFilter = NULL;
  lastChannel = -1;

  config.timeZone = -5;
  //config.setTime  = 0;
}

cPluginAtscepg::~cPluginAtscepg()
{
  // Clean up after yourself!
  if (myATSCFilter) delete myATSCFilter;
}

const char *cPluginAtscepg::CommandLineHelp(void)
{
  // Return a string that describes all known command line options.
  return NULL;
}

bool cPluginAtscepg::ProcessArgs(int argc, char *argv[])
{
  // Implement command line argument processing here if applicable.
  return true;
}

bool cPluginAtscepg::Initialize(void)
{
  // Initialize any background activities the plugin shall perform.
  myATSCFilter = new cATSCFilter;
  
  return true;
}

bool cPluginAtscepg::Start(void)
{
  // Start any background activities the plugin shall perform.
  return true;
}

void cPluginAtscepg::Stop(void)
{
  // Stop any background activities the plugin shall perform.
}

void cPluginAtscepg::Housekeeping(void)
{
  // Perform any cleanup or other regular tasks.
}

void cPluginAtscepg::MainThreadHook(void)
{
  // Perform actions in the context of the main program thread.
  // WARNING: Use with great care - see PLUGINS.html!
}

cString cPluginAtscepg::Active(void)
{
  // Return a message string if shutdown should be postponed
  return NULL;
}

cOsdObject *cPluginAtscepg::MainMenuAction(void)
{
  // Perform the action when selected from the main VDR menu.
  return NULL;
}

cMenuSetupPage *cPluginAtscepg::SetupMenu(void)
{
  // Return a setup menu in case the plugin supports one.
  return new cATSCSetupMenu;
}

bool cPluginAtscepg::SetupParse(const char *Name, const char *Value)
{
  // Parse your own setup parameters and store their values.
  if      (!strcasecmp(Name, "timeZone"))   config.timeZone  = atoi(Value);
  //else if (!strcasecmp(Name, "setTime"))  config.setTime = atoi(Value);
  
  else return false;
  
  return true;
}

bool cPluginAtscepg::Service(const char *Id, void *Data)
{
  // Handle custom service requests from other plugins
  return false;
}

const char **cPluginAtscepg::SVDRPHelpPages(void)
{
  // Return help text for SVDRP commands this plugin implements
  return NULL;
}

cString cPluginAtscepg::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode)
{
  // Process SVDRP commands this plugin implements
  return NULL;
}


void cPluginAtscepg::ChannelSwitch(const cDevice *Device, int ChannelNumber)
{ 
  if (ChannelNumber)
  {
    cChannel* c = Channels.GetByNumber(ChannelNumber); 
    if (c) {
    	if (c->Modulation() == 7/*VSB_8*/ && ChannelNumber != lastChannel)
    	{
    	  lastChannel = ChannelNumber;
    	  myATSCFilter->Attach(Device);
    	  DEBUG_MSG("ATSC (8-VSB) Channel Detected");
    	}
  	  
  	  //DEBUG_MSG("Channel Switched to %d \t ID: %s \t Mod: %d", ChannelNumber, (const char *) (c->GetChannelID()).ToString(), c->Modulation() );
  	}
  	
  }
  else
  {
  	myATSCFilter->Detach(Device);
  }
}
 
  
sATSCConfig config;


VDRPLUGINCREATOR(cPluginAtscepg); // Don't touch this!
