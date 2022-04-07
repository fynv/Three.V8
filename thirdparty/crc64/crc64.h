#ifndef _crc64_h
#define _crc64_h

#include <cstdint>

uint64_t crc64(uint64_t crc, const unsigned char *s, uint64_t l);

#endif