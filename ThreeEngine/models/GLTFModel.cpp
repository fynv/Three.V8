#include <GL/glew.h>
#include <list>
#include <gtx/hash.hpp>

#include "GLTFModel.h"
#include "utils/Utils.h"
#include "renderers/bvh_routines/PrimitiveBatch.h"
#include "renderers/LightmapRenderTarget.h"
#include "renderers/GLRenderer.h"

GLTFModel::GLTFModel()
{
}

GLTFModel::~GLTFModel()
{

}

void GLTFModel::calculate_bounding_box()
{
	m_min_pos = { FLT_MAX, FLT_MAX, FLT_MAX };
	m_max_pos = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

	size_t num_meshes = m_meshs.size();
	for (size_t i = 0; i < num_meshes; i++)
	{
		Mesh& mesh = m_meshs[i];

		glm::vec3 mesh_min_pos = { FLT_MAX, FLT_MAX, FLT_MAX };
		glm::vec3 mesh_max_pos = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

		size_t num_prims = mesh.primitives.size();
		for (size_t j = 0; j < num_prims; j++)
		{
			Primitive& prim = mesh.primitives[j];
			mesh_min_pos = glm::min(mesh_min_pos, prim.min_pos);
			mesh_max_pos = glm::max(mesh_max_pos, prim.max_pos);
		}
		
		if (mesh.node_id >= 0 && mesh.skin_id < 0)
		{
			Node& node = m_nodes[mesh.node_id];
			glm::mat4 mesh_mat = node.g_trans;

			glm::vec4 model_pos[8];
			model_pos[0] = mesh_mat * glm::vec4(mesh_min_pos.x, mesh_min_pos.y, mesh_min_pos.z, 1.0f);
			model_pos[1] = mesh_mat * glm::vec4(mesh_max_pos.x, mesh_min_pos.y, mesh_min_pos.z, 1.0f);
			model_pos[2] = mesh_mat * glm::vec4(mesh_min_pos.x, mesh_max_pos.y, mesh_min_pos.z, 1.0f);
			model_pos[3] = mesh_mat * glm::vec4(mesh_max_pos.x, mesh_max_pos.y, mesh_min_pos.z, 1.0f);
			model_pos[4] = mesh_mat * glm::vec4(mesh_min_pos.x, mesh_min_pos.y, mesh_max_pos.z, 1.0f);
			model_pos[5] = mesh_mat * glm::vec4(mesh_max_pos.x, mesh_min_pos.y, mesh_max_pos.z, 1.0f);
			model_pos[6] = mesh_mat * glm::vec4(mesh_min_pos.x, mesh_max_pos.y, mesh_max_pos.z, 1.0f);
			model_pos[7] = mesh_mat * glm::vec4(mesh_max_pos.x, mesh_max_pos.y, mesh_max_pos.z, 1.0f);

			for (int k = 0; k < 8; k++)
			{
				glm::vec4 pos = model_pos[k];
				if (pos.x < m_min_pos.x) m_min_pos.x = pos.x;
				if (pos.x > m_max_pos.x) m_max_pos.x = pos.x;
				if (pos.y < m_min_pos.y) m_min_pos.y = pos.y;
				if (pos.y > m_max_pos.y) m_max_pos.y = pos.y;
				if (pos.z < m_min_pos.z) m_min_pos.z = pos.z;
				if (pos.z > m_max_pos.z) m_max_pos.z = pos.z;
			}
		}
		else
		{
			m_min_pos = glm::min(m_min_pos, mesh_min_pos);
			m_max_pos = glm::max(m_max_pos, mesh_max_pos);
		}

		
	}
}

struct ModelConst
{
	glm::mat4 ModelMat;
	glm::mat4 NormalMat;
};

