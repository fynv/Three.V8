#include <cstdio>
#include "FileLoader.h"

void LoadBinaryFile(const char* filename, std::vector<unsigned char>& data)
{
	FILE* fp = fopen(filename, "rb");
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	data.resize((size_t)size);
	fread(data.data(), 1, size, fp);
	fclose(fp);
}

void LoadTextFile(const char* filename, std::vector<char>& data)
{
	FILE* fp = fopen(filename, "rb");
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	data.resize((size_t)size + 1, 0);
	fread(data.data(), 1, size, fp);
	fclose(fp);
}