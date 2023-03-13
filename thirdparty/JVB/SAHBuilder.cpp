#include <algorithm>
#include "SAHBuilder.h"

#include "Sort.h"
#include "BVHPartitions.h"

using namespace flex_bvh;

template<typename Primitive>
static void build_bvh_recursive(SAHBuilder& builder, BVHNode2& node, const std::vector<Primitive>& primitives, int* indices[3], int first_index, int index_count)
{
	if (index_count == 1) {
		// Leaf Node, terminate recursion
		// We do not terminate based on the SAH termination criterion, so that the
		// BVHs that are cached to disk have a standard layout (1 triangle per leaf node)
		// If desired these trees can be collapsed based on the SAH cost using BVHCollapser::collapse
		node.first = first_index;
		node.count = index_count;	

		return;
	}

	ObjectSplit split = partition_sah(primitives, indices, first_index, index_count, new(builder.scratch.data()) float[index_count]);

	for (int i = first_index; i < split.index; i++) builder.indices_going_left[indices[split.dimension][i]] = true;
	for (int i = split.index; i < first_index + index_count; i++) builder.indices_going_left[indices[split.dimension][i]] = false;

	for (int dim = 0; dim < 3; dim++) 
	{
		if (dim == split.dimension) continue;

		int left = 0;
		int right = split.index - first_index;
		int* temp = new(builder.scratch.data()) int[index_count];

		for (int i = first_index; i < first_index + index_count; i++) 
		{
			int index = indices[dim][i];

			bool goes_left = builder.indices_going_left[index];
			if (goes_left) 
			{
				temp[left++] = index;
			}
			else 
			{
				temp[right++] = index;
			}
		}

		memcpy(indices[dim] + first_index, temp, index_count * sizeof(int));
	}

	node.left = builder.bvh.nodes.size();
	node.count = 0;
	node.axis = split.dimension;

	BVHNode2& node_left = builder.bvh.nodes.emplace_back();
	BVHNode2& node_right = builder.bvh.nodes.emplace_back();
	node_left.aabb = split.aabb_left;
	node_right.aabb = split.aabb_right;

	int num_left = split.index - first_index;
	int num_right = first_index + index_count - split.index;


	build_bvh_recursive(builder, node_left, primitives, indices, first_index, num_left);
	build_bvh_recursive(builder, node_right, primitives, indices, first_index + num_left, num_right);


	return;
}


template<typename Primitive>
static void build_bvh_impl(SAHBuilder& builder, const std::vector<Primitive>& primitives)
{
	builder.bvh.indices.clear();
	builder.bvh.nodes.clear();
	builder.bvh.nodes.emplace_back(); // Root
	builder.bvh.nodes.emplace_back(); // Dummy

	AABB root_aabb = AABB::create_empty();
	for (size_t i = 0; i < primitives.size(); i++) 
	{
		root_aabb.expand(primitives[i].get_aabb());
	}
	builder.bvh.nodes[0].aabb = root_aabb;

	{
		std::vector<int> radix_sort_tmp = std::vector<int>(primitives.size());		

		radix_sort(builder.indices_x.data(), builder.indices_x.data() + builder.indices_x.size(), radix_sort_tmp.data(), [&primitives](int index) { return RadixSortAdapter<float>()(primitives[index].get_center().x); });
		radix_sort(builder.indices_y.data(), builder.indices_y.data() + builder.indices_y.size(), radix_sort_tmp.data(), [&primitives](int index) { return RadixSortAdapter<float>()(primitives[index].get_center().y); });
		radix_sort(builder.indices_z.data(), builder.indices_z.data() + builder.indices_z.size(), radix_sort_tmp.data(), [&primitives](int index) { return RadixSortAdapter<float>()(primitives[index].get_center().z); });
	}

	int* indices[3] = { builder.indices_x.data(), builder.indices_y.data(), builder.indices_z.data() };

	build_bvh_recursive(builder, builder.bvh.nodes[0], primitives, indices, 0, primitives.size());

	builder.bvh.indices = builder.indices_x; // NOTE: copy!

	
}

void SAHBuilder::build(const std::vector<Triangle>& triangles) 
{
	return build_bvh_impl(*this, triangles);
}

