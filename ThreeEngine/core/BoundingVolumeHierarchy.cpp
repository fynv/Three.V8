#include <unordered_set>

#include "Object3D.h"
#include "BoundingVolumeHierarchy.h"
#include "models/SimpleModel.h"
#include "models/GLTFModel.h"

template <typename T>
inline void t_get_indices(T* indices, int face_id, unsigned& i0, unsigned& i1, unsigned& i2)
{
	i0 = indices[face_id * 3];
	i1 = indices[face_id * 3 + 1];
	i2 = indices[face_id * 3 + 2];
}

inline void get_indices(void* indices, int type_indices, int face_id, unsigned& i0, unsigned& i1, unsigned& i2)
{
	if (type_indices == 1)
	{
		t_get_indices((uint8_t*)indices, face_id, i0, i1, i2);
	}
	else if (type_indices == 2)
	{
		t_get_indices((uint16_t*)indices, face_id, i0, i1, i2);
	}
	else if (type_indices == 4)
	{
		t_get_indices((uint32_t*)indices, face_id, i0, i1, i2);
	}
}

void BoundingVolumeHierarchy::BLAS::_build_bvh()
{
	auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(m_triangles.data(), m_triangles.size());
	m_bounding_box = bvh::compute_bounding_boxes_union(bboxes.get(), m_triangles.size());

	m_bvh = std::unique_ptr<bvh::Bvh<float>>(new bvh::Bvh<float>);
	bvh::SweepSahBuilder<bvh::Bvh<float>> builder(*m_bvh);
	builder.build(m_bounding_box, bboxes.get(), centers.get(), m_triangles.size());

	m_intersector[0] = std::unique_ptr<IntersectorType>(new IntersectorType(*m_bvh, m_triangles.data()));
	m_intersector[0]->culling = 0;
	m_intersector[1] = std::unique_ptr<IntersectorType>(new IntersectorType(*m_bvh, m_triangles.data()));
	m_intersector[1]->culling = 1;
	m_intersector[2] = std::unique_ptr<IntersectorType>(new IntersectorType(*m_bvh, m_triangles.data()));
	m_intersector[2]->culling = 2;
	m_traverser = std::unique_ptr<TraversorType>(new TraversorType(*m_bvh));
	m_collision_traverser = std::unique_ptr<TraversorType2>(new TraversorType2(*m_bvh));

}

BoundingVolumeHierarchy::BLAS::BLAS(const Primitive* primitive, const glm::mat4& model_matrix)
{
	if (primitive->index_buf != nullptr)
	{
		for (int i = 0; i < primitive->num_face; i++)
		{
			unsigned i0, i1, i2;
			get_indices(primitive->cpu_indices->data(), primitive->type_indices, i, i0, i1, i2);

			glm::vec4 v0, v1, v2;
			v0 = primitive->cpu_pos->data()[i0];
			v1 = primitive->cpu_pos->data()[i1];
			v2 = primitive->cpu_pos->data()[i2];

			v0 = model_matrix * v0;
			v1 = model_matrix * v1;
			v2 = model_matrix * v2;


			m_triangles.emplace_back(PrimitiveType(
				bvh::Vector3<float>(v0.x, v0.y, v0.z),
				bvh::Vector3<float>(v1.x, v1.y, v1.z),
				bvh::Vector3<float>(v2.x, v2.y, v2.z)
			));

		}
	}
	else
	{
		for (int i = 0; i < primitive->num_pos / 3; i++)
		{
			glm::vec4 v0, v1, v2;
			v0 = primitive->cpu_pos->data()[i * 3];
			v1 = primitive->cpu_pos->data()[i * 3 + 1];
			v2 = primitive->cpu_pos->data()[i * 3 + 2];

			v0 = model_matrix * v0;
			v1 = model_matrix * v1;
			v2 = model_matrix * v2;

			m_triangles.emplace_back(PrimitiveType(
				bvh::Vector3<float>(v0.x, v0.y, v0.z),
				bvh::Vector3<float>(v1.x, v1.y, v1.z),
				bvh::Vector3<float>(v2.x, v2.y, v2.z)
			));
		}
	}
	_build_bvh();
}

BoundingVolumeHierarchy::BLAS::BLAS(FILE* fp)
{
	int count;
	fread(&count, 4, 1, fp);
	m_triangles.resize(count);
	fread(m_triangles.data(), sizeof(PrimitiveType), m_triangles.size(), fp);
	_build_bvh();
}

BoundingVolumeHierarchy::BLAS::~BLAS()
{

}

void BoundingVolumeHierarchy::BLAS::serialize(FILE* fp)
{
	int count = (int)m_triangles.size();
	fwrite(&count, 4, 1, fp);
	fwrite(m_triangles.data(), sizeof(PrimitiveType), m_triangles.size(), fp);
}

