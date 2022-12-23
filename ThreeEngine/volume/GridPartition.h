#pragma once

#include <glm.hpp>
#include "renderers/GLUtils.h"

class GridPartition
{
public:
	GLTexture3D minmax_texture;
	int bytes_per_pixel = 1;
	glm::ivec3 bsize = glm::ivec3(1, 1, 1);
	glm::ivec3 bnum = glm::ivec3(0, 0, 0);
};
