#ifndef __ATSCTOOLS_H
#define __ATSCTOOLS_H



// Disable this to remove debug messages
#define AE_DEBUG


#ifdef AE_DEBUG
	#define DEBUG_MSG(x...) fprintf(stderr, "ATSC EPG: ");fprintf(stderr, x);fprintf(stderr, "\n")
#else
	#define DEBUG_MSG(x...) 
#endif


#define tPrint(x...) fprintf(stderr, x);

  
#define u1  bool
#define u8  unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define uchar unsigned char



inline u16 get_u16(const u8* d) { return (d[0] << 8) | d[1]; }
inline u32 get_u24(const u8* d) { return (d[0] << 16) | (d[1] << 8) | d[2]; }
inline u32 get_u32(const u8* d) { return (d[0] << 24) | (d[1] << 16) | (d[2] << 8) | d[3]; } 




#endif //__ATSCTOOLS_H