bvh::Vector3<float> BoundingVolumeHierarchy::BLAS::center() const
{
	return m_bounding_box.center();
}

bvh::BoundingBox<float> BoundingVolumeHierarchy::BLAS::bounding_box() const
{
	return m_bounding_box;
}

std::optional<BoundingVolumeHierarchy::BLAS::Intersection> BoundingVolumeHierarchy::BLAS::intersect(const bvh::Ray<float>& ray, int culling) const
{
	auto hit = m_traverser->traverse(ray, *m_intersector[culling]);
	if (hit.has_value())
	{
		auto intersection = hit->intersection;
		Intersection ret;
		ret.triangle_index = hit->primitive_index;
		ret.t = intersection.t;
		ret.u = intersection.u;
		ret.v = intersection.v;
		return std::make_optional<Intersection>(ret);
	}
	return std::nullopt;		
}

std::optional<BoundingVolumeHierarchy::BLAS::Intersection> BoundingVolumeHierarchy::BLAS::collide(const bvh::Sphere<float>& sphere) const
{
	auto hit = m_collision_traverser->traverse(sphere, *m_intersector[0]);
	if (hit.has_value())
	{
		auto intersection = hit->intersection;
		Intersection ret;
		ret.triangle_index = hit->primitive_index;
		ret.t = intersection.t;
		ret.u = intersection.u;
		ret.v = intersection.v;
		return std::make_optional<Intersection>(ret);
	}
	return std::nullopt;

}


union Signature
{
	struct {
		int id_model;
		int primitive_idx;
	} data;
	uint64_t key;
};


void BoundingVolumeHierarchy::_add_primitive(const Primitive* primitive, const glm::mat4& model_matrix, Object3D* obj, int primitive_idx)
{	
	m_primitive_map.push_back({ obj, primitive_idx });
	m_primitives.emplace_back(BLAS(primitive, model_matrix));

	Signature s = { obj->id, primitive_idx };
	m_primitive_index_map[s.key] = (int)(m_primitive_map.size()-1);
}

void BoundingVolumeHierarchy::_add_model(SimpleModel* model)
{
	const MeshStandardMaterial& material = model->material;
	if (material.alphaMode != AlphaMode::Opaque) return;
	Primitive* primitive = &model->geometry;
	_add_primitive(primitive, model->matrixWorld, model, 0);
}

void BoundingVolumeHierarchy::_add_model(GLTFModel* model)
{
	std::vector<const MeshStandardMaterial*> material_lst(model->m_materials.size());
	for (size_t i = 0; i < material_lst.size(); i++)
		material_lst[i] = model->m_materials[i].get();

	int count_primitive = 0;
	size_t num_mesh = model->m_meshs.size();
	for (size_t i = 0; i < num_mesh; i++)
	{
		Mesh& mesh = model->m_meshs[i];
		glm::mat4 matrix = model->matrixWorld;
		if (mesh.node_id >= 0 && mesh.skin_id < 0)
		{
			Node& node = model->m_nodes[mesh.node_id];
			matrix *= node.g_trans;
		}
		size_t num_primitives = mesh.primitives.size();
		for (size_t j = 0; j < num_primitives; j++, count_primitive++)
		{
			Primitive* primitive = &mesh.primitives[j];
			const MeshStandardMaterial& material = *material_lst[primitive->material_idx];
			if (material.alphaMode == AlphaMode::Opaque)
			{
				_add_primitive(primitive, matrix, model, count_primitive);
			}			
		}
	}
}

void BoundingVolumeHierarchy::_update_primitive(const Primitive* primitive, const glm::mat4& model_matrix, Object3D* obj, int primitive_idx)
{
	PrimitiveInfo info = { obj, primitive_idx };
	Signature s = { obj->id, primitive_idx };		

	auto iter = m_primitive_index_map.find(s.key);
	if (iter != m_primitive_index_map.end())
	{
		int idx = iter->second;
		m_primitives[idx] = BLAS(primitive, model_matrix);
	}
	else
	{
		m_primitive_map.push_back(info);
		m_primitives.emplace_back(BLAS(primitive, model_matrix));
		m_primitive_index_map[s.key] = (int)(m_primitive_map.size() - 1);
	}
}

void BoundingVolumeHierarchy::_update_model(SimpleModel* model)
{
	Primitive* primitive = &model->geometry;
	_update_primitive(primitive, model->matrixWorld, model, 0);
}

