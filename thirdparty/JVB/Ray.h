#pragma once

#include <glm.hpp>


namespace flex_bvh
{
	struct Ray
	{
		glm::vec3 origin;
		glm::vec3 direction;
		float tmin = 0.005f;
		float tmax = -1.0f;
	};

}