void GLTFModel::updateMeshConstants()
{
	size_t num_mesh = m_meshs.size();
	for (size_t i = 0; i < num_mesh; i++)
	{
		Mesh& mesh = m_meshs[i];
		glm::mat4 matrix = matrixWorld;
		if (mesh.node_id >= 0 && mesh.skin_id < 0)
		{
			Node& node = m_nodes[mesh.node_id];
			matrix *= node.g_trans;
		}
		ModelConst c;
		c.ModelMat = matrix;
		c.NormalMat = glm::transpose(glm::inverse(matrix));
		mesh.model_constant->upload(&c);
	}

	if (batched_mesh != nullptr)
	{
		ModelConst c;
		c.ModelMat = matrixWorld;
		c.NormalMat = glm::transpose(glm::inverse(matrixWorld));
		batched_mesh->model_constant->upload(&c);
	}
}

void GLTFModel::updateNodes()
{
	std::list<int> node_queue;
	size_t num_roots = m_roots.size();
	for (size_t i = 0; i < num_roots; i++)
	{
		int idx_root = m_roots[i];		
		Node& node = m_nodes[idx_root];
		node.g_trans = glm::identity<glm::mat4>();
		node_queue.push_back(idx_root);
	}

	while (!node_queue.empty())
	{
		int id_node = node_queue.front();
		node_queue.pop_front();
		Node& node = m_nodes[id_node];

		glm::mat4 local = glm::identity<glm::mat4>();
		local = glm::translate(local, node.translation);
		local *= glm::toMat4(node.rotation);
		local = glm::scale(local, node.scale);
		node.g_trans *= local;

		for (size_t i = 0; i < node.children.size(); i++)
		{
			int id_child = node.children[i];
			Node& child = m_nodes[id_child];
			child.g_trans = node.g_trans;
			node_queue.push_back(id_child);
		}
	}

	// update skins 
	size_t num_skins = m_skins.size();
	for (size_t i = 0; i < num_skins; i++)
	{
		Skin& skin = m_skins[i];
		size_t num_joints = skin.joints.size();				
		std::vector<glm::mat4> rela_mats(num_joints);
		for (int j = 0; j < num_joints; j++)
		{
			Node& node = m_nodes[skin.joints[j]];
			rela_mats[j] = node.g_trans * skin.inverseBindMatrices[j];			
		}
		skin.buf_rela_mat->upload(rela_mats.data());
	}

	needUpdateSkinnedMeshes = num_skins>0;
}

template<typename T>
inline void t_copy_indices(int num_face, int pos_offset, int face_offset, const uint8_t* p_indices_in, uint8_t* p_indices_out)
{
	const T* p_in = (const T*)p_indices_in;
	int* p_out = (int*)p_indices_out;
	for (int i = 0; i < num_face * 3; i++)
	{
		p_out[face_offset * 3 + i] = (int)(p_in[i]) + pos_offset;
	}
}

inline void g_copy_indices(int num_face, int pos_offset, int face_offset, int type_indices, const uint8_t* p_indices_in, uint8_t* p_indices_out)
{
	if (type_indices == 1)
	{
		t_copy_indices<uint8_t>(num_face, pos_offset, face_offset, p_indices_in, p_indices_out);
	}
	if (type_indices == 2)
	{
		t_copy_indices<uint16_t>(num_face, pos_offset, face_offset, p_indices_in, p_indices_out);
	}
	else if (type_indices == 4)
	{
		t_copy_indices<uint32_t>(num_face, pos_offset, face_offset, p_indices_in, p_indices_out);
	}

}

