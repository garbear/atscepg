#ifndef _ATSC_HUFFMAN_H_
#define _ATSC_HUFFMAN_H_

#include <unistd.h>
#include <string>

#include "ATSCTools.h"


//////////////////////////////////////////////////////////////////////////////


#define uint unsigned int


std::string atsc_huffman1_to_string(const u8* compressed, u32 size, u32 table);

std::string uncompressed(const u8* data, u8 number_bytes, u8 mode);  


//////////////////////////////////////////////////////////////////////////////


#endif //_ATSC_HUFFMAN_H_
