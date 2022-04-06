#pragma once

class Image;
class ImageLoader
{
public:
	static void LoadFile(Image* image, const char* fn, bool keep_alpha = false);
};

