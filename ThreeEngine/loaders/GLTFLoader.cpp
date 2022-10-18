#include <GL/glew.h>
#include <gtx/matrix_decompose.hpp>

#include "GLTFLoader.h"

#include "models/ModelComponents.h"
#include "models/GLTFModel.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_ENABLE_DRACO
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

template<typename T>
inline void t_calc_normal(int num_face, int num_pos, const T* p_indices, const glm::vec4* p_pos, glm::vec4* p_norm)
{
	std::vector<float> counts(num_pos, 0.0f);
	for (int j = 0; j < num_face; j++)
	{
		glm::uvec3 ind;
		if (p_indices != nullptr)
		{
			ind.x = (uint32_t)p_indices[j * 3];
			ind.y = (uint32_t)p_indices[j * 3 + 1];
			ind.z = (uint32_t)p_indices[j * 3 + 2];
		}
		else
		{
			ind.x = j * 3;
			ind.y = j * 3 + 1;
			ind.z = j * 3 + 2;
		}

		glm::vec3 v0 = p_pos[ind.x];
		glm::vec3 v1 = p_pos[ind.y];
		glm::vec3 v2 = p_pos[ind.z];
		glm::vec4 face_normals = glm::vec4(glm::cross(v1 - v0, v2 - v0), 0.0f);

		p_norm[ind.x] += face_normals;
		p_norm[ind.y] += face_normals;
		p_norm[ind.z] += face_normals;
		counts[ind.x] += 1.0f;
		counts[ind.y] += 1.0f;
		counts[ind.z] += 1.0f;
	}

	for (int j = 0; j < num_pos; j++)
		p_norm[j] = p_norm[j]/ counts[j];
}

inline void g_calc_normal(int num_face, int num_pos, int type_indices, const void* p_indices, const glm::vec4* p_pos, glm::vec4* p_norm)
{
	if (type_indices == 1)
	{
		t_calc_normal<uint8_t>(num_face, num_pos, (uint8_t*)p_indices, p_pos, p_norm);
	}
	if (type_indices == 2)
	{
		t_calc_normal<uint16_t>(num_face, num_pos, (uint16_t*)p_indices, p_pos, p_norm);
	}
	else if (type_indices == 4)
	{
		t_calc_normal<uint32_t>(num_face, num_pos, (uint32_t*)p_indices, p_pos, p_norm);
	}
}

template<typename T>
inline void t_calc_tangent(int num_face, int num_pos, const T* p_indices, const glm::vec4* p_pos, const glm::vec2* p_uv, glm::vec4* p_tangent, glm::vec4* p_bitangent)
{
	std::vector<float> counts(num_pos, 0.0f);
	for (int j = 0; j < num_face; j++)
	{
		glm::uvec3 ind;
		if (p_indices != nullptr)
		{
			ind.x = (uint32_t)p_indices[j * 3];
			ind.y = (uint32_t)p_indices[j * 3 + 1];
			ind.z = (uint32_t)p_indices[j * 3 + 2];
		}
		else
		{
			ind.x = j * 3;
			ind.y = j * 3 + 1;
			ind.z = j * 3 + 2;
		}

		glm::vec3 v0 = p_pos[ind.x];
		glm::vec3 v1 = p_pos[ind.y];
		glm::vec3 v2 = p_pos[ind.z];
		glm::vec2 texCoord0 = p_uv[ind.x];
		glm::vec2 texCoord1 = p_uv[ind.y];
		glm::vec2 texCoord2 = p_uv[ind.z];

		glm::vec3 edge1 = v1 - v0;
		glm::vec3 edge2 = v2 - v0;
		glm::vec2 delta1 = texCoord1 - texCoord0;
		glm::vec2 delta2 = texCoord2 - texCoord0;

		float f = 1.0f / (delta1[0] * delta2[1] - delta2[0] * delta1[1]);
		glm::vec4 tagent = glm::vec4((f * delta2[1]) * edge1 - (f * delta1[1]) * edge2, 0.0f);
		glm::vec4 bitangent = glm::vec4((-f * delta2[0]) * edge1 + (f * delta1[0]) * edge2, 0.0f);

		p_tangent[ind.x] += tagent;
		p_tangent[ind.y] += tagent;
		p_tangent[ind.z] += tagent;

		p_bitangent[ind.x] += bitangent;
		p_bitangent[ind.y] += bitangent;
		p_bitangent[ind.z] += bitangent;

		counts[ind.x] += 1.0f;
		counts[ind.y] += 1.0f;
		counts[ind.z] += 1.0f;
	}

	for (int j = 0; j < num_pos; j++)
	{
		p_tangent[j] = p_tangent[j] / counts[j];
		p_bitangent[j] = p_bitangent[j] / counts[j];
	}
}

