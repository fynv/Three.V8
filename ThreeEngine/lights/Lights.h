#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "renderers/GLUtils.h"
#include "lights/EnvironmentMap.h"
#include "lights/AmbientLight.h"
#include "lights/HemisphereLight.h"

struct Lights
{
	int num_directional_lights = 0;
	uint64_t hash_directional_lights = 0;
	std::unique_ptr<GLDynBuffer> constant_directional_lights;

	int num_directional_shadows = 0;
	uint64_t hash_directional_shadows = 0;
	std::unique_ptr<GLDynBuffer> constant_directional_shadows;

	std::vector<unsigned> directional_shadow_texs;

	const EnvironmentMap* environment_map = nullptr;
	const AmbientLight* ambient_light = nullptr;
	const HemisphereLight* hemisphere_light = nullptr;
};

