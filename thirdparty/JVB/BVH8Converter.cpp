#include <vector>
#include "BVH8Converter.h"
#include "Util.h"

namespace flex_bvh
{
	struct BVH8Converter
	{
		BVH8& bvh8;
		const BVH2& bvh2;

		BVH8Converter(BVH8& bvh8, const BVH2& bvh2) : bvh8(bvh8), bvh2(bvh2) 
		{
			bvh8.indices.clear();
			bvh8.indices.reserve(bvh2.indices.size());
			bvh8.nodes.clear();
			bvh8.nodes.reserve(bvh2.nodes.size());
			decisions.resize(bvh2.nodes.size() * 7);
		}

		struct Decision 
		{
			enum struct Type : char 
			{
				LEAF,
				INTERNAL,
				DISTRIBUTE
			} type;

			char distribute_left;
			char distribute_right;

			float cost;
		};

		std::vector<Decision> decisions;


		int calculate_cost(int node_index, const std::vector<BVHNode2>& nodes);

		void get_children(int node_index, const std::vector<BVHNode2>& nodes, int children[8], int& child_count, int i);
		void order_children(int node_index, const std::vector<BVHNode2>& nodes, int children[8], int   child_count);

		int count_primitives(int node_index, const std::vector<BVHNode2>& nodes, const std::vector<int>& indices_sbvh);

		void collapse(const std::vector<BVHNode2>& nodes_bvh, const std::vector<int>& indices_bvh, int node_index_bvh8, int node_index_bvh2);

	};

	int BVH8Converter::calculate_cost(int node_index, const std::vector<BVHNode2>& nodes)
	{
		const BVHNode2& node = nodes[node_index];

		int num_primitives;

		if (node.is_leaf()) 
		{
			num_primitives = node.count;			

			// SAH cost
			float cost_leaf = node.aabb.surface_area() * float(num_primitives);

			for (int i = 0; i < 7; i++) 
			{
				decisions[node_index * 7 + i].type = Decision::Type::LEAF;
				decisions[node_index * 7 + i].cost = cost_leaf;
			}
		}
		else 
		{
			num_primitives =
				calculate_cost(node.left, nodes) +
				calculate_cost(node.left + 1, nodes);

			// Separate case: i=0 (i=1 in the paper)
			{
				float cost_leaf = num_primitives <= 3 ? float(num_primitives) * node.aabb.surface_area() : INFINITY;

				float cost_distribute = INFINITY;

				char distribute_left = -1;
				char distribute_right = -1;

				for (int k = 0; k < 7; k++) 
				{
					float c =
						decisions[(node.left) * 7 + k].cost +
						decisions[(node.left + 1) * 7 + 6 - k].cost;

					if (c < cost_distribute) 
					{
						cost_distribute = c;

						distribute_left = k;
						distribute_right = 6 - k;
					}
				}

				float cost_internal = cost_distribute + node.aabb.surface_area();

				if (cost_leaf < cost_internal) 
				{
					decisions[node_index * 7].type = Decision::Type::LEAF;
					decisions[node_index * 7].cost = cost_leaf;
				}
				else 
				{
					decisions[node_index * 7].type = Decision::Type::INTERNAL;
					decisions[node_index * 7].cost = cost_internal;
				}

				decisions[node_index * 7].distribute_left = distribute_left;
				decisions[node_index * 7].distribute_right = distribute_right;
			}

			// In the paper i=2..7
			for (int i = 1; i < 7; i++) 
			{
				float cost_distribute = decisions[node_index * 7 + i - 1].cost;

				char distribute_left = -1;
				char distribute_right = -1;

				for (int k = 0; k < i; k++) 
				{
					float c =
						decisions[(node.left) * 7 + k].cost +
						decisions[(node.left + 1) * 7 + i - k - 1].cost;

					if (c < cost_distribute) 
					{
						cost_distribute = c;

						distribute_left = k;
						distribute_right = i - k - 1;
					}
				}

				decisions[node_index * 7 + i].cost = cost_distribute;

				if (distribute_left != -1) 
				{
					decisions[node_index * 7 + i].type = Decision::Type::DISTRIBUTE;
					decisions[node_index * 7 + i].distribute_left = distribute_left;
					decisions[node_index * 7 + i].distribute_right = distribute_right;
				}
				else 
				{
					decisions[node_index * 7 + i] = decisions[node_index * 7 + i - 1];
				}
			}
		}

		return num_primitives;

	}


