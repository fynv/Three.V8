#pragma once

#include <glm.hpp>
#include <memory>
#include <vector>

#include <bvh/bvh.hpp>
#include <bvh/vector.hpp>
#include <bvh/triangle.hpp>
#include <bvh/sweep_sah_builder.hpp>

#include <bvh/ray.hpp>
#include <bvh/single_ray_traverser.hpp>
#include <bvh/primitive_intersectors.hpp>

class Object3D;
class Primitive;
class SimpleModel;
class GLTFModel;

class BoundingVolumeHierarchy
{
public:
	class BLAS
	{
	public:
		struct Intersection {
			int triangle_index = -1;
			float t = -1.0f;
			float u = 0.0f;
			float v = 0.0f;
			float distance() const { return t; }
		};

		using ScalarType = float;
		using IntersectionType = Intersection;		

		BLAS(BLAS&&) = default;
		BLAS(const Primitive* primitive, const glm::mat4& model_matrix);
		~BLAS();

		bvh::Vector3<float> center() const;
		bvh::BoundingBox<float> bounding_box() const;
		std::optional<Intersection> intersect(const bvh::Ray<float>& ray) const;

	private:
		typedef bvh::ClosestPrimitiveIntersector<bvh::Bvh<float>, bvh::Triangle<float>> IntersectorType;
		typedef bvh::SingleRayTraverser<bvh::Bvh<float>> TraversorType;

		std::vector<bvh::Triangle<float>> m_triangles;
		bvh::BoundingBox<float> m_bounding_box;
		std::unique_ptr<bvh::Bvh<float>> m_bvh;
		std::unique_ptr<IntersectorType> m_intersector;
		std::unique_ptr<TraversorType> m_traverser;
	};

	struct Intersection {
		Object3D* object = nullptr;
		int primitive_index = -1;
		int triangle_index = -1;
		float t = -1.0f;
		float u = 0.0f;
		float v = 0.0f;
		float distance() const { return t; }
	};

	BoundingVolumeHierarchy(const std::vector<Object3D*>& objects);
	~BoundingVolumeHierarchy();

	std::optional<Intersection> intersect(const bvh::Ray<float>& ray) const;
	
private:
	struct PrimitiveInfo
	{
		Object3D* obj;		
		int primitive_idx;
	};

	typedef bvh::ClosestPrimitiveIntersector<bvh::Bvh<float>, BLAS> IntersectorType;
	typedef bvh::SingleRayTraverser<bvh::Bvh<float>> TraversorType;

	std::vector<PrimitiveInfo> m_primitive_map;
	std::vector<BLAS> m_primitives;
	std::unique_ptr<bvh::Bvh<float>> m_bvh;
	std::unique_ptr<IntersectorType> m_intersector;
	std::unique_ptr<TraversorType> m_traverser;

	void _add_primitive(const Primitive* primitive, const glm::mat4& model_matrix, Object3D* obj, int primitive_idx);
	void _add_model(SimpleModel* model);
	void _add_model(GLTFModel* model);

};
