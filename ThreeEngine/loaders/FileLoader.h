#pragma once

#include <vector>

void LoadBinaryFile(const char* filename, std::vector<unsigned char>& data);
void LoadTextFile(const char* filename, std::vector<char>& data);

