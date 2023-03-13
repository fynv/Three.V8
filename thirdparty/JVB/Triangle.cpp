#include "Triangle.h"

using namespace flex_bvh;

bool Triangle::intersect(const Ray& ray, float max_t, float& t, float& u, float& v) const
{
	glm::vec3 edge_1 = position_1 - position_0;
	glm::vec3 edge_2 = position_2 - position_0;

	glm::vec3 h = glm::cross(ray.direction, edge_2);
	float a = glm::dot(edge_1, h);

	float f = 1.0f / a;
	glm::vec3 s = ray.origin - position_0;
	u = f * glm::dot(s, h);

	if (u < 0.0f || u > 1.0f) return false;

	glm::vec3 q = glm::cross(s, edge_1);
	v = f * glm::dot(ray.direction, q);
	
	if (v < 0.0f || (u + v) > 1.0f) return false;
	t = f * glm::dot(edge_2, q);
	
	if (t < ray.tmin) return false;	
	return t <= max_t;
}