inline void g_calc_tangent(int num_face, int num_pos, int type_indices, const void* p_indices, const glm::vec4* p_pos, const glm::vec2* p_uv, glm::vec4* p_tangent, glm::vec4* p_bitangent)
{
	if (type_indices == 1)
	{
		t_calc_tangent<uint8_t>(num_face, num_pos, (uint8_t*)p_indices, p_pos, p_uv, p_tangent, p_bitangent);
	}
	if (type_indices == 2)
	{
		t_calc_tangent<uint16_t>(num_face, num_pos, (uint16_t*)p_indices, p_pos, p_uv, p_tangent, p_bitangent);
	}
	else if (type_indices == 4)
	{
		t_calc_tangent<uint32_t>(num_face, num_pos, (uint32_t*)p_indices, p_pos, p_uv, p_tangent, p_bitangent);
	}
}

template<typename T>
inline void t_fill_sparse_positions(int count, const T* p_indices, const glm::vec3* p_pos_in, glm::vec4* p_pos_out, int* p_none_zero)
{	
	for (int i = 0; i < count; i++)
	{
		T idx = p_indices[i];
		p_pos_out[idx] = glm::vec4(p_pos_in[i], 0.0f);
		p_none_zero[idx] = 1;
	}
}

inline void g_fill_sparse_positions(int count, int type_idx, const void* p_indices, const glm::vec3* p_pos_in, glm::vec4* p_pos_out, int* p_none_zero)
{
	if (type_idx == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
	{
		t_fill_sparse_positions<uint8_t>(count, (uint8_t*)p_indices, p_pos_in, p_pos_out, p_none_zero);
	}
	else if (type_idx == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
	{
		t_fill_sparse_positions<uint16_t>(count, (uint16_t*)p_indices, p_pos_in, p_pos_out, p_none_zero);
	}
	else if (type_idx == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
	{
		t_fill_sparse_positions<uint32_t>(count, (uint32_t*)p_indices, p_pos_in, p_pos_out, p_none_zero);
	}

}

template<typename T>
inline void t_copy_joints(int num_pos, const T* p_in, glm::uvec4* p_out)
{
	for (int i = 0; i < num_pos; i++)
	{
		const T& j_in = p_in[i];
		glm::uvec4& j_out = p_out[i];
		j_out.x = j_in.x;
		j_out.y = j_in.y;
		j_out.z = j_in.z;
		j_out.w = j_in.w;
	}
}

inline void load_animations(tinygltf::Model& model, std::vector<AnimationClip>& animations);

inline void load_model(tinygltf::Model& model, GLTFModel* model_out)
{
	struct TexLoadOptions
	{
		bool is_srgb = true;
		bool reverse = false;
	};

	size_t num_textures = model.textures.size();
	model_out->m_textures.resize(num_textures);
	std::vector<TexLoadOptions> tex_opts(num_textures);

	size_t num_materials = model.materials.size();
	model_out->m_materials.resize(num_materials);
	for (size_t i = 0; i < num_materials; i++)
	{
		tinygltf::Material& material_in = model.materials[i];
		MeshStandardMaterial* material_out = new MeshStandardMaterial();
		model_out->m_materials[i] = std::unique_ptr<MeshStandardMaterial>(material_out);
		if (material_in.alphaMode == "OPAQUE")
		{
			material_out->alphaMode = AlphaMode::Opaque;
		}
		else if (material_in.alphaMode == "MASK")
		{
			material_out->alphaMode = AlphaMode::Mask;
		}
		else if (material_in.alphaMode == "BLEND")
		{
			material_out->alphaMode = AlphaMode::Blend;
		}
		material_out->alphaCutoff = (float)material_in.alphaCutoff;
		material_out->doubleSided = material_in.doubleSided;

		tinygltf::PbrMetallicRoughness& pbr = material_in.pbrMetallicRoughness;
		material_out->color = { pbr.baseColorFactor[0], pbr.baseColorFactor[1], pbr.baseColorFactor[2], pbr.baseColorFactor[3] };
		material_out->tex_idx_map = pbr.baseColorTexture.index;

		material_out->metallicFactor = pbr.metallicFactor;
		material_out->roughnessFactor = pbr.roughnessFactor;

		int id_mr = pbr.metallicRoughnessTexture.index;
		if (id_mr >= 0)
		{
			tex_opts[id_mr].is_srgb = false;			
			material_out->tex_idx_metalnessMap = id_mr;
			material_out->tex_idx_roughnessMap = id_mr;
		}

		if (material_in.normalTexture.index >= 0)
		{
			tex_opts[material_in.normalTexture.index].is_srgb = false;
			material_out->tex_idx_normalMap = material_in.normalTexture.index;
			float scale = (float)material_in.normalTexture.scale;
			material_out->normalScale = { scale, scale };
		}

		material_out->emissive = { material_in.emissiveFactor[0], material_in.emissiveFactor[1], material_in.emissiveFactor[2] };
		material_out->tex_idx_emissiveMap = material_in.emissiveTexture.index;

		material_out->update_uniform();
	}

	for (size_t i = 0; i < num_textures; i++)
	{
		tinygltf::Texture& tex_in = model.textures[i];
		GLTexture2D* tex_out = new GLTexture2D();
		model_out->m_textures[i] = std::unique_ptr<GLTexture2D>(tex_out);

		tinygltf::Image& img_in = model.images[tex_in.source];
		model_out->m_tex_dict[img_in.name] = i;
		const TexLoadOptions& opts = tex_opts[i];

		bool has_alpha = img_in.component > 3;
		if (opts.reverse)
		{
			if (has_alpha)
			{
				tex_out->load_memory_bgra(img_in.width, img_in.height, img_in.image.data(), opts.is_srgb);
			}
			else
			{
				tex_out->load_memory_bgr(img_in.width, img_in.height, img_in.image.data(), opts.is_srgb);
			}
		}
		else
		{
			if (has_alpha)
			{
				tex_out->load_memory_rgba(img_in.width, img_in.height, img_in.image.data(), opts.is_srgb);
			}
			else
			{
				tex_out->load_memory_rgb(img_in.width, img_in.height, img_in.image.data(), opts.is_srgb);
			}
		}
	}

	size_t num_meshes = model.meshes.size();
	std::vector<bool> skinned(num_meshes, false);
	size_t num_nodes = model.nodes.size();	
	for (size_t i = 0; i < num_nodes; i++)
	{
		tinygltf::Node& node_in = model.nodes[i];
		if (node_in.mesh >= 0 && node_in.skin >= 0)
		{
			skinned[node_in.mesh] = true;
		}
	}

	
	model_out->m_meshs.resize(num_meshes);
	for (size_t i = 0; i < num_meshes; i++)
	{
		tinygltf::Mesh& mesh_in = model.meshes[i];
		Mesh& mesh_out = model_out->m_meshs[i];
		bool is_skinned = skinned[i];

		size_t num_primitives = mesh_in.primitives.size();
		mesh_out.primitives.resize(num_primitives);

		for (size_t j = 0; j < num_primitives; j++)
		{
			tinygltf::Primitive& primitive_in = mesh_in.primitives[j];
			Primitive& primitive_out = mesh_out.primitives[j];
			primitive_out.material_idx = primitive_in.material;

			MeshStandardMaterial* material = model_out->m_materials[primitive_out.material_idx].get();
			bool has_tangent = material->tex_idx_normalMap >= 0;

			int num_targets = primitive_in.targets.size();
			primitive_out.num_targets = num_targets;

			int num_geo_sets = 1;
			if (num_targets > 0) num_geo_sets++;
			if (is_skinned) num_geo_sets++;
			primitive_out.geometry.resize(num_geo_sets);

			int id_pos_in = primitive_in.attributes["POSITION"];
			tinygltf::Accessor& acc_pos_in = model.accessors[id_pos_in];
			primitive_out.num_pos = (int)acc_pos_in.count;
			tinygltf::BufferView& view_pos_in = model.bufferViews[acc_pos_in.bufferView];
			const glm::vec3* p_pos = (const glm::vec3*)(model.buffers[view_pos_in.buffer].data.data() + view_pos_in.byteOffset + acc_pos_in.byteOffset);

			primitive_out.min_pos = { acc_pos_in.minValues[0], acc_pos_in.minValues[1], acc_pos_in.minValues[2] };
			primitive_out.max_pos = { acc_pos_in.maxValues[0], acc_pos_in.maxValues[1], acc_pos_in.maxValues[2] };

			int id_indices_in = primitive_in.indices;
			const void* p_indices = nullptr;
			if (id_indices_in >= 0)
			{
				tinygltf::Accessor& acc_indices_in = model.accessors[id_indices_in];
				primitive_out.num_face = (int)(acc_indices_in.count / 3);
				tinygltf::BufferView& view_indices_in = model.bufferViews[acc_indices_in.bufferView];
				p_indices = model.buffers[view_indices_in.buffer].data.data() + view_indices_in.byteOffset + acc_indices_in.byteOffset;

				if (acc_indices_in.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
				{
					primitive_out.type_indices = 1;
				}
				else if (acc_indices_in.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
				{
					primitive_out.type_indices = 2;
				}
				else if (acc_indices_in.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
				{
					primitive_out.type_indices = 4;
				}
				primitive_out.index_buf = Attribute(new GLBuffer((size_t)primitive_out.type_indices * (size_t)primitive_out.num_face * 3, GL_ELEMENT_ARRAY_BUFFER));
				primitive_out.index_buf->upload(p_indices);

				primitive_out.cpu_indices = std::unique_ptr<std::vector<uint8_t>>(new std::vector<uint8_t>(primitive_out.index_buf->m_size));
				memcpy(primitive_out.cpu_indices->data(), p_indices, primitive_out.index_buf->m_size);
			}
			else
			{
				primitive_out.num_face = primitive_out.num_pos / 3;
			}

			GeometrySet& geometry = primitive_out.geometry[0];
			primitive_out.cpu_pos = std::unique_ptr<std::vector<glm::vec4>>(new std::vector<glm::vec4>(primitive_out.num_pos));

			for (int k = 0; k < primitive_out.num_pos; k++)
				(*primitive_out.cpu_pos)[k] = glm::vec4(p_pos[k], 1.0f);

			geometry.pos_buf = Attribute(new GLBuffer(sizeof(glm::vec4) * primitive_out.num_pos));
			geometry.pos_buf->upload(primitive_out.cpu_pos->data());

			std::vector<glm::vec4> norms(primitive_out.num_pos, glm::vec4(0.0f));

			geometry.normal_buf = Attribute(new GLBuffer(sizeof(glm::vec4) * primitive_out.num_pos));
			if (primitive_in.attributes.find("NORMAL") != primitive_in.attributes.end())
			{
				int id_norm_in = primitive_in.attributes["NORMAL"];
				tinygltf::Accessor& acc_norm_in = model.accessors[id_norm_in];
				tinygltf::BufferView& view_norm_in = model.bufferViews[acc_norm_in.bufferView];
				const glm::vec3* p_norm = (const glm::vec3*)(model.buffers[view_norm_in.buffer].data.data() + view_norm_in.byteOffset + acc_norm_in.byteOffset);

				for (int k = 0; k < primitive_out.num_pos; k++)
					norms[k] = glm::vec4(p_norm[k], 0.0f);
			}
			else
			{
				g_calc_normal(primitive_out.num_face, primitive_out.num_pos, primitive_out.type_indices, p_indices, primitive_out.cpu_pos->data(), norms.data());
			}
			geometry.normal_buf->upload(norms.data());

			if (primitive_in.attributes.find("COLOR_0") != primitive_in.attributes.end())
			{
				int id_color_in = primitive_in.attributes["COLOR_0"];
				tinygltf::Accessor& acc_color_in = model.accessors[id_color_in];
				tinygltf::BufferView& view_color_in = model.bufferViews[acc_color_in.bufferView];

				primitive_out.color_buf = Attribute(new GLBuffer(sizeof(glm::vec4) * primitive_out.num_pos));
				primitive_out.color_buf->upload(model.buffers[view_color_in.buffer].data.data() + view_color_in.byteOffset + acc_color_in.byteOffset);
			}

			const glm::vec2* p_uv = nullptr;

			if (primitive_in.attributes.find("TEXCOORD_0") != primitive_in.attributes.end())
			{
				int id_uv_in = primitive_in.attributes["TEXCOORD_0"];
				tinygltf::Accessor& acc_uv_in = model.accessors[id_uv_in];
				tinygltf::BufferView& view_uv_in = model.bufferViews[acc_uv_in.bufferView];

				p_uv = (const glm::vec2*)(model.buffers[view_uv_in.buffer].data.data() + view_uv_in.byteOffset + acc_uv_in.byteOffset);
				primitive_out.uv_buf = Attribute(new GLBuffer(sizeof(glm::vec2) * primitive_out.num_pos));
				primitive_out.uv_buf->upload(p_uv);
			}

			if (primitive_in.attributes.find("JOINTS_0") != primitive_in.attributes.end())
			{
				int id_joints_in = primitive_in.attributes["JOINTS_0"];
				tinygltf::Accessor& acc_joints_in = model.accessors[id_joints_in];
				tinygltf::BufferView& view_joints_in = model.bufferViews[acc_joints_in.bufferView];
				std::vector<glm::uvec4> joints(primitive_out.num_pos);

				void* p_joints = model.buffers[view_joints_in.buffer].data.data() + view_joints_in.byteOffset + acc_joints_in.byteOffset;
				if (acc_joints_in.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
				{
					t_copy_joints(primitive_out.num_pos, (glm::u8vec4*)p_joints, joints.data());
				}
				else if (acc_joints_in.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
				{
					t_copy_joints(primitive_out.num_pos, (glm::u16vec4*)p_joints, joints.data());
				}
				else if (acc_joints_in.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
				{
					t_copy_joints(primitive_out.num_pos, (glm::u32vec4*)p_joints, joints.data());
				}

				primitive_out.joints_buf = Attribute(new GLBuffer(sizeof(glm::uvec4) * primitive_out.num_pos));
				primitive_out.joints_buf->upload(joints.data());
			}

			if (primitive_in.attributes.find("WEIGHTS_0") != primitive_in.attributes.end())
			{
				int id_weights_in = primitive_in.attributes["WEIGHTS_0"];
				tinygltf::Accessor& acc_weights_in = model.accessors[id_weights_in];
				tinygltf::BufferView& view_weights_in = model.bufferViews[acc_weights_in.bufferView];

				primitive_out.weights_buf = Attribute(new GLBuffer(sizeof(glm::vec4) * primitive_out.num_pos));
				primitive_out.weights_buf->upload(model.buffers[view_weights_in.buffer].data.data() + view_weights_in.byteOffset + acc_weights_in.byteOffset);
			}

			std::vector<glm::vec4> tangent;
			std::vector<glm::vec4> bitangent;
			if (has_tangent)
			{
				tangent.resize(primitive_out.num_pos, glm::vec4(0.0f));
				bitangent.resize(primitive_out.num_pos, glm::vec4(0.0f));
				g_calc_tangent(primitive_out.num_face, primitive_out.num_pos, primitive_out.type_indices, p_indices, primitive_out.cpu_pos->data(), p_uv, tangent.data(), bitangent.data());
				geometry.tangent_buf = Attribute(new GLBuffer(sizeof(glm::vec4) * primitive_out.num_pos));
				geometry.bitangent_buf = Attribute(new GLBuffer(sizeof(glm::vec4) * primitive_out.num_pos));
				geometry.tangent_buf->upload(tangent.data());
				geometry.bitangent_buf->upload(bitangent.data());
			}

			if (num_targets > 0)
			{
				std::vector<glm::vec4> pos_targets((size_t)num_targets * (size_t)primitive_out.num_pos, glm::vec4(0.0f));
				std::vector<glm::vec4> norm_targets((size_t)num_targets * (size_t)primitive_out.num_pos, glm::vec4(0.0f));
				std::vector<glm::vec4> tangent_targets;
				std::vector<glm::vec4> bitangent_targets;
				if (has_tangent)
				{
					tangent_targets.resize((size_t)num_targets * (size_t)primitive_out.num_pos, glm::vec4(0.0f));
					bitangent_targets.resize((size_t)num_targets * (size_t)primitive_out.num_pos, glm::vec4(0.0f));
				}

				bool sparse = true;
				std::vector<int> non_zero((size_t)primitive_out.num_pos, 0);

				for (size_t k = 0; k < num_targets; k++)
				{
					std::map<std::string, int>& target_in = primitive_in.targets[k];

					int id_t_pos_in = target_in["POSITION"];
					tinygltf::Accessor& acc_t_pos_in = model.accessors[id_t_pos_in];
					
					glm::vec4* pos_delta_k = pos_targets.data() + k * primitive_out.num_pos;
					if (!acc_t_pos_in.sparse.isSparse)
					{
						sparse = false;
						tinygltf::BufferView& view_t_pos_in = model.bufferViews[acc_t_pos_in.bufferView];
						const glm::vec3* t_pos = (const glm::vec3*)(model.buffers[view_t_pos_in.buffer].data.data() + view_t_pos_in.byteOffset + acc_t_pos_in.byteOffset);
						for (int l = 0; l < primitive_out.num_pos; l++)
							pos_delta_k[l] = glm::vec4(t_pos[l], 0.0f);
					}
					else
					{
						int count_idx = acc_t_pos_in.sparse.count;
						tinygltf::BufferView& view_idx = model.bufferViews[acc_t_pos_in.sparse.indices.bufferView];
						const void* p_idx = model.buffers[view_idx.buffer].data.data() + view_idx.byteOffset + acc_t_pos_in.sparse.indices.byteOffset;
						int idx_type = acc_t_pos_in.sparse.indices.componentType;
						tinygltf::BufferView& view_value = model.bufferViews[acc_t_pos_in.sparse.values.bufferView];						
						const glm::vec3* p_value = (const glm::vec3*)(model.buffers[view_value.buffer].data.data() + view_value.byteOffset + acc_t_pos_in.sparse.values.byteOffset);
						g_fill_sparse_positions(count_idx, idx_type, p_idx, p_value, pos_delta_k, non_zero.data());
					}

					std::vector<glm::vec4> pos_target_k((size_t)primitive_out.num_pos);
					glm::vec4* pos_k = pos_target_k.data();
					
					for (int l = 0; l < primitive_out.num_pos; l++)
						pos_k[l] = glm::vec4(p_pos[l], 1.0f) + pos_delta_k[l];

					glm::vec4* norm_k = norm_targets.data() + k * primitive_out.num_pos;
					if (target_in.find("NORMAL") != target_in.end())
					{
						int id_t_norm_in = target_in["NORMAL"];
						tinygltf::Accessor& acc_t_norm_in = model.accessors[id_t_norm_in];

						if (!acc_t_norm_in.sparse.isSparse)
						{
							sparse = false;
							tinygltf::BufferView& view_t_norm_in = model.bufferViews[acc_t_norm_in.bufferView];
							const glm::vec3* t_norm = (const glm::vec3*)(model.buffers[view_t_norm_in.buffer].data.data() + view_t_norm_in.byteOffset + acc_t_norm_in.byteOffset);
							for (int l = 0; l < primitive_out.num_pos; l++)
								norm_k[l] = glm::vec4(t_norm[l], 0.0f);
						}
						else
						{
							int count_idx = acc_t_norm_in.sparse.count;
							tinygltf::BufferView& view_idx = model.bufferViews[acc_t_norm_in.sparse.indices.bufferView];
							const void* p_idx = model.buffers[view_idx.buffer].data.data() + view_idx.byteOffset + acc_t_norm_in.sparse.indices.byteOffset;
							int idx_type = acc_t_norm_in.sparse.indices.componentType;
							tinygltf::BufferView& view_value = model.bufferViews[acc_t_norm_in.sparse.values.bufferView];
							const glm::vec3* p_value = (const glm::vec3*)(model.buffers[view_value.buffer].data.data() + view_value.byteOffset + acc_t_norm_in.sparse.values.byteOffset);
							g_fill_sparse_positions(count_idx, idx_type, p_idx, p_value, norm_k, non_zero.data());
						}						
					}
					else
					{
						g_calc_normal(primitive_out.num_face, primitive_out.num_pos, primitive_out.type_indices, p_indices, pos_k, norm_k);
						for (int l = 0; l < primitive_out.num_pos; l++)
						{
							glm::vec4 norm_delta = norm_k[l] - norms[l];
							norm_k[l] = norm_delta;
							if (sparse && (norm_delta.x != 0.0f || norm_delta.y != 0.0f || norm_delta.z != 0.0f))
							{
								non_zero[l] = 1;
							}								
						}
					}

					if (has_tangent)
					{
						glm::vec4* tangent_k = tangent_targets.data() + k * primitive_out.num_pos;
						glm::vec4* bitangent_k = bitangent_targets.data() + k * primitive_out.num_pos;
						g_calc_tangent(primitive_out.num_face, primitive_out.num_pos, primitive_out.type_indices, p_indices, pos_k, p_uv, tangent_k, bitangent_k);

						for (int l = 0; l < primitive_out.num_pos; l++)
						{
							glm::vec4 tan_delta= tangent_k[l] - tangent[l];
							glm::vec4 bitan_delta = bitangent_k[l] - bitangent[l];
							tangent_k[l] = tan_delta;
							bitangent_k[l] = bitan_delta;

							if (sparse && (
								tan_delta.x != 0.0f || tan_delta.y != 0.0f || tan_delta.z != 0.0f || 
								bitan_delta.x != 0.0f || bitan_delta.y != 0.0f || bitan_delta.z != 0.0f))
							{
								non_zero[l] = 1;
							}
						}
					}
				}

				primitive_out.targets.pos_buf = Attribute(new GLBuffer(sizeof(glm::vec4) * primitive_out.num_pos * num_targets));
				primitive_out.targets.pos_buf->upload(pos_targets.data());
				primitive_out.targets.normal_buf = Attribute(new GLBuffer(sizeof(glm::vec4) * primitive_out.num_pos * num_targets));
				primitive_out.targets.normal_buf->upload(norm_targets.data());
				if (has_tangent)
				{
					primitive_out.targets.tangent_buf = Attribute(new GLBuffer(sizeof(glm::vec4) * primitive_out.num_pos * num_targets));
					primitive_out.targets.tangent_buf->upload(tangent_targets.data());
					primitive_out.targets.bitangent_buf = Attribute(new GLBuffer(sizeof(glm::vec4) * primitive_out.num_pos * num_targets));
					primitive_out.targets.bitangent_buf->upload(bitangent_targets.data());
				}

				if (sparse)
				{
					primitive_out.none_zero_buf = Attribute(new GLBuffer(sizeof(int) * primitive_out.num_pos));
					primitive_out.none_zero_buf->upload(non_zero.data());
				}
			}


			for (int k = 1; k < num_geo_sets; k++)
			{
				GeometrySet& geometry2 = primitive_out.geometry[k];

				// passthrough 
				geometry2.pos_buf = Attribute(new GLBuffer(geometry.pos_buf->m_size));
				*geometry2.pos_buf = *geometry.pos_buf;

				geometry2.normal_buf = Attribute(new GLBuffer(geometry.normal_buf->m_size));
				*geometry2.normal_buf = *geometry.normal_buf;

				if (has_tangent)
				{
					geometry2.tangent_buf = Attribute(new GLBuffer(geometry.tangent_buf->m_size));
					*geometry2.tangent_buf = *geometry.tangent_buf;

					geometry2.bitangent_buf = Attribute(new GLBuffer(geometry.bitangent_buf->m_size));
					*geometry2.bitangent_buf = *geometry.bitangent_buf;
				}
			}
		}
		if (mesh_out.primitives.size() > 0)
		{
			if (mesh_out.primitives[0].num_targets > 0)
			{
				mesh_out.weights.resize(mesh_out.primitives[0].num_targets, 0.0f);				
				mesh_out.buf_weights = std::unique_ptr<GLDynBuffer>(new GLDynBuffer(sizeof(float) * mesh_out.primitives[0].num_targets, GL_SHADER_STORAGE_BUFFER));
			}
		}
	}
	
	model_out->m_nodes.resize(num_nodes);
	for (size_t i = 0; i < num_nodes; i++)
	{
		tinygltf::Node& node_in = model.nodes[i];
		Node& node_out = model_out->m_nodes[i];		
		node_out.children = node_in.children;

		if (node_in.matrix.size() > 0)
		{
			glm::mat4 matrix;
			for (int c = 0; c < 16; c++)
			{
				matrix[c/4][c%4] = (float)node_in.matrix[c];
			}
			glm::quat rot;
			glm::vec3 skew;
			glm::vec4 persp;
			glm::decompose(matrix, node_out.scale, node_out.rotation, node_out.translation, skew, persp);
		}
		else
		{
			if (node_in.translation.size() > 0)
			{
				node_out.translation.x = (float)node_in.translation[0];
				node_out.translation.y = (float)node_in.translation[1];
				node_out.translation.z = (float)node_in.translation[2];
			}
			else
			{
				node_out.translation = { 0.0f, 0.0f, 0.0f };
			}

			if (node_in.rotation.size() > 0)
			{
				node_out.rotation.x = (float)node_in.rotation[0];
				node_out.rotation.y = (float)node_in.rotation[1];
				node_out.rotation.z = (float)node_in.rotation[2];
				node_out.rotation.w = (float)node_in.rotation[3];
			}
			else
			{
				node_out.rotation = glm::identity<glm::quat>();
			}

			if (node_in.scale.size() > 0)
			{
				node_out.scale.x = (float)node_in.scale[0];
				node_out.scale.y = (float)node_in.scale[1];
				node_out.scale.z = (float)node_in.scale[2];
			}
			else
			{
				node_out.scale = { 1.0f, 1.0f, 1.0f };
			}
		}

		std::string name = node_in.name;
		if (name == "")
		{
			char node_name[32];
			sprintf(node_name, "node_%d", (int)i);
			name = node_name;
		}

		model_out->m_node_dict[name] = i;
	}
	model_out->m_roots = model.scenes[0].nodes;

	size_t num_skins = model.skins.size();
	model_out->m_skins.resize(num_skins);
	for (size_t i = 0; i < num_skins; i++)
	{
		tinygltf::Skin& skin_in = model.skins[i];
		Skin& skin_out = model_out->m_skins[i];		
		size_t num_joints = skin_in.joints.size();
		skin_out.joints = skin_in.joints;
		skin_out.inverseBindMatrices.resize(num_joints);
		tinygltf::Accessor& acc_mats = model.accessors[skin_in.inverseBindMatrices];
		tinygltf::BufferView& view_mats = model.bufferViews[acc_mats.bufferView];
		memcpy(skin_out.inverseBindMatrices.data(), model.buffers[view_mats.buffer].data.data() + view_mats.byteOffset + acc_mats.byteOffset, sizeof(glm::mat4)* num_joints);
		skin_out.buf_rela_mat = std::unique_ptr<GLDynBuffer>(new GLDynBuffer(sizeof(glm::mat4) * num_joints));
	}

	for (size_t i = 0; i < num_nodes; i++)
	{
		tinygltf::Node& node_in = model.nodes[i];
		Node& node_out = model_out->m_nodes[i];
		int j = node_in.mesh;

		if (j >= 0)
		{	
			Mesh& mesh_out = model_out->m_meshs[j];
			mesh_out.node_id = i;
			mesh_out.skin_id = node_in.skin;

			std::string name = node_in.name;
			if (name == "")
			{
				char mesh_name[32];
				sprintf(mesh_name, "mesh_%d", j);
				name = mesh_name;
			}
			model_out->m_mesh_dict[name] = j;
		}
	}	

	model_out->updateNodes();
	model_out->calculate_bounding_box();

	load_animations(model, model_out->m_animations);

	model_out->buildAnimDict();

}

inline void load_animations(tinygltf::Model& model, std::vector<AnimationClip>& animations)
{
	size_t num_anims = model.animations.size();
	animations.resize(num_anims);
	for (size_t i = 0; i < num_anims; i++)
	{
		tinygltf::Animation& anim_in = model.animations[i];
		AnimationClip& anim_out = animations[i];
		anim_out.name = anim_in.name;

		size_t num_tracks = anim_in.channels.size();
		for (size_t j = 0; j < num_tracks; j++)
		{
			tinygltf::AnimationChannel& track_in = anim_in.channels[j];
			tinygltf::AnimationSampler& sampler = anim_in.samplers[track_in.sampler];
			Interpolation interpolation = Interpolation::Linear;
			if (sampler.interpolation == "STEP")
			{
				interpolation = Interpolation::Step;
			}
			else if (sampler.interpolation == "LINEAR")
			{
				interpolation = Interpolation::Linear;
			}
			else if (sampler.interpolation == "CUBICSPLINE")
			{
				interpolation = Interpolation::CubicSpline;
			}

			tinygltf::Accessor& acc_input = model.accessors[sampler.input];
			tinygltf::Accessor& acc_output = model.accessors[sampler.output];
			tinygltf::BufferView& view_input = model.bufferViews[acc_input.bufferView];
			tinygltf::BufferView& view_output = model.bufferViews[acc_output.bufferView];

			size_t num_inputs = acc_input.count;
			float* p_input = (float*)(model.buffers[view_input.buffer].data.data() + view_input.byteOffset + acc_input.byteOffset);
			void* p_output = model.buffers[view_output.buffer].data.data() + view_output.byteOffset + acc_output.byteOffset;

			double end = acc_input.maxValues[0];
			if (end > anim_out.duration) anim_out.duration = end;

			if (track_in.target_path == "weights")
			{
				tinygltf::Node& node_in = model.nodes[track_in.target_node];
				tinygltf::Mesh& mesh_in = model.meshes[node_in.mesh];
				MorphTrack track_out;
				track_out.name = node_in.name;
				track_out.num_targets = (int)mesh_in.primitives[0].targets.size();
				track_out.interpolation = interpolation;

				track_out.times.resize(num_inputs);
				memcpy(track_out.times.data(), p_input, sizeof(float) * num_inputs);

				if (interpolation == Interpolation::Step || interpolation == Interpolation::Linear)
				{
					track_out.values.resize(num_inputs * track_out.num_targets);
					memcpy(track_out.values.data(), p_output, sizeof(float) * num_inputs * track_out.num_targets);
				}
				else if (interpolation == Interpolation::CubicSpline)
				{
					track_out.values.resize(num_inputs * track_out.num_targets * 3);
					memcpy(track_out.values.data(), p_output, sizeof(float) * num_inputs * track_out.num_targets * 3);
				}

				anim_out.morphs.push_back(track_out);
			}
			else if (track_in.target_path == "translation")
			{
				tinygltf::Node& node_in = model.nodes[track_in.target_node];
				TranslationTrack track_out;
				track_out.name = node_in.name;
				track_out.interpolation = interpolation;

				track_out.times.resize(num_inputs);
				memcpy(track_out.times.data(), p_input, sizeof(float) * num_inputs);

				if (interpolation == Interpolation::Step || interpolation == Interpolation::Linear)
				{
					track_out.values.resize(num_inputs);
					memcpy(track_out.values.data(), p_output, sizeof(glm::vec3) * num_inputs);
				}
				else if (interpolation == Interpolation::CubicSpline)
				{
					track_out.values.resize(num_inputs *  3);
					memcpy(track_out.values.data(), p_output, sizeof(glm::vec3) * num_inputs * 3);
				}

				anim_out.translations.push_back(track_out);
			}
			else if (track_in.target_path == "rotation")
			{
				tinygltf::Node& node_in = model.nodes[track_in.target_node];
				RotationTrack track_out;
				track_out.name = node_in.name;
				track_out.interpolation = interpolation;

				track_out.times.resize(num_inputs);
				memcpy(track_out.times.data(), p_input, sizeof(float) * num_inputs);

				if (interpolation == Interpolation::Step || interpolation == Interpolation::Linear)
				{
					track_out.values.resize(num_inputs);
					float* f_output = (float*)p_output;
					for (size_t i = 0; i < num_inputs; i++)
					{
						glm::quat& rot = track_out.values[i];
						rot.x = f_output[4 * i];
						rot.y = f_output[4 * i + 1];
						rot.z = f_output[4 * i + 2];
						rot.w = f_output[4 * i + 3];
					}					
				}
				else if (interpolation == Interpolation::CubicSpline)
				{
					track_out.values.resize(num_inputs*3);
					float* f_output = (float*)p_output;
					for (size_t i = 0; i < num_inputs * 3; i++)
					{
						glm::quat& rot = track_out.values[i];
						rot.x = f_output[4 * i];
						rot.y = f_output[4 * i + 1];
						rot.z = f_output[4 * i + 2];
						rot.w = f_output[4 * i + 3];
					}

				}

				anim_out.rotations.push_back(track_out);
			}
			else if (track_in.target_path == "scale")
			{
				tinygltf::Node& node_in = model.nodes[track_in.target_node];
				ScaleTrack track_out;
				track_out.name = node_in.name;
				track_out.interpolation = interpolation;

				track_out.times.resize(num_inputs);
				memcpy(track_out.times.data(), p_input, sizeof(float) * num_inputs);

				if (interpolation == Interpolation::Step || interpolation == Interpolation::Linear)
				{
					track_out.values.resize(num_inputs);
					memcpy(track_out.values.data(), p_output, sizeof(glm::vec3) * num_inputs);
				}
				else if (interpolation == Interpolation::CubicSpline)
				{
					track_out.values.resize(num_inputs * 3);
					memcpy(track_out.values.data(), p_output, sizeof(glm::vec3) * num_inputs * 3);
				}

				anim_out.scales.push_back(track_out);
			}

		}
	}
}

void GLTFLoader::LoadModelFromFile(GLTFModel* model_out, const char* filename)
{
	std::string err;
	std::string warn;
	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	loader.LoadBinaryFromFile(&model, &err, &warn, filename);
	load_model(model, model_out);
}

void GLTFLoader::LoadAnimationsFromFile(std::vector<AnimationClip>& animations, const char* filename)
{
	std::string err;
	std::string warn;
	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	loader.LoadBinaryFromFile(&model, &err, &warn, filename);
	load_animations(model, animations);
}

void GLTFLoader::LoadModelFromMemory(GLTFModel* model_out, unsigned char* data, size_t size)
{
	std::string err;
	std::string warn;
	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	loader.LoadBinaryFromMemory(&model, &err, &warn, data, size);
	load_model(model, model_out);
}

void GLTFLoader::LoadAnimationsFromMemory(std::vector<AnimationClip>& animations, unsigned char* data, size_t size)
{
	std::string err;
	std::string warn;
	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	loader.LoadBinaryFromMemory(&model, &err, &warn, data, size);
	load_animations(model, animations);
}