void GLTFModel::batch_primitives()
{
	struct BatchInfo
	{
		int num_pos = 0;
		int num_face = 0;
		bool has_color = false;
		bool has_uv = false;
		bool has_uv1 = false;
		bool has_tangent = false;
		std::vector<glm::ivec2> indices;
	};

	std::unordered_map<int, BatchInfo> primitive_map;

	size_t num_meshes = m_meshs.size();
	batch_map.resize(num_meshes);
	for (size_t i = 0; i < num_meshes; i++)
	{
		Mesh& mesh = m_meshs[i];		
		size_t num_prims = mesh.primitives.size();
		batch_map[i].resize(num_prims);
		for (size_t j = 0; j < num_prims; j++)
		{
			Primitive& prim = mesh.primitives[j];
			BatchInfo& info = primitive_map[prim.material_idx];
			info.num_pos += prim.num_pos;
			info.num_face += prim.num_face;
			info.has_color = info.has_color || prim.color_buf != nullptr;
			info.has_uv = info.has_uv || prim.uv_buf != nullptr;
			info.has_uv1 = info.has_uv1 || prim.lightmap_uv_buf != nullptr;
			info.has_tangent = info.has_tangent || prim.geometry[0].tangent_buf != nullptr;
			info.indices.push_back({ i,j });
		}
	}	

	std::unique_ptr<PrimitiveBatch> batchers[8];
	GLDynBuffer subModel(sizeof(ModelConst), GL_UNIFORM_BUFFER);

	batched_mesh = std::unique_ptr<Mesh>(new Mesh);
	{
		ModelConst c;
		c.ModelMat = matrixWorld;
		c.NormalMat = glm::transpose(glm::inverse(matrixWorld));
		batched_mesh->model_constant->upload(&c);
	}


	batched_mesh->primitives.resize(primitive_map.size());
	int idx_prim = 0;

	auto iter = primitive_map.begin();
	while (iter != primitive_map.end())
	{
		BatchInfo info = iter->second;				
		
		Primitive& prim_batch = batched_mesh->primitives[idx_prim];
		prim_batch.material_idx = iter->first;
		prim_batch.type_indices = 4;
		prim_batch.num_pos = info.num_pos;
		prim_batch.num_face = info.num_face;		

		bool has_color = info.has_color;
		bool has_uv = info.has_uv;	
		bool has_uv1 = info.has_uv1;
		bool has_tangent = info.has_tangent;

		int batcher_idx = (has_color ? 1 : 0) + (has_uv ? 2 : 0) + (has_tangent ? 4 : 0);
		auto& batcher = batchers[batcher_idx];
		if (batcher == nullptr)
		{
			PrimitiveBatch::Options options;
			options.has_color = has_color;
			options.has_uv = has_uv;
			options.has_uv1 = has_uv1;
			options.has_tangent = has_tangent;
			batcher = std::unique_ptr<PrimitiveBatch>(new PrimitiveBatch(options));
		}

		prim_batch.geometry.resize(1);
		GeometrySet& geometry = prim_batch.geometry[0];

		geometry.pos_buf = Attribute(new GLBuffer(sizeof(glm::vec4)* prim_batch.num_pos));	
		geometry.normal_buf = Attribute(new GLBuffer(sizeof(glm::vec4) * prim_batch.num_pos));

		if (has_color)
		{
			prim_batch.color_buf = Attribute(new GLBuffer(sizeof(glm::vec4) * prim_batch.num_pos));
		}

		if (has_uv)
		{
			prim_batch.uv_buf = Attribute(new GLBuffer(sizeof(glm::vec2) * prim_batch.num_pos));
		}

		if (has_uv1)
		{
			prim_batch.lightmap_uv_buf = Attribute(new GLBuffer(sizeof(glm::vec2) * prim_batch.num_pos));
		}

		if (has_tangent)
		{
			geometry.tangent_buf = Attribute(new GLBuffer(sizeof(glm::vec4) * prim_batch.num_pos));
			geometry.bitangent_buf = Attribute(new GLBuffer(sizeof(glm::vec4) * prim_batch.num_pos));
		}		

		prim_batch.cpu_pos = std::unique_ptr<std::vector<glm::vec4>>(new std::vector<glm::vec4>(prim_batch.num_pos));
		prim_batch.cpu_indices = std::unique_ptr<std::vector<uint8_t>>(new std::vector<uint8_t>(prim_batch.num_face * 3 * 4));

		int pos_offset = 0;
		int face_offset = 0;

		const std::vector<glm::ivec2>& indices = info.indices;
		for (size_t i = 0; i < indices.size(); i++)
		{
			int idx_mesh = indices[i].x;
			int idx_prim = indices[i].y;
			Mesh& mesh = m_meshs[idx_mesh];
			Primitive& prim = mesh.primitives[idx_prim];

			std::vector<int>& batch_map_mesh = batch_map[idx_mesh];
			batch_map_mesh[idx_prim] = face_offset * 3 * sizeof(int);

			prim_batch.min_pos = glm::min(prim_batch.min_pos, prim.min_pos);
			prim_batch.max_pos = glm::max(prim_batch.max_pos, prim.max_pos);

			int num_pos = prim.num_pos;
			for (int j = 0; j < num_pos; j++)
			{
				glm::vec4 pos = (*prim.cpu_pos)[j];
				if (mesh.node_id >= 0 && mesh.skin_id < 0)
				{
					Node& node = m_nodes[mesh.node_id];
					pos = node.g_trans * pos;
				}
				(*prim_batch.cpu_pos)[pos_offset + j] = pos;
			}						

			int num_face = prim.num_face;
			if (prim.index_buf != nullptr)
			{
				g_copy_indices(num_face, pos_offset, face_offset, prim.type_indices, prim.cpu_indices->data(), prim_batch.cpu_indices->data());
			}
			else
			{
				int* p_out = (int*)prim_batch.cpu_indices->data();
				for (int i = 0; i < num_face * 3; i++)
				{
					p_out[face_offset * 3 + i] = i  + pos_offset;
				}
			}

			{
				glm::mat4 matrix = glm::identity<glm::mat4>();
				if (mesh.node_id >= 0 && mesh.skin_id < 0)
				{
					Node& node = m_nodes[mesh.node_id];
					matrix = node.g_trans;
				}

				ModelConst c;
				c.ModelMat = matrix;
				c.NormalMat = glm::transpose(glm::inverse(matrix));
				subModel.upload(&c);
			}

			PrimitiveBatch::Params params;
			params.offset = pos_offset;
			params.constant_model = &subModel;
			params.primitive_in = &prim;
			params.primitive_out = &prim_batch;
			batcher->update(params);

			pos_offset += num_pos;
			face_offset += num_face;
		}		

		prim_batch.index_buf = Index(new IndexTextureBuffer(prim_batch.cpu_indices->size(), 4));
		prim_batch.index_buf->upload(prim_batch.cpu_indices->data());		

		iter++;
		idx_prim++;
	}

}

