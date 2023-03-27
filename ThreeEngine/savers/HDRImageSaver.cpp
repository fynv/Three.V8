#include "HDRImageSaver.h"
#include "utils/HDRImage.h"

#include "stb_image_write.h"

void HDRImageSaver::SaveFile(const HDRImage* image, const char* fn)
{
	stbi_write_hdr(fn, image->width(), image->height(), 3, image->data());
}

