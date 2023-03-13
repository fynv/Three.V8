#pragma once

#include "BitArray.h"
#include "BVH.h"

namespace flex_bvh
{
	struct Triangle;
	struct SAHBuilder
	{
		BVH2& bvh;

		std::vector<int> indices_x;
		std::vector<int> indices_y;
		std::vector<int> indices_z;

		std::vector<char> scratch;
		BitArray indices_going_left;

		SAHBuilder(BVH2& bvh, size_t primitive_count) :
			bvh(bvh),
			indices_x(primitive_count),
			indices_y(primitive_count),
			indices_z(primitive_count),
			scratch(primitive_count* std::max(sizeof(float), sizeof(int))),
			indices_going_left(primitive_count)
		{
			for (int i = 0; i < primitive_count; i++) 
			{
				indices_x[i] = i;
				indices_y[i] = i;
				indices_z[i] = i;
			}

			bvh.nodes.reserve(2 * primitive_count);
		}

		void build(const std::vector<Triangle>& triangles);	

	};
}