void BoundingVolumeHierarchy::_update_model(GLTFModel* model)
{
	int count_primitive = 0;
	size_t num_mesh = model->m_meshs.size();
	for (size_t i = 0; i < num_mesh; i++)
	{
		Mesh& mesh = model->m_meshs[i];
		glm::mat4 matrix = model->matrixWorld;
		if (mesh.node_id >= 0 && mesh.skin_id < 0)
		{
			Node& node = model->m_nodes[mesh.node_id];
			matrix *= node.g_trans;
		}
		size_t num_primitives = mesh.primitives.size();
		for (size_t j = 0; j < num_primitives; j++, count_primitive++)
		{
			Primitive* primitive = &mesh.primitives[j];
			_update_primitive(primitive, matrix, model, count_primitive);
		}
	}
}

void BoundingVolumeHierarchy::_remove_primitive(const Primitive* primitive, const glm::mat4& model_matrix, Object3D* obj, int primitive_idx)
{
	PrimitiveInfo info = { obj, primitive_idx };
	Signature s = { obj->id, primitive_idx };

	auto iter = m_primitive_index_map.find(s.key);
	if (iter != m_primitive_index_map.end())
	{
		int idx = iter->second;
		m_primitive_map.erase(m_primitive_map.begin() + idx);
		m_primitives.erase(m_primitives.begin() + idx);
		m_primitive_index_map.erase(iter);
		for (size_t i = (size_t)idx; i < m_primitives.size(); i++)
		{
			const PrimitiveInfo& info2 = m_primitive_map[i];
			Signature s2 = { info2.obj->id, info2.primitive_idx };
			m_primitive_index_map[s2.key] = i;
		}		
	}	
}

void BoundingVolumeHierarchy::_remove_model(SimpleModel* model)
{
	Primitive* primitive = &model->geometry;
	_remove_primitive(primitive, model->matrixWorld, model, 0);
}

void BoundingVolumeHierarchy::_remove_model(GLTFModel* model)
{
	int count_primitive = 0;
	size_t num_mesh = model->m_meshs.size();
	for (size_t i = 0; i < num_mesh; i++)
	{
		Mesh& mesh = model->m_meshs[i];
		glm::mat4 matrix = model->matrixWorld;
		if (mesh.node_id >= 0 && mesh.skin_id < 0)
		{
			Node& node = model->m_nodes[mesh.node_id];
			matrix *= node.g_trans;
		}
		size_t num_primitives = mesh.primitives.size();
		for (size_t j = 0; j < num_primitives; j++, count_primitive++)
		{
			Primitive* primitive = &mesh.primitives[j];
			_remove_primitive(primitive, matrix, model, count_primitive);
		}
	}
}

void BoundingVolumeHierarchy::_build_bvh()
{	
	auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(m_primitives.data(), m_primitives.size());
	bvh::BoundingBox<float> global_bbox = bvh::compute_bounding_boxes_union(bboxes.get(), m_primitives.size());

	m_bvh = std::unique_ptr<bvh::Bvh<float>>(new bvh::Bvh<float>);
	bvh::SweepSahBuilder<bvh::Bvh<float>> builder(*m_bvh);
	builder.build(global_bbox, bboxes.get(), centers.get(), m_primitives.size());

	m_intersector[0] = std::unique_ptr<IntersectorType>(new IntersectorType(*m_bvh, m_primitives.data()));
	m_intersector[0]->culling = 0;
	m_intersector[1] = std::unique_ptr<IntersectorType>(new IntersectorType(*m_bvh, m_primitives.data()));
	m_intersector[1]->culling = 1;
	m_intersector[2] = std::unique_ptr<IntersectorType>(new IntersectorType(*m_bvh, m_primitives.data()));
	m_intersector[2]->culling = 2;
	m_traverser = std::unique_ptr<TraversorType>(new TraversorType(*m_bvh));
	m_collision_traverser = std::unique_ptr<TraversorType2>(new TraversorType2(*m_bvh));
}

BoundingVolumeHierarchy::BoundingVolumeHierarchy(const std::vector<Object3D*>& objects) 
{	
	std::unordered_set<Object3D*> object_set;
	std::unordered_set<Object3D*>* p_objects = &object_set;

	for (size_t i = 0; i < objects.size(); i++)
	{
		objects[i]->updateWorldMatrix(true, false);
		objects[i]->traverse([p_objects](Object3D* obj) {
			obj->updateWorldMatrix(false, false);
			p_objects->insert(obj);
		});
	}

	for (auto iter = object_set.begin(); iter != object_set.end(); iter++)
	{
		do
		{
			{
				SimpleModel* model = dynamic_cast<SimpleModel*>(*iter);
				if (model)
				{
					_add_model(model);
					break;
				}
			}
			{
				GLTFModel* model = dynamic_cast<GLTFModel*>(*iter);
				if (model)
				{
					_add_model(model);
					break;
				}
			}

		} while (false);
	}	

	_build_bvh();
}

