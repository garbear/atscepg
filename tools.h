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

#ifndef __ATSC_TOOLS_H
#define __ATSC_TOOLS_H

#include <iconv.h>
#include <stdint.h>
#include <stdio.h>

#include "log.h"


//////////////////////////////////////////////////////////////////////////////


#define u1    bool
#define u8    uint8_t
#define u16   uint16_t
#define u32   uint32_t
#define uchar uint8_t


static inline u16 get_u16(const u8* d) { return (d[0] << 8) | d[1]; }
static inline u32 get_u24(const u8* d) { return (d[0] << 16) | (d[1] << 8) | d[2]; }
static inline u32 get_u32(const u8* d) { return (d[0] << 24) | (d[1] << 16) | (d[2] << 8) | d[3]; } 


//////////////////////////////////////////////////////////////////////////////


class Utf16Converter
{
public:
  Utf16Converter(void);
 ~Utf16Converter();
 
  void Convert(const char* in, size_t inSize, char* out, size_t outSize);

private:
  iconv_t cd;
};


extern Utf16Converter Utf16;


//////////////////////////////////////////////////////////////////////////////

#endif //__ATSC_TOOLS_H
