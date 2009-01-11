#ifndef __ATSCTYPES_H
#define __ATSCTYPES_H

#include "tools.h"


//////////////////////////////////////////////////////////////////////////////


const char* TableTypeText(u16 type);

u8 TableTypeToTID(u16 type);

const char* ModulationModeText(u8 type);

const char* ETMLocationText(u8 type, u8 tableID);

const char* ServiceTypeText(u8 type);

const char* DescriptorText(u8 type);

const char* GenreText(u8 type);


// AC-3 Related
const char* SampleRateText(u8 type);
const char* BitRateText(u8 type);
const char* SurroundModeText(u8 type);
const char* NumberOfChannelsText(u8 type);


//////////////////////////////////////////////////////////////////////////////


#endif //__ATSCTYPES_H
