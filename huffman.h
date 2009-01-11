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

#ifndef _ATSC_HUFFMAN_H_
#define _ATSC_HUFFMAN_H_

#include <unistd.h>
#include <string>

#include "tools.h"


//////////////////////////////////////////////////////////////////////////////


#define uint unsigned int


std::string ATSCHuffman1toString(const u8* compressed, u32 size, u32 table);

std::string Uncompressed(const u8* data, u8 number_bytes, u8 mode);  


//////////////////////////////////////////////////////////////////////////////


#endif //_ATSC_HUFFMAN_H_