void GLTFModel::init_lightmap(GLRenderer* renderer, int width, int height, int texels_per_unit)
{
	lightmap = std::unique_ptr<Lightmap>(new Lightmap(width, height, texels_per_unit));

	lightmap_target = std::unique_ptr<LightmapRenderTarget>(new LightmapRenderTarget);
	lightmap_target->update_framebuffer(width, height);
	renderer->rasterize_atlas(this);

	{
		glm::vec4 zero = { 0.0f, 0.0f, 0.0f, 0.0f };
		glClearTexImage(lightmap->lightmap->tex_id, 0, GL_RGBA, GL_FLOAT, &zero);
	}

	std::vector<float> alpha_mask(lightmap->width * lightmap->height);

	glBindTexture(GL_TEXTURE_2D, lightmap_target->m_tex_position->tex_id);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_ALPHA, GL_FLOAT, alpha_mask.data());
	glBindTexture(GL_TEXTURE_2D, 0);

	std::vector<glm::u16vec2> lst_valid;
	for (int i = 0; i < lightmap->width * lightmap->height; i++)
	{		
		float f = alpha_mask[i];
		if (f > 0.5f)
		{
			uint16_t x = (uint16_t)(i % lightmap->width);
			uint16_t y = (uint16_t)(i / lightmap->width);
			lst_valid.push_back({ x,y });	
		}	
	}

	lightmap_target->count_valid = (int)lst_valid.size();		
	lightmap_target->valid_list = std::unique_ptr<TextureBuffer>(new TextureBuffer(sizeof(glm::u16vec2) * lightmap_target->count_valid, GL_RG16UI));
	lightmap_target->valid_list->upload(lst_valid.data());
}

#include "utils/HDRImage.h"
#include "loaders/HDRImageLoader.h"

void GLTFModel::load_lightmap(const char* fn)
{
	HDRImage img;
	HDRImageLoader::LoadFile(&img, fn);
	lightmap = std::unique_ptr<Lightmap>(new Lightmap(img));
}

