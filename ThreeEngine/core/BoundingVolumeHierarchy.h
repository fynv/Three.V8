#pragma once

#include <glm.hpp>
#include <cstdint>
#include <memory>
#include <vector>
#include <unordered_map>

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
		std::optional<Intersection> intersect(const bvh::Ray<float>& ray, int culling) const;

		BLAS& operator =(BLAS&&) = default;

	private:
		typedef bvh::Triangle<float, true, true> PrimitiveType;
		typedef bvh::ClosestPrimitiveIntersector<bvh::Bvh<float>, PrimitiveType> IntersectorType;
		typedef bvh::SingleRayTraverser<bvh::Bvh<float>> TraversorType;

		std::vector<PrimitiveType> m_triangles;
		bvh::BoundingBox<float> m_bounding_box;
		std::unique_ptr<bvh::Bvh<float>> m_bvh;
		std::unique_ptr<IntersectorType> m_intersector[3];
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

	void update(Object3D* obj);
	void remove(Object3D* obj);

	std::optional<Intersection> intersect(const bvh::Ray<float>& ray, int culling = 0) const;
	
private:
	bool m_cull_front = false;

	struct PrimitiveInfo
	{
		Object3D* obj;		
		int primitive_idx;
	};

	typedef bvh::ClosestPrimitiveIntersector<bvh::Bvh<float>, BLAS> IntersectorType;
	typedef bvh::SingleRayTraverser<bvh::Bvh<float>> TraversorType;

	std::vector<PrimitiveInfo> m_primitive_map;
	std::vector<BLAS> m_primitives;

	std::unordered_map<uint64_t, int> m_primitive_index_map;	

	std::unique_ptr<bvh::Bvh<float>> m_bvh;
	std::unique_ptr<IntersectorType> m_intersector[3];
	std::unique_ptr<TraversorType> m_traverser;

	void _add_primitive(const Primitive* primitive, const glm::mat4& model_matrix, Object3D* obj, int primitive_idx);
	void _add_model(SimpleModel* model);
	void _add_model(GLTFModel* model);

	void _update_primitive(const Primitive* primitive, const glm::mat4& model_matrix, Object3D* obj, int primitive_idx);
	void _update_model(SimpleModel* model);
	void _update_model(GLTFModel* model);

	void _remove_primitive(const Primitive* primitive, const glm::mat4& model_matrix, Object3D* obj, int primitive_idx);
	void _remove_model(SimpleModel* model);
	void _remove_model(GLTFModel* model);

	void _build_bvh();

};


