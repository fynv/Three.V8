#pragma once

#include <glm.hpp>
#include "renderers/GLUtils.h"

class VolumeData
{
public:
	GLTexture3D texture;
	int bytes_per_pixel = 1;
	glm::ivec3 size = glm::ivec3(0,0,0);
	glm::vec3 spacing = glm::vec3(0.001f, 0.001f, 0.001f);

	void GetMinMax(glm::vec3& min_pos, glm::vec3& max_pos);

};