	void BVH8Converter::get_children(int node_index, const std::vector<BVHNode2>& nodes, int children[8], int& child_count, int i) {
		const BVHNode2& node = nodes[node_index];

		if (node.is_leaf()) {
			children[child_count++] = node_index;
			return;
		}

		char distribute_left = decisions[node_index * 7 + i].distribute_left;
		char distribute_right = decisions[node_index * 7 + i].distribute_right;	

		// Recurse on left child if it needs to distribute
		if (decisions[node.left * 7 + distribute_left].type == Decision::Type::DISTRIBUTE) {
			get_children(node.left, nodes, children, child_count, distribute_left);
		}
		else {
			children[child_count++] = node.left;
		}

		// Recurse on right child if it needs to distribute
		if (decisions[(node.left + 1) * 7 + distribute_right].type == Decision::Type::DISTRIBUTE) {
			get_children(node.left + 1, nodes, children, child_count, distribute_right);
		}
		else {
			children[child_count++] = node.left + 1;
		}
	}

	void BVH8Converter::order_children(int node_index, const std::vector<BVHNode2>& nodes, int children[8], int child_count) 
	{
		glm::vec3 p = nodes[node_index].aabb.get_center();

		float cost[8][8] = { };

		// Fill cost table
		for (int c = 0; c < child_count; c++) 
		{
			for (int s = 0; s < 8; s++) 
			{
				glm::vec3 direction(
					(s & 0b100) ? -1.0f : +1.0f,
					(s & 0b010) ? -1.0f : +1.0f,
					(s & 0b001) ? -1.0f : +1.0f
				);

				cost[c][s] = glm::dot(nodes[children[c]].aabb.get_center() - p, direction);
			}
		}

		int   assignment[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
		bool slot_filled[8] = { };

		// Greedy child ordering, the paper mentions this as an alternative
		// that works about as well as the auction algorithm in practice
		while (true) 
		{
			float min_cost = INFINITY;

			int min_slot = -1;
			int min_index = -1;

			// Find cheapest unfilled slot of any unassigned child
			for (int c = 0; c < child_count; c++) 
			{
				if (assignment[c] == -1) 
				{
					for (int s = 0; s < 8; s++)
					{
						if (!slot_filled[s] && cost[c][s] < min_cost) 
						{
							min_cost = cost[c][s];

							min_slot = s;
							min_index = c;
						}
					}
				}
			}

			if (min_slot == -1) break;

			slot_filled[min_slot] = true;
			assignment[min_index] = min_slot;
		}

		// Permute children array according to assignment
		int children_copy[8] = { };
		for (int i = 0; i < 8; i++) 
		{
			children_copy[i] = children[i];
			children[i] = -1;
		}
		for (int i = 0; i < child_count; i++) 
		{			
			children[assignment[i]] = children_copy[i];
		}
	}

	// Recursively count triangles in subtree of the given Node
	// Simultaneously fills the indices buffer of the BVH8
	int BVH8Converter::count_primitives(int node_index, const std::vector<BVHNode2>& nodes, const std::vector<int>& indices) 
	{
		const BVHNode2& node = nodes[node_index];

		if (node.is_leaf()) 
		{
			for (unsigned i = 0; i < node.count; i++)
			{
				bvh8.indices.push_back(indices[node.first + i]);
			}

			return node.count;
		}

		return
			count_primitives(node.left, nodes, indices) +
			count_primitives(node.left + 1, nodes, indices);
	}

	void BVH8Converter::collapse(const std::vector<BVHNode2>& nodes_bvh, const std::vector<int>& indices_bvh, int node_index_bvh8, int node_index_bvh2)
	{
		BVHNode8& node = bvh8.nodes[node_index_bvh8];
		const AABB& aabb = nodes_bvh[node_index_bvh2].aabb;

		node.p = aabb.pos_min;

		constexpr int Nq = 8;
		constexpr float denom = 1.0f / float((1 << Nq) - 1);

		glm::vec3 e(
			exp2f(ceilf(log2f((aabb.pos_max.x - aabb.pos_min.x) * denom))),
			exp2f(ceilf(log2f((aabb.pos_max.y - aabb.pos_min.y) * denom))),
			exp2f(ceilf(log2f((aabb.pos_max.z - aabb.pos_min.z) * denom)))
		);

		glm::vec3 one_over_e(1.0f / e.x, 1.0f / e.y, 1.0f / e.z);

		unsigned u_ex = bit_cast<unsigned>(e.x);
		unsigned u_ey = bit_cast<unsigned>(e.y);
		unsigned u_ez = bit_cast<unsigned>(e.z);

		// Store only 8 bit exponent
		node.e[0] = u_ex >> 23;
		node.e[1] = u_ey >> 23;
		node.e[2] = u_ez >> 23;

		int child_count = 0;
		int children[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
		get_children(node_index_bvh2, nodes_bvh, children, child_count, 0);

		order_children(node_index_bvh2, nodes_bvh, children, child_count);

		node.imask = 0;

		node.base_index_triangle = unsigned(bvh8.indices.size());
		node.base_index_child = unsigned(bvh8.nodes.size());

		int num_internal_nodes = 0;
		int num_triangles = 0;

		for (int i = 0; i < 8; i++) 
		{
			int child_index = children[i];
			if (child_index == -1) continue; // Empty slot

			const AABB& child_aabb = nodes_bvh[child_index].aabb;

			node.quantized_min_x[i] = byte(floorf((child_aabb.pos_min.x - node.p.x) * one_over_e.x));
			node.quantized_min_y[i] = byte(floorf((child_aabb.pos_min.y - node.p.y) * one_over_e.y));
			node.quantized_min_z[i] = byte(floorf((child_aabb.pos_min.z - node.p.z) * one_over_e.z));

			node.quantized_max_x[i] = byte(ceilf((child_aabb.pos_max.x - node.p.x) * one_over_e.x));
			node.quantized_max_y[i] = byte(ceilf((child_aabb.pos_max.y - node.p.y) * one_over_e.y));
			node.quantized_max_z[i] = byte(ceilf((child_aabb.pos_max.z - node.p.z) * one_over_e.z));

			switch (decisions[child_index * 7].type) {
				case Decision::Type::LEAF: {
					int triangle_count = count_primitives(child_index, nodes_bvh, indices_bvh);				

					// Three highest bits contain unary representation of triangle count
					for (int j = 0; j < triangle_count; j++) {
						node.meta[i] |= (1 << (j + 5));
					}

					node.meta[i] |= num_triangles;

					num_triangles += triangle_count;				
					break;
				}
				case Decision::Type::INTERNAL: {
					node.meta[i] = (i + 24) | 0b00100000;

					node.imask |= (1 << i);
					num_internal_nodes++;
					break;
				}			
			}
		}

		for (int i = 0; i < num_internal_nodes; i++) {
			bvh8.nodes.emplace_back();
		}
		node = bvh8.nodes[node_index_bvh8]; // NOTE: 'node' may have been invalidated by emplace_back on previous line

		// Recurse on Internal Nodes
		int offset = 0;
		for (int i = 0; i < 8; i++) {
			int child_index = children[i];
			if (child_index == -1) continue;

			if (node.imask & (1 << i)) {
				collapse(nodes_bvh, indices_bvh, node.base_index_child + offset++, child_index);
			}
		}
	}


	void ConvertBVH2ToBVH8(const BVH2& bvh2, BVH8& bvh8)
	{
		BVH8Converter converter(bvh8, bvh2);
		bvh8.nodes.emplace_back(); // Root
		
		converter.calculate_cost(0, bvh2.nodes);
		converter.collapse(bvh2.nodes, bvh2.indices, 0, 0);
	}

}