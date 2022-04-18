#pragma once

#include <memory>
#include <cstdint>
#include "renderers/GLUtils.h"

struct Lights
{
	int num_directional_lights = 0;
	uint64_t hash_directional_lights = 0;
	std::unique_ptr<GLDynBuffer> constant_directional_lights;
};

