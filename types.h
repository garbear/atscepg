#ifndef __ATSCTYPES_H
#define __ATSCTYPES_H

#include "tools.h"


//////////////////////////////////////////////////////////////////////////////


const char* tableType(u16 type);

u8 tableID(u16 type);

const char* modulationMode(u8 type);

const char* ETMLocation(u8 type, u8 tableID);

const char* serviceType(u8 type);

const char* descriptor(u8 type);

const char* genre(u8 type);


// AC-3 Related
const char* sampleRate(u8 type);
const char* bitRate(u8 type);
const char* surroundMode(u8 type);
const char* numberOfChannels(u8 type);


//////////////////////////////////////////////////////////////////////////////


#endif //__ATSCTYPES_H
