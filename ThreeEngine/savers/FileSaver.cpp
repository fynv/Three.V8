#include <cstdio>
#include <cstring>
#include "FileSaver.h"

void SaveBinaryFile(const char* filename, const unsigned char* data, size_t size)
{
	FILE* fp = fopen(filename, "wb");	
	fwrite(data, 1, size, fp);
	fclose(fp);
}
void SaveTextFile(const char* filename, const char* data)
{
	size_t size = strlen(data);
	FILE* fp = fopen(filename, "wb");
	fwrite(data, 1, size, fp);
	fclose(fp);
}
