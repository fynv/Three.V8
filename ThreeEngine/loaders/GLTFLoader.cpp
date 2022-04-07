#include <unordered_map>
#include <GL/glew.h>
#include "GLTFLoader.h"

#include "models/ModelComponents.h"
#include "models/GLTFModel.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_ENABLE_DRACO
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

template<typename T>
inline void g_calc_normal(int num_face, int num_pos, const T* p_indices, const glm::vec3* p_pos, glm::vec3* p_norm)
{
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
		glm::vec3 face_normals = glm::normalize(glm::cross(v1 - v0, v2 - v0));

		p_norm[ind.x] += face_normals;
		p_norm[ind.y] += face_normals;
		p_norm[ind.z] += face_normals;
	}

	for (int j = 0; j < num_pos; j++)
		p_norm[j] = glm::normalize(glm::vec3(p_norm[j]));
}

void GLTFLoader::LoadModelFromFile(GLTFModel* model_out, const char* filename)
{
	std::string err;
	std::string warn;
	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	loader.LoadBinaryFromFile(&model, &err, &warn, filename);

	tinygltf::Buffer& buffer_in = model.buffers[0];
	uint8_t* p_data_in = buffer_in.data.data();

	struct MeshTransform
	{
		glm::mat4 mat;
		glm::mat4 norm_mat;
	};

	std::unordered_map<int, MeshTransform> mesh_node_map;

	size_t num_nodes = model.nodes.size();
	for (size_t i = 0; i < num_nodes; i++)
	{
		tinygltf::Node& node_in = model.nodes[i];
		if (node_in.mesh >= 0)
		{
			MeshTransform trans;
			trans.mat = glm::identity<glm::mat4>();
			
			if (node_in.translation.size() > 0)
			{
				trans.mat = glm::translate(trans.mat, { node_in.translation[0],  node_in.translation[1],  node_in.translation[2] });				
			}
			
			if (node_in.rotation.size() > 0)
			{
				glm::quat quaternion(node_in.rotation[3], node_in.rotation[0], node_in.rotation[1], node_in.rotation[2]);
				trans.mat *= glm::toMat4(quaternion);
			}

			if (node_in.scale.size() > 0)
			{
				trans.mat *= glm::scale(trans.mat, { node_in.scale[0],  node_in.scale[1],  node_in.scale[2] });
			}

			trans.norm_mat = glm::transpose(glm::inverse(trans.mat));			
		
			mesh_node_map[node_in.mesh] = trans;
		}
	}

	size_t num_meshes = model.meshes.size();
	model_out->m_meshs.resize(num_meshes);
	for (size_t i = 0; i < num_meshes; i++)
	{
		tinygltf::Mesh& mesh_in = model.meshes[i];
		Mesh& mesh_out = model_out->m_meshs[i];

		size_t num_primitives = mesh_in.primitives.size();
		mesh_out.primitives.resize(num_primitives);

		MeshTransform trans = mesh_node_map[i];

		for (size_t j = 0; j < num_primitives; j++)
		{
			tinygltf::Primitive& primitive_in = mesh_in.primitives[j];
			Primitive& primitive_out = mesh_out.primitives[j];

			primitive_out.has_blendshape = primitive_in.targets.size() > 0;

			int num_geo_sets = 1;
			if (primitive_out.has_blendshape) num_geo_sets++;
			primitive_out.geometry.resize(num_geo_sets);

			int id_pos_in = primitive_in.attributes["POSITION"];
			tinygltf::Accessor& acc_pos_in = model.accessors[id_pos_in];
			primitive_out.num_pos = acc_pos_in.count;
			tinygltf::BufferView& view_pos_in = model.bufferViews[acc_pos_in.bufferView];
			glm::vec3* p_pos = (glm::vec3*)(p_data_in + view_pos_in.byteOffset + acc_pos_in.byteOffset);

			int id_indices_in = primitive_in.indices;
			void* p_indices = nullptr;
			if (id_indices_in >= 0)
			{
				tinygltf::Accessor& acc_indices_in = model.accessors[id_indices_in];
				primitive_out.num_face = acc_indices_in.count / 3;
				tinygltf::BufferView& view_indices_in = model.bufferViews[acc_indices_in.bufferView];
				p_indices = p_data_in + view_indices_in.byteOffset + acc_indices_in.byteOffset;

				if (acc_indices_in.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
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
			primitive_out.cpu_pos = std::unique_ptr<std::vector<glm::vec3>>(new std::vector<glm::vec3>(primitive_out.num_pos));
			memcpy(primitive_out.cpu_pos->data(), p_pos, sizeof(glm::vec3)* primitive_out.num_pos);

			for (int k = 0; k < primitive_out.num_pos; k++)
			{
				glm::vec3& pos = (*primitive_out.cpu_pos)[k];
				pos = trans.mat * glm::vec4(pos, 1.0f);
			}

			geometry.pos_buf = Attribute(new GLBuffer(sizeof(glm::vec3) * primitive_out.num_pos));
			geometry.pos_buf->upload(primitive_out.cpu_pos->data());

			geometry.normal_buf = Attribute(new GLBuffer(sizeof(glm::vec3) * primitive_out.num_pos));
			if (primitive_in.attributes.find("NORMAL") != primitive_in.attributes.end())
			{
				int id_norm_in = primitive_in.attributes["NORMAL"];
				tinygltf::Accessor& acc_norm_in = model.accessors[id_norm_in];
				tinygltf::BufferView& view_norm_in = model.bufferViews[acc_norm_in.bufferView];
				glm::vec3* p_norm = (glm::vec3 *)(p_data_in + view_norm_in.byteOffset + acc_norm_in.byteOffset);

				std::vector<glm::vec3> norms(primitive_out.num_pos);
				memcpy(norms.data(), p_norm, sizeof(glm::vec3)* primitive_out.num_pos);

				for (int k = 0; k < primitive_out.num_pos; k++)
				{
					glm::vec3& norm = norms[k];
					norm = trans.norm_mat * glm::vec4(norm, 0.0f);
				}

				geometry.normal_buf->upload(norms.data());
			}
			else
			{
				std::vector<glm::vec3> normal(primitive_out.num_pos);
				if (primitive_out.type_indices == 2)
				{
					g_calc_normal<uint16_t>(primitive_out.num_face, primitive_out.num_pos, (uint16_t*)p_indices, primitive_out.cpu_pos->data(), normal.data());
				}
				else if (primitive_out.type_indices == 4)
				{
					g_calc_normal<uint32_t>(primitive_out.num_face, primitive_out.num_pos, (uint32_t*)p_indices, primitive_out.cpu_pos->data(), normal.data());
				}
				geometry.normal_buf->upload(normal.data());
			}

			if (primitive_in.attributes.find("COLOR_0") != primitive_in.attributes.end())
			{				
				int id_color_in = primitive_in.attributes["COLOR_0"];
				tinygltf::Accessor& acc_color_in = model.accessors[id_color_in];
				tinygltf::BufferView& view_color_in = model.bufferViews[acc_color_in.bufferView];

				primitive_out.type_color = acc_color_in.type;
				primitive_out.color_buf = Attribute(new GLBuffer(sizeof(float)* primitive_out.type_color * primitive_out.num_pos));
				primitive_out.color_buf->upload(p_data_in + view_color_in.byteOffset + acc_color_in.byteOffset);
			}

			if (primitive_in.attributes.find("TEXCOORD_0") != primitive_in.attributes.end())
			{				
				int id_uv_in = primitive_in.attributes["TEXCOORD_0"];
				tinygltf::Accessor& acc_uv_in = model.accessors[id_uv_in];
				tinygltf::BufferView& view_uv_in = model.bufferViews[acc_uv_in.bufferView];

				primitive_out.uv_buf = Attribute(new GLBuffer(sizeof(glm::vec2) * primitive_out.num_pos));
				primitive_out.uv_buf->upload(p_data_in + view_uv_in.byteOffset + acc_uv_in.byteOffset);
			}

			for (int k = 1; k < num_geo_sets; k++)
			{
				GeometrySet& geometry2 = primitive_out.geometry[k];

				// passthrough 
				geometry2.pos_buf = Attribute(new GLBuffer(geometry.pos_buf->m_size));
				*geometry2.pos_buf = *geometry.pos_buf;

				geometry2.normal_buf = Attribute(new GLBuffer(geometry.normal_buf->m_size));
				*geometry2.normal_buf = *geometry.normal_buf;
			}
		}
	}

}