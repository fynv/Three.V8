#pragma once

class ProbeGrid;
class ProbeGridLoader
{
public:
	static void LoadFile(ProbeGrid* probe_grid, const char* fn);
	static void LoadMemory(ProbeGrid* probe_grid, unsigned char* data, size_t size);

};