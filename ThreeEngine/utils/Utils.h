#pragma once

#include <cstdint>
#include <chrono>
#include <cstdio>

inline uint64_t time_micro_sec()
{
	std::chrono::time_point<std::chrono::system_clock> tpSys = std::chrono::system_clock::now();
	std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds> tpMicro
		= std::chrono::time_point_cast<std::chrono::microseconds>(tpSys);
	return tpMicro.time_since_epoch().count();
}

inline uint64_t time_milli_sec()
{
	return (time_micro_sec() + 500) / 1000;
}

inline double time_sec()
{
	return (double)time_micro_sec() / 1000000.0;
}

inline bool exists_test(const char* name)
{
	if (FILE *file = fopen(name, "r"))
	{
		fclose(file);
		return true;
	}
	else
	{
		return false;
	}
}

inline bool writable_test(const char* name)
{
	if (FILE* file = fopen(name, "a"))
	{
		fclose(file);
		return true;
	}
	else
	{
		return false;
	}
}