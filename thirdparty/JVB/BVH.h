#pragma once

#include <vector>
#include <glm.hpp>
#include "AABB.h"
#include "Triangle.h"
#include "Ray.h"

#define BVH_AXIS_X_BITS (0b01 << 30)
#define BVH_AXIS_Y_BITS (0b10 << 30)
#define BVH_AXIS_Z_BITS (0b11 << 30)
#define BVH_AXIS_MASK   (0b11 << 30)

namespace flex_bvh
{
	struct Intersection
	{
		int triangle_index = -1;
		float t = -1.0f;
		float u = 0.0f;
		float v = 0.0f;
	};

	typedef unsigned char byte;

	struct BVHNode2
	{
		AABB aabb;
		union {
			int left;
			int first;
		};
		unsigned count : 30;
		unsigned axis : 2;

		inline int get_count() const {
			return count & ~BVH_AXIS_MASK;
		}

		inline int get_axis() const {
			return count & BVH_AXIS_MASK;
		}

		inline bool is_leaf() const 
		{
			return count > 0;
		}
		
		inline bool should_visit_left_first(const Ray& ray) const
		{
			switch (get_axis()) 
			{
				case BVH_AXIS_X_BITS: return ray.direction.x > 0.0f;
				case BVH_AXIS_Y_BITS: return ray.direction.y > 0.0f;
				case BVH_AXIS_Z_BITS: return ray.direction.z > 0.0f;
			}
		}

	};


	struct BVHNode8 
	{
		glm::vec3 p;
		byte e[3];
		byte imask;

		unsigned base_index_child;
		unsigned base_index_triangle;

		byte meta[8] = { };

		byte quantized_min_x[8] = { }, quantized_max_x[8] = { };
		byte quantized_min_y[8] = { }, quantized_max_y[8] = { };
		byte quantized_min_z[8] = { }, quantized_max_z[8] = { };

		inline bool is_leaf(int child_index) 
		{
			return (meta[child_index] & 0b00011111) < 24;
		}
	};

	struct BVH2;

	struct BVH
	{
		std::vector<int> indices;

		virtual ~BVH() = default;

		virtual size_t node_count() const = 0;
	};


	struct BVH2 final : BVH 
	{
		BVH2() {}
		std::vector<BVHNode2> nodes;

		DEFAULT_COPYABLE(BVH2);
		DEFAULT_MOVEABLE(BVH2);

		size_t node_count() const override { return nodes.size(); }

		void create_from_triangles(const std::vector<Triangle>& triangles);
		void intersect(const Ray& ray, Intersection& hit, const std::vector<Triangle>& triangles);
	
	};


	struct BVH8 final : BVH 
	{
		BVH8() {}
		std::vector<BVHNode8> nodes;

		size_t node_count() const override { return nodes.size(); }
	};



}