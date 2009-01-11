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
