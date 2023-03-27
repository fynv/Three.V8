#pragma once

class HDRImage;
class HDRImageSaver
{
public:
	static void SaveFile(const HDRImage* image, const char* fn);
};

