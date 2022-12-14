#pragma once

#include <glm.hpp>

class VolumeData;
class VolumeDataLoader
{
public:
	static void LoadRawVolumeFile(VolumeData* data, const char* filename, const glm::ivec3& size, const glm::vec3& spacing, int bytes_per_pixel);

};
