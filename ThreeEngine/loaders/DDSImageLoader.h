#pragma once

#include <cstdio>
#include <cstdint>

class DDSImage;
struct DDSHeader;
struct DDSHeader10;
class DDSImageLoader
{
public:
	static void LoadFile(DDSImage* image, const char* fn);
	static void LoadMemory(DDSImage* image, unsigned char* data, size_t size);

private:
	static void _read_bgra(DDSImage* image, FILE* fp, DDSHeader& header, DDSHeader10& header10);
	static void _read_blocks(DDSImage* image, FILE* fp, DDSHeader& header, DDSHeader10& header10);

	static void _read_bgra(DDSImage* image, unsigned char* &ptr, DDSHeader& header, DDSHeader10& header10);
	static void _read_blocks(DDSImage* image, unsigned char* &ptr, DDSHeader& header, DDSHeader10& header10);

};