void GLTFModel::setAnimationFrame(const AnimationFrame& frame)
{
	size_t num_morphs = frame.morphs.size();
	for (size_t i = 0; i < num_morphs; i++)
	{
		const MorphFrame& morph = frame.morphs[i];
		auto iter = m_mesh_dict.find(morph.name);
		if (iter != m_mesh_dict.end())
		{
			Mesh& mesh = m_meshs[iter->second];
			mesh.weights = morph.weights;
			mesh.needUpdateMorphTargets = true;
		}
	}

	size_t num_translations = frame.translations.size();
	for (size_t i = 0; i < num_translations; i++)
	{
		const TranslationFrame& trans = frame.translations[i];
		auto iter = m_node_dict.find(trans.name);
		if (iter != m_node_dict.end())
		{
			Node& node = m_nodes[iter->second];
			node.translation = trans.translation;			
		}
	}

	size_t num_rotations = frame.rotations.size();
	for (size_t i = 0; i < num_rotations; i++)
	{
		const RotationFrame& rot = frame.rotations[i];
		auto iter = m_node_dict.find(rot.name);
		if (iter != m_node_dict.end())
		{
			Node& node = m_nodes[iter->second];
			node.rotation = rot.rotation;
		}
	}

	size_t num_scales = frame.scales.size();
	for (size_t i = 0; i < num_scales; i++)
	{
		const ScaleFrame& scale = frame.scales[i];
		auto iter = m_node_dict.find(scale.name);
		if (iter != m_node_dict.end())
		{
			Node& node = m_nodes[iter->second];
			node.scale = scale.scale;
		}
	}

	updateNodes();
}

void GLTFModel::buildAnimDict()
{
	m_animation_dict.clear();
	for (size_t i = 0; i < m_animations.size(); i++)
	{
		m_animation_dict[m_animations[i].name] = i;
	}
}

void GLTFModel::addAnimation(const AnimationClip& anim)
{
	m_animations.push_back(anim);
	m_animation_dict[anim.name] = (int)(m_animations.size() - 1);
}

void GLTFModel::playAnimation(const char* name)
{
	auto iter = m_animation_dict.find(name);
	if (iter != m_animation_dict.end())
	{
		int id_anim = iter->second;
		for (size_t i = 0; i < m_current_playing.size(); i++)
		{
			PlayBack& playback = m_current_playing[i];
			if (playback.id_anim == id_anim)
			{
				playback.time_start = time_sec();
				return;
			}
		}
		m_current_playing.push_back({ id_anim, time_sec() });
	}
}

void GLTFModel::stopAnimation(const char* name)
{
	auto iter = m_animation_dict.find(name);
	if (iter != m_animation_dict.end())
	{
		int id_anim = iter->second;
		for (size_t i = 0; i < m_current_playing.size(); i++)
		{
			PlayBack& playback = m_current_playing[i];
			if (playback.id_anim == id_anim)
			{
				m_current_playing.erase(m_current_playing.begin() + i);
				return;
			}
		}
	}
}

void GLTFModel::updateAnimation()
{
	double t = time_sec();
	for (size_t i = 0; i < m_current_playing.size(); i++)
	{
		PlayBack& playback = m_current_playing[i];
		AnimationClip& anim = m_animations[playback.id_anim];
		double duration = anim.duration;
		double x = 0.0;
		if (duration > 0.0)
		{
			while (t - playback.time_start >= duration)
			{
				playback.time_start += duration;
			}
			x = t - playback.time_start;
		}
		AnimationFrame frame;
		anim.get_frame(x, frame);
		this->setAnimationFrame(frame);
	}
}

void GLTFModel::set_toon_shading(int mode, float wire_width, const glm::vec3& wire_color)
{
	for (size_t i = 0; i < m_materials.size(); i++)
	{
		m_materials[i]->tone_shading = mode;
		m_materials[i]->wire_width = wire_width;
		m_materials[i]->wire_color = wire_color;
	}
}
