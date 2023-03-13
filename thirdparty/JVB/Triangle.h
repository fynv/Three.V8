#pragma once

#include <glm.hpp>
#include "Constructors.h"
#include "AABB.h"

namespace flex_bvh
{
	struct Triangle
	{
		glm::vec3 position_0;
		glm::vec3 position_1;
		glm::vec3 position_2;

		Triangle() = default;

		Triangle(
			glm::vec3 position_0,
			glm::vec3 position_1,
			glm::vec3 position_2
		) :
			position_0(position_0),
			position_1(position_1),
			position_2(position_2)
		{


		}

		DEFAULT_COPYABLE(Triangle);
		DEFAULT_MOVEABLE(Triangle);

		~Triangle() = default;

		glm::vec3 get_center() const 
		{
			return (position_0 + position_1 + position_2) / 3.0f;
		}

		AABB get_aabb() const 
		{
			glm::vec3 vertices[3] = { position_0, position_1, position_2 };
			return AABB::from_points(vertices, 3);
		}

		bool intersect(const Ray& ray, float max_t, float& t, float& u, float& v) const;
	};

}