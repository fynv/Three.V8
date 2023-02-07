#pragma once


class ProbeGrid;
class ProbeGridSaver
{
public:
	static void SaveFile(const ProbeGrid* image, const char* fn);
};