#pragma once


class ProbeGrid;
class ProbeGridSaver
{
public:
	static void SaveFile(ProbeGrid* image, const char* fn);
};