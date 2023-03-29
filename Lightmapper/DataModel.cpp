#include <stb_image.h>
#include <stb_image_write.h>
#include <gtx/matrix_decompose.hpp>
#include <gtx/hash.hpp>
#include <gtx/string_cast.hpp>

#include <models/GLTFModel.h>

#include "DataModel.h"
#include "xatlas.h"

inline void calc_normal(int num_face, int num_pos, const glm::ivec3* p_indices, const glm::vec4* p_pos, glm::vec4* p_norm)
{
	std::vector<float> counts(num_pos, 0.0f);
	for (int j = 0; j < num_face; j++)
	{
		glm::ivec3 ind = p_indices[j];

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
		p_norm[j] = p_norm[j] / counts[j];
}

inline void calc_normal(int num_face, int num_pos, const glm::ivec3* p_indices, const glm::vec3* p_pos, glm::vec3* p_norm)
{
	std::vector<float> counts(num_pos, 0.0f);
	for (int j = 0; j < num_face; j++)
	{
		glm::ivec3 ind = p_indices[j];

		glm::vec3 v0 = p_pos[ind.x];
		glm::vec3 v1 = p_pos[ind.y];
		glm::vec3 v2 = p_pos[ind.z];
		glm::vec3 face_normals = glm::cross(v1 - v0, v2 - v0);

		p_norm[ind.x] += face_normals;
		p_norm[ind.y] += face_normals;
		p_norm[ind.z] += face_normals;
		counts[ind.x] += 1.0f;
		counts[ind.y] += 1.0f;
		counts[ind.z] += 1.0f;
	}

	for (int j = 0; j < num_pos; j++)
		p_norm[j] = p_norm[j] / counts[j];
}

inline void calc_tangent(int num_face, int num_pos, const glm::ivec3* p_indices, const glm::vec4* p_pos, const glm::vec2* p_uv, glm::vec4* p_tangent, glm::vec4* p_bitangent)
{
	std::vector<float> counts(num_pos, 0.0f);
	for (int j = 0; j < num_face; j++)
	{
		glm::ivec3 ind = p_indices[j];

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

void DataModel::LoadGlb(const char* fn_glb)
{
	std::string err;
	std::string warn;
	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	loader.LoadBinaryFromFile(&model, &err, &warn, fn_glb);

	size_t num_textures = model.textures.size();
	textures.resize(num_textures);

	for (size_t i = 0; i < num_textures; i++)
	{
		tinygltf::Texture& tex_in = model.textures[i];
		tinygltf::Image& img_in = model.images[tex_in.source];
		Image& tex_out = textures[i];
		tex_out.name = img_in.name;
		tex_out.width = img_in.width;
		tex_out.height = img_in.height;
		tex_out.mimeType = img_in.mimeType;
		tex_out.data = img_in.image;

		if (img_in.bufferView >= 0)
		{
			tinygltf::BufferView& buf_view = model.bufferViews[img_in.bufferView];
			unsigned char* data = model.buffers[buf_view.buffer].data.data() + buf_view.byteOffset;
			tex_out.storage.resize(buf_view.byteLength);
			memcpy(tex_out.storage.data(), data, buf_view.byteLength);
		}
	}

	materials = model.materials;
	nodes = model.nodes;
	scene = model.scenes[0];

	size_t num_nodes = model.nodes.size();
	std::vector<Node> mid_nodes(num_nodes);
	for (size_t i = 0; i < num_nodes; i++)
	{
		tinygltf::Node& node_in = model.nodes[i];
		Node& node_out = mid_nodes[i];
		node_out.children = node_in.children;

		if (node_in.matrix.size() > 0)
		{
			glm::mat4 matrix;
			for (int c = 0; c < 16; c++)
			{
				matrix[c / 4][c % 4] = (float)node_in.matrix[c];
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
	}

	std::list<int> node_queue;
	size_t num_roots = scene.nodes.size();
	for (size_t i = 0; i < num_roots; i++)
	{
		int idx_root = scene.nodes[i];
		Node& node = mid_nodes[idx_root];
		node.g_trans = glm::identity<glm::mat4>();
		node_queue.push_back(idx_root);
	}

	while (!node_queue.empty())
	{
		int id_node = node_queue.front();
		node_queue.pop_front();
		Node& node = mid_nodes[id_node];

		glm::mat4 local = glm::identity<glm::mat4>();
		local = glm::translate(local, node.translation);
		local *= glm::toMat4(node.rotation);
		local = glm::scale(local, node.scale);
		node.g_trans *= local;

		for (size_t i = 0; i < node.children.size(); i++)
		{
			int id_child = node.children[i];
			Node& child = mid_nodes[id_child];
			child.g_trans = node.g_trans;
			node_queue.push_back(id_child);
		}
	}

	size_t num_meshes = model.meshes.size();
	meshes.resize(num_meshes);

	for (size_t i = 0; i < num_nodes; i++)
	{
		tinygltf::Node& node_in = model.nodes[i];
		Node& node_out = mid_nodes[i];
		if (node_in.mesh >= 0 && node_in.skin < 0)
		{
			Mesh& mesh_out = meshes[node_in.mesh];
			mesh_out.g_trans = node_out.g_trans;
		}		
	}

	for (size_t i = 0; i < num_meshes; i++)
	{
		tinygltf::Mesh& mesh_in = model.meshes[i];
		Mesh& mesh_out = meshes[i];

		size_t num_primitives = mesh_in.primitives.size();
		mesh_out.primitives.resize(num_primitives);

		for (size_t j = 0; j < num_primitives; j++)
		{
			tinygltf::Primitive& primitive_in = mesh_in.primitives[j];
			Primitive& primitive_out = mesh_out.primitives[j];
			primitive_out.material = primitive_in.material;

			int id_pos_in = primitive_in.attributes["POSITION"];
			tinygltf::Accessor& acc_pos_in = model.accessors[id_pos_in];
			int num_pos = (int)acc_pos_in.count;
			tinygltf::BufferView& view_pos_in = model.bufferViews[acc_pos_in.bufferView];
			const glm::vec3* p_pos = (const glm::vec3*)(model.buffers[view_pos_in.buffer].data.data() + view_pos_in.byteOffset + acc_pos_in.byteOffset);
			primitive_out.positions.resize((size_t)num_pos);
			memcpy(primitive_out.positions.data(), p_pos, sizeof(glm::vec3)* (size_t)num_pos);

			int id_indices_in = primitive_in.indices;			
			if (id_indices_in >= 0)
			{
				tinygltf::Accessor& acc_indices_in = model.accessors[id_indices_in];
				int num_face = (int)(acc_indices_in.count / 3);
				tinygltf::BufferView& view_indices_in = model.bufferViews[acc_indices_in.bufferView];				
				primitive_out.indices.resize((size_t)num_face);

				if (acc_indices_in.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
				{
					const glm::u8vec3* p_indices = (const glm::u8vec3*)(model.buffers[view_indices_in.buffer].data.data() + view_indices_in.byteOffset + acc_indices_in.byteOffset);
					for (int i = 0; i < num_face; i++)
					{
						primitive_out.indices[i] = glm::ivec3(p_indices[i]);
					}					
				}
				else if (acc_indices_in.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
				{
					const glm::u16vec3* p_indices = (const glm::u16vec3*)(model.buffers[view_indices_in.buffer].data.data() + view_indices_in.byteOffset + acc_indices_in.byteOffset);
					for (int i = 0; i < num_face; i++)
					{
						primitive_out.indices[i] = glm::ivec3(p_indices[i]);
					}
				}
				else if (acc_indices_in.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
				{
					const glm::u32vec3* p_indices = (const glm::u32vec3*)(model.buffers[view_indices_in.buffer].data.data() + view_indices_in.byteOffset + acc_indices_in.byteOffset);
					memcpy(primitive_out.indices.data(), p_indices, sizeof(glm::ivec3)* (size_t)num_face);
				}			
			}
			else
			{
				primitive_out.indices.resize((size_t)num_pos/3);
				int* p_ind = (int*)primitive_out.indices.data();
				for (int i = 0; i < num_pos; i++)
				{
					p_ind[i] = i;
				}
			}				

			if (primitive_in.attributes.find("NORMAL") != primitive_in.attributes.end())
			{
				int id_norm_in = primitive_in.attributes["NORMAL"];
				tinygltf::Accessor& acc_norm_in = model.accessors[id_norm_in];
				tinygltf::BufferView& view_norm_in = model.bufferViews[acc_norm_in.bufferView];
				const glm::vec3* p_norm = (const glm::vec3*)(model.buffers[view_norm_in.buffer].data.data() + view_norm_in.byteOffset + acc_norm_in.byteOffset);
				primitive_out.normals.resize((size_t)num_pos);
				memcpy(primitive_out.normals.data(), p_norm, sizeof(glm::vec3) * (size_t)num_pos);
			}

			if (primitive_in.attributes.find("COLOR_0") != primitive_in.attributes.end())
			{
				int id_color_in = primitive_in.attributes["COLOR_0"];
				tinygltf::Accessor& acc_color_in = model.accessors[id_color_in];
				tinygltf::BufferView& view_color_in = model.bufferViews[acc_color_in.bufferView];			

				primitive_out.colors.resize((size_t)num_pos);

				if (acc_color_in.type == TINYGLTF_TYPE_VEC4)
				{
					if (acc_color_in.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
					{
						const glm::vec4* p_norm = (const glm::vec4*)(model.buffers[view_color_in.buffer].data.data() + view_color_in.byteOffset + acc_color_in.byteOffset);
						for (int i = 0; i < num_pos; i++)
						{
							primitive_out.colors[i] = glm::vec3(p_norm[i]);
						}
						
					}
					else if (acc_color_in.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
					{
						const glm::u16vec4* p_norm = (const glm::u16vec4*)(model.buffers[view_color_in.buffer].data.data() + view_color_in.byteOffset + acc_color_in.byteOffset);
						for (int i = 0; i < num_pos; i++)
						{
							primitive_out.colors[i] = glm::vec3(p_norm[i]) / 65535.0f;
						}
					}
					else if (acc_color_in.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
					{
						const glm::u8vec4* p_norm = (const glm::u8vec4*)(model.buffers[view_color_in.buffer].data.data() + view_color_in.byteOffset + acc_color_in.byteOffset);
						for (int i = 0; i < num_pos; i++)
						{
							primitive_out.colors[i] = glm::vec3(p_norm[i]) / 255.0f;
						}
					}
				}
				else if (acc_color_in.type == TINYGLTF_TYPE_VEC3)
				{
					if (acc_color_in.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
					{
						const glm::vec3* p_norm = (const glm::vec3*)(model.buffers[view_color_in.buffer].data.data() + view_color_in.byteOffset + acc_color_in.byteOffset);
						for (int i = 0; i < num_pos; i++)
						{
							primitive_out.colors[i] = p_norm[i];
						}
					}
					else if (acc_color_in.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
					{
						const glm::u16vec3* p_norm = (const glm::u16vec3*)(model.buffers[view_color_in.buffer].data.data() + view_color_in.byteOffset + acc_color_in.byteOffset);
						for (int i = 0; i < num_pos; i++)
						{
							primitive_out.colors[i] = glm::vec3(p_norm[i]) / 65535.0f;
						}
					}
					else if (acc_color_in.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
					{
						const glm::u8vec3* p_norm = (const glm::u8vec3*)(model.buffers[view_color_in.buffer].data.data() + view_color_in.byteOffset + acc_color_in.byteOffset);
						for (int i = 0; i < num_pos; i++)
						{
							primitive_out.colors[i] = glm::vec3(p_norm[i]) / 255.0f;
						}
					}
				}
			}

			if (primitive_in.attributes.find("TEXCOORD_0") != primitive_in.attributes.end())
			{
				int id_uv_in = primitive_in.attributes["TEXCOORD_0"];
				tinygltf::Accessor& acc_uv_in = model.accessors[id_uv_in];
				tinygltf::BufferView& view_uv_in = model.bufferViews[acc_uv_in.bufferView];
				const glm::vec2* p_uv = (const glm::vec2*)(model.buffers[view_uv_in.buffer].data.data() + view_uv_in.byteOffset + acc_uv_in.byteOffset);
				primitive_out.texcoords.resize((size_t)num_pos);
				memcpy(primitive_out.texcoords.data(), p_uv, sizeof(glm::vec2) * (size_t)num_pos);
			}

			if (primitive_in.attributes.find("TEXCOORD_1") != primitive_in.attributes.end())
			{
				int id_uv_in = primitive_in.attributes["TEXCOORD_1"];
				tinygltf::Accessor& acc_uv_in = model.accessors[id_uv_in];
				tinygltf::BufferView& view_uv_in = model.bufferViews[acc_uv_in.bufferView];
				const glm::vec2* p_uv = (const glm::vec2*)(model.buffers[view_uv_in.buffer].data.data() + view_uv_in.byteOffset + acc_uv_in.byteOffset);
				primitive_out.texcoords1.resize((size_t)num_pos);
				memcpy(primitive_out.texcoords1.data(), p_uv, sizeof(glm::vec2) * (size_t)num_pos);
			}
		}
	}
	
	for (size_t i = 0; i < num_nodes; i++)
	{
		tinygltf::Node& node_in = model.nodes[i];		
		int j = node_in.mesh;

		if (j >= 0)
		{
			Mesh& mesh_out = meshes[j];

			std::string name = node_in.name;
			if (name == "")
			{
				char mesh_name[32];
				sprintf(mesh_name, "mesh_%d", j);
				name = mesh_name;
			}
			mesh_out.name = name;
		}
	}

	extensionsUsed = model.extensionsUsed;
	extensionsRequired = model.extensionsRequired;
}

void DataModel::SaveGlb(const char* fn_glb)
{
	tinygltf::Model m_out;
	
	m_out.asset.version = "2.0";
	m_out.asset.generator = "tinygltf";

	m_out.buffers.resize(1);
	tinygltf::Buffer& buf_out = m_out.buffers[0];

	size_t offset = 0;
	size_t length = 0;
	size_t view_id = 0;
	size_t acc_id = 0;

	// sampler
	m_out.samplers.resize(1);
	tinygltf::Sampler& sampler = m_out.samplers[0];
	sampler.minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
	sampler.magFilter = TINYGLTF_TEXTURE_FILTER_LINEAR;

	// texture
	int num_textures = (int)textures.size();
	m_out.images.resize(num_textures);
	m_out.textures.resize(num_textures);
	for (int i = 0; i < num_textures; i++)
	{
		Image& tex_in = textures[i];
		tinygltf::Image& img_out = m_out.images[i];
		tinygltf::Texture& tex_out = m_out.textures[i];
		img_out.name = tex_in.name;
		img_out.width = tex_in.width;
		img_out.height = tex_in.height;
		img_out.component = 4;
		img_out.bits = 8;
		img_out.pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;

		if (tex_in.mimeType == "image/png")
		{
			img_out.mimeType = "image/png";

			std::vector<unsigned char>& png_buf = tex_in.storage;
			if (png_buf.size() == 0)
			{
				stbi_write_png_to_func([](void* context, void* data, int size)
					{
						std::vector<unsigned char>* buf = (std::vector<unsigned char>*)context;
				size_t offset = buf->size();
				buf->resize(offset + size);
				memcpy(buf->data() + offset, data, size);
					}, &png_buf, img_out.width, img_out.height, 4, tex_in.data.data(), img_out.width * 4);
			}

			offset = buf_out.data.size();
			length = png_buf.size();
			buf_out.data.resize((offset + length + 3) / 4 * 4);
			memcpy(buf_out.data.data() + offset, png_buf.data(), length);
		}
		else if (tex_in.mimeType == "image/jpeg")
		{
			img_out.mimeType = "image/jpeg";

			std::vector<unsigned char>& jpg_buf = tex_in.storage;
			if (jpg_buf.size() == 0)
			{
				stbi_write_jpg_to_func([](void* context, void* data, int size)
					{
						std::vector<unsigned char>* buf = (std::vector<unsigned char>*)context;
				size_t offset = buf->size();
				buf->resize(offset + size);
				memcpy(buf->data() + offset, data, size);
					}, &jpg_buf, img_out.width, img_out.height, 4, tex_in.data.data(), 80);
			}
			offset = buf_out.data.size();
			length = jpg_buf.size();
			buf_out.data.resize((offset + length + 3) / 4 * 4);
			memcpy(buf_out.data.data() + offset, jpg_buf.data(), length);
		}

		view_id = m_out.bufferViews.size();
		{
			tinygltf::BufferView view;
			view.buffer = 0;
			view.byteOffset = offset;
			view.byteLength = length;
			m_out.bufferViews.push_back(view);
		}
		img_out.bufferView = view_id;

		tex_out.name = tex_in.name;
		tex_out.sampler = 0;
		tex_out.source = i;
	}

	m_out.materials = materials;
	m_out.nodes = nodes;
	m_out.scenes.push_back(scene);


	// meshes
	m_out.meshes.resize(meshes.size());
	for (size_t i = 0; i < meshes.size(); i++)
	{
		Mesh& mesh_in = meshes[i];
		tinygltf::Mesh& mesh_out = m_out.meshes[i];
		int num_primitives = (int)mesh_in.primitives.size();		
		mesh_out.name = mesh_in.name;
		mesh_out.primitives.resize(num_primitives);

		for (int j = 0; j < num_primitives; j++)
		{
			Primitive& prim_in = mesh_in.primitives[j];
			tinygltf::Primitive& prim_out = mesh_out.primitives[j];
			prim_out.material = prim_in.material;
			prim_out.mode = TINYGLTF_MODE_TRIANGLES;

			int num_pos = (int)prim_in.positions.size();
			int num_face = (int)prim_in.indices.size();

			offset = buf_out.data.size();
			length = sizeof(glm::ivec3) * num_face;
			buf_out.data.resize(offset + length);
			memcpy(buf_out.data.data() + offset, prim_in.indices.data(), length);

			view_id = m_out.bufferViews.size();
			{
				tinygltf::BufferView view;
				view.buffer = 0;
				view.byteOffset = offset;
				view.byteLength = length;
				view.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
				m_out.bufferViews.push_back(view);
			}

			acc_id = m_out.accessors.size();
			{
				tinygltf::Accessor acc;
				acc.bufferView = view_id;
				acc.byteOffset = 0;
				acc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
				acc.count = (size_t)(num_face * 3);
				acc.type = TINYGLTF_TYPE_SCALAR;
				acc.maxValues = { (double)(num_pos - 1) };
				acc.minValues = { 0 };
				m_out.accessors.push_back(acc);
			}

			prim_out.indices = acc_id;

			glm::vec3 min_pos = { FLT_MAX, FLT_MAX, FLT_MAX };
			glm::vec3 max_pos = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

			for (int k = 0; k < num_pos; k++)
			{
				glm::vec3 pos = prim_in.positions[k];
				if (pos.x < min_pos.x) min_pos.x = pos.x;
				if (pos.x > max_pos.x) max_pos.x = pos.x;
				if (pos.y < min_pos.y) min_pos.y = pos.y;
				if (pos.y > max_pos.y) max_pos.y = pos.y;
				if (pos.z < min_pos.z) min_pos.z = pos.z;
				if (pos.z > max_pos.z) max_pos.z = pos.z;
			}


			offset = buf_out.data.size();
			length = sizeof(glm::vec3) * num_pos;
			buf_out.data.resize(offset + length);
			memcpy(buf_out.data.data() + offset, prim_in.positions.data(), length);

			view_id = m_out.bufferViews.size();
			{
				tinygltf::BufferView view;
				view.buffer = 0;
				view.byteOffset = offset;
				view.byteLength = length;
				view.target = TINYGLTF_TARGET_ARRAY_BUFFER;
				m_out.bufferViews.push_back(view);
			}

			acc_id = m_out.accessors.size();
			{
				tinygltf::Accessor acc;
				acc.bufferView = view_id;
				acc.byteOffset = 0;
				acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
				acc.count = (size_t)(num_pos);
				acc.type = TINYGLTF_TYPE_VEC3;
				acc.maxValues = { max_pos.x, max_pos.y, max_pos.z };
				acc.minValues = { min_pos.x, min_pos.y, min_pos.z };
				m_out.accessors.push_back(acc);
			}

			prim_out.attributes["POSITION"] = acc_id;

			if (prim_in.normals.size() > 0)
			{
				offset = buf_out.data.size();
				length = sizeof(glm::vec3) * num_pos;
				buf_out.data.resize(offset + length);
				memcpy(buf_out.data.data() + offset, prim_in.normals.data(), length);

				view_id = m_out.bufferViews.size();
				{
					tinygltf::BufferView view;
					view.buffer = 0;
					view.byteOffset = offset;
					view.byteLength = length;
					view.target = TINYGLTF_TARGET_ARRAY_BUFFER;
					m_out.bufferViews.push_back(view);
				}

				acc_id = m_out.accessors.size();
				{
					tinygltf::Accessor acc;
					acc.bufferView = view_id;
					acc.byteOffset = 0;
					acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
					acc.count = (size_t)(num_pos);
					acc.type = TINYGLTF_TYPE_VEC3;
					m_out.accessors.push_back(acc);
				}

				prim_out.attributes["NORMAL"] = acc_id;
			}

			if (prim_in.colors.size() > 0)
			{
				offset = buf_out.data.size();
				length = sizeof(glm::vec3) * num_pos;
				buf_out.data.resize(offset + length);
				memcpy(buf_out.data.data() + offset, prim_in.colors.data(), length);

				view_id = m_out.bufferViews.size();
				{
					tinygltf::BufferView view;
					view.buffer = 0;
					view.byteOffset = offset;
					view.byteLength = length;
					view.target = TINYGLTF_TARGET_ARRAY_BUFFER;
					m_out.bufferViews.push_back(view);
				}

				acc_id = m_out.accessors.size();
				{
					tinygltf::Accessor acc;
					acc.bufferView = view_id;
					acc.byteOffset = 0;
					acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
					acc.count = (size_t)(num_pos);
					acc.type = TINYGLTF_TYPE_VEC3;
					m_out.accessors.push_back(acc);
				}

				prim_out.attributes["COLOR_0"] = acc_id;

			}

			if (prim_in.texcoords.size() > 0)
			{
				offset = buf_out.data.size();
				length = sizeof(glm::vec2) * num_pos;
				buf_out.data.resize(offset + length);
				memcpy(buf_out.data.data() + offset, prim_in.texcoords.data(), length);

				view_id = m_out.bufferViews.size();
				{
					tinygltf::BufferView view;
					view.buffer = 0;
					view.byteOffset = offset;
					view.byteLength = length;
					view.target = TINYGLTF_TARGET_ARRAY_BUFFER;
					m_out.bufferViews.push_back(view);
				}

				acc_id = m_out.accessors.size();
				{
					tinygltf::Accessor acc;
					acc.bufferView = view_id;
					acc.byteOffset = 0;
					acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
					acc.count = (size_t)(num_pos);
					acc.type = TINYGLTF_TYPE_VEC2;
					m_out.accessors.push_back(acc);
				}

				prim_out.attributes["TEXCOORD_0"] = acc_id;
			}

			if (prim_in.texcoords1.size() > 0)
			{
				offset = buf_out.data.size();
				length = sizeof(glm::vec2) * num_pos;
				buf_out.data.resize(offset + length);
				memcpy(buf_out.data.data() + offset, prim_in.texcoords1.data(), length);

				view_id = m_out.bufferViews.size();
				{
					tinygltf::BufferView view;
					view.buffer = 0;
					view.byteOffset = offset;
					view.byteLength = length;
					view.target = TINYGLTF_TARGET_ARRAY_BUFFER;
					m_out.bufferViews.push_back(view);
				}

				acc_id = m_out.accessors.size();
				{
					tinygltf::Accessor acc;
					acc.bufferView = view_id;
					acc.byteOffset = 0;
					acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
					acc.count = (size_t)(num_pos);
					acc.type = TINYGLTF_TYPE_VEC2;
					m_out.accessors.push_back(acc);
				}

				prim_out.attributes["TEXCOORD_1"] = acc_id;
			}
		}
	}

	m_out.extensionsUsed = extensionsUsed;
	m_out.extensionsRequired = extensionsRequired;

	tinygltf::TinyGLTF gltf;
	gltf.WriteGltfSceneToFile(&m_out, fn_glb, true, true, false, true);
}

void DataModel::CreateModel(GLTFModel* model_out)
{
	struct TexLoadOptions
	{
		bool is_srgb = true;
		bool reverse = false;
	};
	size_t num_textures = textures.size();
	model_out->m_textures.resize(num_textures);
	std::vector<TexLoadOptions> tex_opts(num_textures);

	size_t num_materials = materials.size();
	model_out->m_materials.resize(num_materials + 1);
	for (size_t i = 0; i < num_materials; i++)
	{
		tinygltf::Material& material_in = materials[i];
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

		if (material_in.normalTexture.index >= 0)
		{
			tex_opts[material_in.normalTexture.index].is_srgb = false;
			material_out->tex_idx_normalMap = material_in.normalTexture.index;
			float scale = (float)material_in.normalTexture.scale;
			material_out->normalScale = { scale, scale };
		}

		material_out->emissive = { material_in.emissiveFactor[0], material_in.emissiveFactor[1], material_in.emissiveFactor[2] };

		if (material_in.extensions.find("KHR_materials_emissive_strength") != material_in.extensions.end())
		{
			tinygltf::Value::Object& emissive_stength = material_in.extensions["KHR_materials_emissive_strength"].Get<tinygltf::Value::Object>();
			float strength = (float)emissive_stength["emissiveStrength"].Get<double>();
			material_out->emissive *= strength;
		}

		material_out->tex_idx_emissiveMap = material_in.emissiveTexture.index;

		material_out->metallicFactor = pbr.metallicFactor;
		material_out->roughnessFactor = pbr.roughnessFactor;

		int id_mr = pbr.metallicRoughnessTexture.index;
		if (id_mr >= 0)
		{
			tex_opts[id_mr].is_srgb = false;
			material_out->tex_idx_metalnessMap = id_mr;
			material_out->tex_idx_roughnessMap = id_mr;
		}

		if (material_in.extensions.find("KHR_materials_pbrSpecularGlossiness") != material_in.extensions.end())
		{
			material_out->specular_glossiness = true;
			tinygltf::Value::Object& sg = material_in.extensions["KHR_materials_pbrSpecularGlossiness"].Get<tinygltf::Value::Object>();

			if (sg.find("diffuseFactor") != sg.end())
			{
				tinygltf::Value& color = sg["diffuseFactor"];
				float r = (float)color.Get(0).Get<double>();
				float g = (float)color.Get(1).Get<double>();
				float b = (float)color.Get(2).Get<double>();
				float a = (float)color.Get(3).Get<double>();
				material_out->color = { r,g,b,a };
			}

			if (sg.find("diffuseTexture") != sg.end())
			{
				tinygltf::Value::Object& tex = sg["diffuseTexture"].Get<tinygltf::Value::Object>();
				int idx = tex["index"].Get<int>();
				material_out->tex_idx_map = idx;
			}

			if (sg.find("glossinessFactor") != sg.end())
			{
				float v = (float)sg["glossinessFactor"].Get<double>();
				material_out->glossinessFactor = v;
			}

			if (sg.find("specularFactor") != sg.end())
			{
				tinygltf::Value& color = sg["specularFactor"];
				float r = (float)color.Get(0).Get<double>();
				float g = (float)color.Get(1).Get<double>();
				float b = (float)color.Get(2).Get<double>();
				material_out->specular = { r,g,b };
			}

			if (sg.find("specularGlossinessTexture") != sg.end())
			{
				tinygltf::Value::Object& tex = sg["specularGlossinessTexture"].Get<tinygltf::Value::Object>();
				int idx = tex["index"].Get<int>();
				material_out->tex_idx_specularMap = idx;
				material_out->tex_idx_glossinessMap = idx;
			}
		}

		material_out->update_uniform();
	}

	// default material
	{
		MeshStandardMaterial* material_out = new MeshStandardMaterial();
		model_out->m_materials[num_materials] = std::unique_ptr<MeshStandardMaterial>(material_out);
		material_out->update_uniform();
	}

	for (size_t i = 0; i < num_textures; i++)
	{
		Image& tex_in = textures[i];
		GLTexture2D* tex_out = new GLTexture2D();
		model_out->m_textures[i] = std::unique_ptr<GLTexture2D>(tex_out);
		model_out->m_tex_dict[tex_in.name] = i;

		tex_out->load_memory_rgba(tex_in.width, tex_in.height, tex_in.data.data(), tex_opts[i].is_srgb);
	}

	size_t num_meshes = meshes.size();
	model_out->m_meshs.resize(num_meshes);
	for (size_t i = 0; i < num_meshes; i++)
	{
		Mesh& mesh_in = meshes[i];
		::Mesh& mesh_out = model_out->m_meshs[i];
		
		size_t num_primitives = mesh_in.primitives.size();
		mesh_out.primitives.resize(num_primitives);

		for (size_t j = 0; j < num_primitives; j++)
		{
			Primitive& primitive_in = mesh_in.primitives[j];
			::Primitive& primitive_out = mesh_out.primitives[j];
			primitive_out.material_idx = primitive_in.material;

			MeshStandardMaterial* material = model_out->m_materials[primitive_out.material_idx].get();
			bool has_tangent = material->tex_idx_normalMap >= 0;

			int num_geo_sets = 1;			
			primitive_out.geometry.resize(num_geo_sets);

			int num_pos = (int)primitive_in.positions.size();
			primitive_out.num_pos = num_pos;

			glm::vec3 min_pos = { FLT_MAX, FLT_MAX, FLT_MAX };
			glm::vec3 max_pos = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

			for (int k = 0; k < num_pos; k++)
			{
				glm::vec3 pos = primitive_in.positions[k];
				if (pos.x < min_pos.x) min_pos.x = pos.x;
				if (pos.x > max_pos.x) max_pos.x = pos.x;
				if (pos.y < min_pos.y) min_pos.y = pos.y;
				if (pos.y > max_pos.y) max_pos.y = pos.y;
				if (pos.z < min_pos.z) min_pos.z = pos.z;
				if (pos.z > max_pos.z) max_pos.z = pos.z;
			}

			primitive_out.min_pos = min_pos;
			primitive_out.max_pos = max_pos;

			int num_face = (int)primitive_in.indices.size();
			primitive_out.num_face = num_face;

			primitive_out.type_indices = 4;
			primitive_out.index_buf = Index(new IndexTextureBuffer((size_t)primitive_out.type_indices * (size_t)num_face * 3, primitive_out.type_indices));
			primitive_out.index_buf->upload(primitive_in.indices.data());

			primitive_out.cpu_indices = std::unique_ptr<std::vector<uint8_t>>(new std::vector<uint8_t>(primitive_out.index_buf->m_size));
			memcpy(primitive_out.cpu_indices->data(), primitive_in.indices.data(), primitive_out.index_buf->m_size);

			GeometrySet& geometry = primitive_out.geometry[0];
			primitive_out.cpu_pos = std::unique_ptr<std::vector<glm::vec4>>(new std::vector<glm::vec4>(num_pos));

			for (int k = 0; k < num_pos; k++)
				(*primitive_out.cpu_pos)[k] = glm::vec4(primitive_in.positions[k], 1.0f);

			geometry.pos_buf = Attribute(new GLBuffer(sizeof(glm::vec4) * num_pos));
			geometry.pos_buf->upload(primitive_out.cpu_pos->data());

			std::vector<glm::vec4> norms(num_pos, glm::vec4(0.0f));

			geometry.normal_buf = Attribute(new GLBuffer(sizeof(glm::vec4) * num_pos));

			if (primitive_in.normals.size() > 0)
			{
				for (int k = 0; k < num_pos; k++)
					norms[k] = glm::vec4(primitive_in.normals[k], 0.0f);
			}
			else
			{
				calc_normal(num_face, num_pos, primitive_in.indices.data(), primitive_out.cpu_pos->data(), norms.data());
			}
			geometry.normal_buf->upload(norms.data());


			if (primitive_in.colors.size() > 0)
			{
				primitive_out.color_buf = Attribute(new GLBuffer(sizeof(glm::vec4) * primitive_out.num_pos));
				glm::vec3* p_in = (glm::vec3*)primitive_in.colors.data();
				std::vector<glm::vec4> tmp(primitive_out.num_pos);
				for (int k = 0; k < primitive_out.num_pos; k++)
				{
					tmp[k] = glm::vec4(p_in[k], 1.0f);
				}
				primitive_out.color_buf->upload(tmp.data());
			}

			if (primitive_in.texcoords.size() > 0)
			{
				primitive_out.uv_buf = Attribute(new GLBuffer(sizeof(glm::vec2) * num_pos));
				primitive_out.uv_buf->upload(primitive_in.texcoords.data());
			}

			std::vector<glm::vec4> tangent;
			std::vector<glm::vec4> bitangent;
			if (has_tangent)
			{
				tangent.resize(primitive_out.num_pos, glm::vec4(0.0f));
				bitangent.resize(primitive_out.num_pos, glm::vec4(0.0f));
				calc_tangent(primitive_out.num_face, primitive_out.num_pos, primitive_in.indices.data(), primitive_out.cpu_pos->data(), primitive_in.texcoords.data(), tangent.data(), bitangent.data());

				geometry.tangent_buf = Attribute(new GLBuffer(sizeof(glm::vec4) * primitive_out.num_pos));
				geometry.bitangent_buf = Attribute(new GLBuffer(sizeof(glm::vec4) * primitive_out.num_pos));
				geometry.tangent_buf->upload(tangent.data());
				geometry.bitangent_buf->upload(bitangent.data());
			}

			if (primitive_in.texcoords1.size() > 0)
			{
				primitive_out.lightmap_uv_buf = Attribute(new GLBuffer(sizeof(glm::vec2) * num_pos));
				primitive_out.lightmap_uv_buf->upload(primitive_in.texcoords1.data());

			}
		}
	}

	size_t num_nodes = nodes.size();
	model_out->m_nodes.resize(num_nodes);
	for (size_t i = 0; i < num_nodes; i++)
	{
		tinygltf::Node& node_in = nodes[i];
		Node& node_out = model_out->m_nodes[i];
		node_out.children = node_in.children;

		if (node_in.matrix.size() > 0)
		{
			glm::mat4 matrix;
			for (int c = 0; c < 16; c++)
			{
				matrix[c / 4][c % 4] = (float)node_in.matrix[c];
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
	model_out->m_roots = scene.nodes;

	for (size_t i = 0; i < num_nodes; i++)
	{
		tinygltf::Node& node_in = nodes[i];		
		int j = node_in.mesh;

		if (j >= 0)
		{
			::Mesh& mesh_out = model_out->m_meshs[j];
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

	if (lightmap_width > 0 && lightmap_height > 0)
	{
		model_out->lightmap = std::unique_ptr<Lightmap>(new Lightmap(lightmap_width, lightmap_height, lightmap_texels_per_unit));
	}
}

void DataModel::CreateAtlas(int texelsPerUnit)
{
	std::vector<Primitive*> primitives;
	std::vector<glm::mat4> trans;

	size_t num_meshes = meshes.size();
	for (size_t i = 0; i < num_meshes; i++)
	{
		Mesh& mesh = meshes[i];
		size_t num_prims = mesh.primitives.size();
		for (size_t j = 0; j < num_prims; j++)
		{
			Primitive& prim = mesh.primitives[j];
			glm::mat4 model_mat = mesh.g_trans;
			primitives.push_back(&prim);
			trans.push_back(model_mat);
		}
	}

	int num_prims = (int)primitives.size();
	xatlas::Atlas* atlas = xatlas::Create();

	for (int i = 0; i < num_prims; i++)
	{
		glm::mat4 model_mat = trans[i];
		glm::mat4 norm_mat = glm::transpose(glm::inverse(model_mat));

		Primitive* prim = primitives[i];
		xatlas::MeshDecl meshDecl;
		meshDecl.vertexCount = (unsigned)prim->positions.size();
		meshDecl.vertexPositionData = prim->positions.data();
		meshDecl.vertexPositionStride = sizeof(float) * 3;
		if (prim->normals.size() > 0)
		{
			meshDecl.vertexNormalData = prim->normals.data();
			meshDecl.vertexNormalStride = sizeof(float) * 3;
		}
		if (prim->texcoords.size() > 0)
		{
			meshDecl.vertexUvData = prim->texcoords.data();
			meshDecl.vertexUvStride = sizeof(float) * 2;
		}
		meshDecl.indexCount = (uint32_t)(prim->indices.size() * 3);
		meshDecl.indexData = prim->indices.data();
		meshDecl.indexFormat = xatlas::IndexFormat::UInt32;

		xatlas::AddMeshError error = xatlas::AddMesh(atlas, meshDecl, 1);
		if (error != xatlas::AddMeshError::Success) {
			xatlas::Destroy(atlas);
			printf("\rError adding mesh: %s\n", xatlas::StringForEnum(error));
		}
	}

	xatlas::ChartOptions chartOptions;
	xatlas::PackOptions packOptions;
	packOptions.padding = 1;
	packOptions.texelsPerUnit = float(texelsPerUnit);

	printf("Generating atlas...\n");
	xatlas::Generate(atlas, chartOptions, packOptions);
	printf("Done.\n");

	lightmap_width = atlas->width;
	lightmap_height = atlas->height;
	lightmap_texels_per_unit = texelsPerUnit;

	tinygltf::Value::Object extras;
	extras["lightmap_width"] = tinygltf::Value(lightmap_width);
	extras["lightmap_height"] = tinygltf::Value(lightmap_height);
	extras["lightmap_texels_per_unit"] = tinygltf::Value(lightmap_texels_per_unit);
	scene.extras = tinygltf::Value(extras);	

	glm::vec2 img_size = glm::vec2(lightmap_width, lightmap_height);

	for (int i = 0; i < num_prims; i++)
	{
		Primitive* prim = primitives[i];
		const xatlas::Mesh& atlas_mesh = atlas->meshes[i];

		Primitive prim_new;
		prim_new.material = prim->material;
		prim_new.indices.resize(prim->indices.size());

		std::unordered_map<glm::ivec2, int> idx_map;
		int num_indices = (int)prim->indices.size() * 3;		
		const int* index_in = (const int*)prim->indices.data();
		int* index_out = (int*)prim_new.indices.data();		
		for (int j = 0; j < num_indices; j++)
		{
			int idx0 = index_in[j];
			int idx1 = (int)atlas_mesh.indexArray[j];
			glm::ivec2 idx = glm::ivec2(idx0, idx1);
			auto iter = idx_map.find(idx);
			if (iter != idx_map.end())
			{
				index_out[j] = iter->second;
			}
			else
			{
				int idx_new = (int)prim_new.positions.size();
				idx_map[idx] = idx_new;
				index_out[j] = idx_new;

				prim_new.positions.push_back(prim->positions[idx0]);
				if (prim->normals.size() > 0)
				{
					prim_new.normals.push_back(prim->normals[idx0]);
				}
				if (prim->colors.size() > 0)
				{
					prim_new.colors.push_back(prim->colors[idx0]);
				}
				if (prim->texcoords.size() > 0)
				{
					prim_new.texcoords.push_back(prim->texcoords[idx0]);
				}
				const float* p_uv = atlas_mesh.vertexArray[idx1].uv;
				prim_new.texcoords1.push_back((glm::vec2(p_uv[0], p_uv[1]) + 0.5f) / img_size);
			}
		}

		*prim = prim_new;
	}

	xatlas::Destroy(atlas);

}
