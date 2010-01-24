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
