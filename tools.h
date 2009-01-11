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

#ifndef __ATSCTOOLS_H
#define __ATSCTOOLS_H

#include <stdint.h>
#include <stdio.h>

// Disable this to remove debug messages
#define AE_DEBUG

#ifdef AE_DEBUG

void SetLogType(uint8_t type);

void dprint(uint8_t type, const char* msg, ...);

#else

#define dprint(type, msg, args...)
#define setLogType(x)

#endif //AE_DEBUG

// Log Types 
#define L_NONE 0x00
#define L_MSG  0x01
#define L_ERR  0x02
#define L_DBG  0x04
#define L_DBGV 0x08
#define L_DAT  0x10
#define L_OTH  0x20
#define L_ALL  0xFF
  
#define u1    bool
#define u8    uint8_t
#define u16   uint16_t
#define u32   uint32_t
#define uchar uint8_t



inline u16 get_u16(const u8* d) { return (d[0] << 8) | d[1]; }
inline u32 get_u24(const u8* d) { return (d[0] << 16) | (d[1] << 8) | d[2]; }
inline u32 get_u32(const u8* d) { return (d[0] << 24) | (d[1] << 16) | (d[2] << 8) | d[3]; } 


#endif //__ATSCTOOLS_H
