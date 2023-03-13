#pragma once

#include <glm.hpp>
#include <vector>
#include "AABB.h"
#include "Util.h"

namespace flex_bvh
{
	struct Triangle;

	struct PrimitiveRef 
	{
		int  index;
		AABB aabb;
	};


	struct ObjectSplit 
	{
		int   index;
		float cost;
		int   dimension;

		AABB aabb_left;
		AABB aabb_right;
	};

	struct SpatialSplit 
	{
		int   index;
		float cost;
		int   dimension;

		AABB aabb_left;
		AABB aabb_right;

		float plane_distance;

		int num_left;
		int num_right;
	};

	inline constexpr int SBVH_BIN_COUNT = 256;

	ObjectSplit partition_sah(const std::vector<Triangle>& triangles, int* indices[3], int first_index, int index_count, float* sah);	

	ObjectSplit partition_sah(std::vector<PrimitiveRef> primitive_refs[3], int first_index, int index_count, float* sah);

	void triangle_intersect_plane(glm::vec3 vertices[3], int dimension, float plane, glm::vec3 intersections[], int* intersection_count);

	SpatialSplit partition_spatial(const std::vector<Triangle>& triangles, const std::vector<PrimitiveRef> indices[3], int first_index, int index_count, float* sah, AABB bounds);


}
