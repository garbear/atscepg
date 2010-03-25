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


#include "tools.h"


//////////////////////////////////////////////////////////////////////////////


Utf16Converter Utf16;


//----------------------------------------------------------------------------

Utf16Converter::Utf16Converter(void)
{ 
  cd = iconv_open(cCharSetConv::SystemCharacterTable(), "UTF-16BE");
}


//----------------------------------------------------------------------------

Utf16Converter::~Utf16Converter()
{
  iconv_close(cd);
}


//----------------------------------------------------------------------------

void Utf16Converter::Convert(const char* in, size_t inSize, char* out, size_t outSize)
{
  char* inBuf = (char*) in;
  while (inSize > 0) 
  {
    if (iconv(cd, &inBuf, &inSize, &out, &outSize) == size_t(-1))
    {
      dprint(L_ERR, "ERROR converting from UTF-16");
      //TODO: more errors...
      if (errno == EILSEQ) { // A character can't be converted
        inBuf++;
        inSize--;
        *out++ = '?';
        outSize--;
      }
      else          
        break;
    }
  }
  *out = 0;
}


//////////////////////////////////////////////////////////////////////////////


