#include <vector>
#include "VolumeData.h"
#include "VolumeDataLoader.h"

void VolumeDataLoader::LoadRawVolumeFile(VolumeData* data, const char* filename, const glm::ivec3& size, const glm::vec3& spacing, int bytes_per_pixel)
{
	std::vector<unsigned char> tmp((size_t)size.x * (size_t)size.y * (size_t)size.z * (size_t)bytes_per_pixel);
	FILE* fp = fopen(filename, "rb");
	fread(tmp.data(), 1, tmp.size(), fp);
	fclose(fp);

	data->bytes_per_pixel = bytes_per_pixel;
	data->size = size;
	data->spacing = spacing;

	data->texture.load_memory(size.x, size.y, size.z, tmp.data(), bytes_per_pixel);
}