BoundingVolumeHierarchy::BoundingVolumeHierarchy(FILE* fp)
{
	static Object3D s_dummy;

	int count;
	fread(&count, 4, 1, fp);
	for (int i = 0; i < count; i++)
	{
		m_primitives.emplace_back(BLAS(fp));
		m_primitive_map.push_back({&s_dummy, i});

		Signature s = { s_dummy.id, i };
		m_primitive_index_map[s.key] = i;
	}
	_build_bvh();
}

BoundingVolumeHierarchy::~BoundingVolumeHierarchy()
{

}

void BoundingVolumeHierarchy::serialize(FILE* fp)
{
	int count = (int)m_primitives.size();
	fwrite(&count, 4, 1, fp);
	for (int i = 0; i < count; i++)
	{
		m_primitives[i].serialize(fp);
	}

}

void BoundingVolumeHierarchy::update(Object3D* obj)
{	
	std::unordered_set<Object3D*> object_set;
	std::unordered_set<Object3D*>* p_objects = &object_set;

	obj->updateWorldMatrix(true, false);
	obj->traverse([p_objects](Object3D* child) {
		child->updateWorldMatrix(false, false);
		p_objects->insert(child);
	});

	size_t map_size = m_primitive_map.size();

	for (auto iter = object_set.begin(); iter != object_set.end(); iter++)
	{
		do
		{
			{
				SimpleModel* model = dynamic_cast<SimpleModel*>(*iter);
				if (model)
				{
					_update_model(model);
					break;
				}
			}
			{
				GLTFModel* model = dynamic_cast<GLTFModel*>(*iter);
				if (model)
				{
					_update_model(model);
					break;
				}
			}

		} while (false);
	}

	_build_bvh();
}

void BoundingVolumeHierarchy::remove(Object3D* obj)
{
	std::unordered_set<Object3D*> object_set;
	std::unordered_set<Object3D*>* p_objects = &object_set;
	
	obj->traverse([p_objects](Object3D* child) {		
		p_objects->insert(child);
	});

	for (auto iter = object_set.begin(); iter != object_set.end(); iter++)
	{
		do
		{
			{
				SimpleModel* model = dynamic_cast<SimpleModel*>(*iter);
				if (model)
				{
					_remove_model(model);
					break;
				}
			}
			{
				GLTFModel* model = dynamic_cast<GLTFModel*>(*iter);
				if (model)
				{
					_remove_model(model);
					break;
				}
			}

		} while (false);
	}
	_build_bvh();
}

std::optional<BoundingVolumeHierarchy::Intersection> BoundingVolumeHierarchy::intersect(const bvh::Ray<float>& ray, int culling) const
{	
	auto hit = m_traverser->traverse(ray, *m_intersector[culling]);
	if (hit.has_value())
	{		
		auto intersection = hit->intersection;		
		const PrimitiveInfo& info = m_primitive_map[hit->primitive_index];				
		Intersection ret;
		ret.object = info.obj;
		ret.primitive_index = info.primitive_idx;
		ret.triangle_index = intersection.triangle_index;
		ret.t = intersection.t;
		ret.u = intersection.u;
		ret.v = intersection.v;
		return std::make_optional<Intersection>(ret);
	}	
	return std::nullopt;	
}

std::optional<BoundingVolumeHierarchy::Intersection> BoundingVolumeHierarchy::collide(const bvh::Sphere<float>& sphere) const
{
	auto hit = m_collision_traverser->traverse(sphere, *m_intersector[0]);
	if (hit.has_value())
	{
		auto intersection = hit->intersection;
		const PrimitiveInfo& info = m_primitive_map[hit->primitive_index];
		Intersection ret;
		ret.object = info.obj;
		ret.primitive_index = info.primitive_idx;
		ret.triangle_index = intersection.triangle_index;
		ret.t = intersection.t;
		ret.u = intersection.u;
		ret.v = intersection.v;
		return std::make_optional<Intersection>(ret);
	}
	return std::nullopt;

}

glm::vec3 BoundingVolumeHierarchy::BLAS::get_intersection_pos(const Intersection& intersection) const
{
	const PrimitiveType& tri = m_triangles[intersection.triangle_index];
	auto p = tri.p0 + tri.e1 * intersection.u + tri.e2 * intersection.v;
	return glm::vec3(p[0], p[1], p[2]);
}

glm::vec3 BoundingVolumeHierarchy::get_intersection_pos(const Intersection& intersection) const
{	
	Signature s = { intersection.object->id, intersection.primitive_index };
	int idx = m_primitive_index_map.at(s.key);

	const BLAS& blas = m_primitives[idx];
	BLAS::Intersection b_intersection;
	b_intersection.triangle_index = intersection.triangle_index;
	b_intersection.t = intersection.t;
	b_intersection.u = intersection.u;
	b_intersection.v = intersection.v;
	return blas.get_intersection_pos(b_intersection);

}


