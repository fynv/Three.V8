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

	size_t num_meshes = model.meshes.size();
	model_out->m_meshs.resize(num_meshes);
	for (size_t i = 0; i < num_meshes; i++)
	{
		tinygltf::Mesh& mesh_in = model.meshes[i];
		Mesh& mesh_out = model_out->m_meshs[i];

		size_t num_primitives = mesh_in.primitives.size();
		mesh_out.primitives.resize(num_primitives);

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
				primitive_out.index_buf = Attribute(new GLBuffer(primitive_out.type_indices * primitive_out.num_face * 3, GL_ELEMENT_ARRAY_BUFFER));
				primitive_out.index_buf->upload(p_indices);
			}
			else
			{
				primitive_out.num_face = primitive_out.num_pos / 3;
			}

			GeometrySet& geometry = primitive_out.geometry[0];
			geometry.pos_buf = Attribute(new GLBuffer(sizeof(glm::vec3) * primitive_out.num_pos));
			geometry.pos_buf->upload(p_pos);

			geometry.normal_buf = Attribute(new GLBuffer(sizeof(glm::vec3) * primitive_out.num_pos));
			if (primitive_in.attributes.find("NORMAL") != primitive_in.attributes.end())
			{
				int id_norm_in = primitive_in.attributes["NORMAL"];
				tinygltf::Accessor& acc_norm_in = model.accessors[id_norm_in];
				tinygltf::BufferView& view_norm_in = model.bufferViews[acc_norm_in.bufferView];
				geometry.normal_buf->upload(p_data_in + view_norm_in.byteOffset + acc_norm_in.byteOffset);
			}
			else
			{
				std::vector<glm::vec3> normal(primitive_out.num_pos);
				if (primitive_out.type_indices == 2)
				{
					g_calc_normal<uint16_t>(primitive_out.num_face, primitive_out.num_pos, (uint16_t*)p_indices, p_pos, normal.data());
				}
				else if (primitive_out.type_indices == 4)
				{
					g_calc_normal<uint32_t>(primitive_out.num_face, primitive_out.num_pos, (uint32_t*)p_indices, p_pos, normal.data());
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
		}
	}

}