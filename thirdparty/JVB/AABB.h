#pragma once

#include <cmath>
#include <glm.hpp>
#include "Ray.h"


namespace flex_bvh
{
	struct AABB
	{
		glm::vec3 pos_min;
		glm::vec3 pos_max;

		static AABB create_empty();
		static AABB from_points(const glm::vec3* points, int point_count);

		inline bool is_valid() const 
		{
			return pos_max.x > pos_min.x && pos_max.y > pos_min.y && pos_max.z > pos_min.z;
		}

		inline bool is_empty() const 
		{
			return
				pos_min.x == INFINITY && pos_min.y == INFINITY && pos_min.z == INFINITY &&
				pos_max.x == -INFINITY && pos_max.y == -INFINITY && pos_max.z == -INFINITY;
		}

		inline void fix_if_needed(float epsilon = 0.001f) 
		{
			if (is_empty()) return;

			for (int dimension = 0; dimension < 3; dimension++) 
			{
				float eps = epsilon;
				while (pos_max[dimension] - pos_min[dimension] < eps) 
				{
					pos_min[dimension] -= eps;
					pos_max[dimension] += eps;
					eps *= 2.0f;
				}
			}
		}

		inline float surface_area() const
		{			
			glm::vec3 diff = pos_max - pos_min;
			return 2.0f * (diff.x * diff.y + diff.y * diff.z + diff.z * diff.x);
		}

		inline void expand(const glm::vec3& point)
		{
			pos_min = glm::min(pos_min, point);
			pos_max = glm::max(pos_max, point);
		}

		inline void expand(const AABB& aabb) {
			pos_min = glm::min(pos_min, aabb.pos_min);
			pos_max = glm::max(pos_max, aabb.pos_max);
		}

		inline glm::vec3 get_center() const
		{
			return (pos_min + pos_max) * 0.5f;
		}

		static AABB unify(const AABB& b1, const AABB& b2);
		static AABB overlap(const AABB& b1, const AABB& b2);

		static AABB transform(const AABB& aabb, const glm::mat4& transformation);

		bool intersect(const Ray& ray, const glm::vec3& inv_direction, float max_distance) const;
	};
}