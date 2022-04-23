#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "renderers/GLUtils.h"
#include "lights/EnvironmentMap.h"

struct Lights
{
	int num_directional_lights = 0;
	uint64_t hash_directional_lights = 0;
	std::unique_ptr<GLDynBuffer> constant_directional_lights;
	std::vector<unsigned> directional_shadow_texs;

	const EnvironmentMap* environment_map = nullptr;
};

