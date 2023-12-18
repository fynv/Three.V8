#pragma once

#include <cmath>

#include "bvh/vector.hpp"
#include "bvh/sphere.hpp"
#include "bvh/platform.hpp"
#include "bvh/utilities.hpp"


namespace bvh {

template <typename Bvh>
struct NodeCollider {
	using Scalar = typename Bvh::ScalarType;
	
	NodeCollider() {}
	~NodeCollider() {}


	std::pair<Scalar, Scalar> collide(const typename Bvh::Node& node, const Sphere<Scalar>& sphere) const
	{
		Scalar dis_far = 0.0f;		

		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				for (int k = 0; k < 2; k++)
				{
					float x = node.bounds[k];
					float y = node.bounds[2 + j];
					float z = node.bounds[4 + i];

					float dx = x - sphere.origin[0];
					float dy = y - sphere.origin[1];
					float dz = z - sphere.origin[2];

					float d = sqrtf(dx * dx + dy * dy + dz * dz);
					if (d > dis_far)
					{
						dis_far = d;
					}					
				}
			}
		}

		Scalar dx = 0.0f;
		Scalar dy = 0.0f;
		Scalar dz = 0.0f;
		if (sphere.origin[0] < node.bounds[0])
		{
			dx = node.bounds[0] - sphere.origin[0];
		}
		else if (sphere.origin[0] > node.bounds[1])
		{
			dx = sphere.origin[0] - node.bounds[1];
		}

		if (sphere.origin[1] < node.bounds[2])
		{
			dy = node.bounds[2] - sphere.origin[1];
		}
		else if (sphere.origin[1] > node.bounds[3])
		{
			dy = sphere.origin[1] - node.bounds[3];
		}

		if (sphere.origin[2] < node.bounds[4])
		{
			dz = node.bounds[4] - sphere.origin[2];
		}
		else if (sphere.origin[2] > node.bounds[5])
		{
			dz = sphere.origin[2] - node.bounds[5];
		}

		Scalar dis_near = sqrtf(dx*dx + dy*dy + dz*dz);

		return std::make_pair(dis_near, dis_far);
	}

};


}


