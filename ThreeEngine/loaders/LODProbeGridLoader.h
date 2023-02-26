#pragma once

class LODProbeGrid;
class LODProbeGridLoader
{
public:
	static void LoadFile(LODProbeGrid* probe_grid, const char* fn);
	static void LoadMemory(LODProbeGrid* probe_grid, unsigned char* data, size_t size);

};
