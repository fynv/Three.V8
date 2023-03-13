#include "BVH.h"
#include "SAHBuilder.h"

using namespace flex_bvh;

void BVH2::create_from_triangles(const std::vector<Triangle>& triangles)
{
	SAHBuilder(*this, triangles.size()).build(triangles);

}

#define BVH_TRAVERSAL_STACK_SIZE 64

void BVH2::intersect(const Ray& ray, Intersection& hit, const std::vector<Triangle>& triangles)
{
	hit.t = ray.tmax < 0.0f ? INFINITY : ray.tmax;

	int stack[BVH_TRAVERSAL_STACK_SIZE];
	int stack_size = 1;

	// Push root on stack
	stack[0] = 0;

	glm::vec3 inv_direction = 1.0f / ray.direction;

	while (stack_size > 0)
	{
		const BVHNode2& node = nodes[stack[--stack_size]];

		bool res = node.aabb.intersect(ray, inv_direction, hit.t);
		if (!res) continue;

		if (node.is_leaf())
		{			
			for (int i = node.first; i < node.first + node.count; i++)
			{				
				int j = indices[i];
				float t, u, v;
				if (triangles[j].intersect(ray, hit.t, t, u, v))
				{
					hit.triangle_index = j;
					hit.t = t;
					hit.u = u;
					hit.v = v;
				}				
			}
		}	
		else
		{
			if (node.should_visit_left_first(ray)) {
				stack[stack_size++] = node.left + 1;
				stack[stack_size++] = node.left;
			}
			else {
				stack[stack_size++] = node.left;
				stack[stack_size++] = node.left + 1;
			}

		}
	}
}

