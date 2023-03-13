#include <gtc/matrix_transform.hpp>

#include "AABB.h"

using namespace flex_bvh;

AABB AABB::create_empty() 
{
	AABB aabb;
	aabb.pos_min = glm::vec3(+INFINITY);
	aabb.pos_max = glm::vec3(-INFINITY);
	return aabb;
}


AABB AABB::from_points(const glm::vec3* points, int point_count) 
{
	AABB aabb = create_empty();

	for (int i = 0; i < point_count; i++) 
	{
		aabb.expand(points[i]);
	}

	aabb.fix_if_needed();

	return aabb;
}

AABB AABB::unify(const AABB& b1, const AABB& b2) 
{
	AABB aabb;
	aabb.pos_min = glm::min(b1.pos_min, b2.pos_min);
	aabb.pos_max = glm::max(b1.pos_max, b2.pos_max);
	return aabb;
}


AABB AABB::overlap(const AABB& b1, const AABB& b2) 
{
	AABB aabb;
	aabb.pos_min = glm::max(b1.pos_min, b2.pos_min);
	aabb.pos_max = glm::min(b1.pos_max, b2.pos_max);

	if (!aabb.is_valid()) aabb = create_empty();

	return aabb;
}

AABB AABB::transform(const AABB& aabb, const glm::mat4& transformation) 
{
	glm::vec3 center = 0.5f * (aabb.pos_min + aabb.pos_max);
	glm::vec3 extent = 0.5f * (aabb.pos_max - aabb.pos_min);

	glm::mat4 abs_trans = glm::mat4(glm::abs(transformation[0]), glm::abs(transformation[1]), glm::abs(transformation[2]), glm::abs(transformation[3]));

	glm::vec3 new_center = glm::vec3(transformation * glm::vec4(center, 1.0f));
	glm::vec3 new_extent = glm::vec3(abs_trans * glm::vec4(extent, 0.0f));

	AABB result;
	result.pos_min = new_center - new_extent;
	result.pos_max = new_center + new_extent;

	return result;
}

bool AABB::intersect(const Ray& ray, const glm::vec3& inv_direction, float max_distance) const
{
	glm::vec3 aabb_min(pos_min);
	glm::vec3 aabb_max(pos_max);

	glm::vec3 t0 = (aabb_min - ray.origin) * inv_direction;
	glm::vec3 t1 = (aabb_max - ray.origin) * inv_direction;

	glm::vec3 t_min = glm::min(t0, t1);
	glm::vec3 t_max = glm::max(t0, t1);

	float t_near = glm::max(glm::max(ray.tmin, t_min.x), glm::max(t_min.y, t_min.z));
	float t_far = glm::min(glm::min(max_distance, t_max.x), glm::min(t_max.y, t_max.z));

	return t_near < t_far;
}



