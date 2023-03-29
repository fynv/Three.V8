#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_set>
#include <glm.hpp>
#include <gtc/quaternion.hpp>

#include <tiny_gltf.h>

class GLTFModel;
class DataModel
{
public:
	struct Image
	{
		std::string name;
		std::string mimeType;
		int width, height;
		std::vector<unsigned char> data;
		std::vector<unsigned char> storage;
	};

	std::vector<Image> textures;

	std::vector<tinygltf::Material> materials;
	std::vector<tinygltf::Node> nodes;
	tinygltf::Scene scene;

	struct Primitive
	{
		int material;
		std::vector<glm::ivec3> indices;
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> colors;
		std::vector<glm::vec2> texcoords;		

		std::vector<glm::vec2> texcoords1;
	};

	struct Mesh
	{
		glm::mat4 g_trans = glm::identity<glm::mat4>();
		std::string name;
		std::vector<Primitive> primitives;
	};

	std::vector<Mesh> meshes;

	std::vector<std::string> extensionsUsed;
	std::vector<std::string> extensionsRequired;

	void LoadGlb(const char* fn_glb);
	void SaveGlb(const char* fn_glb);
	void CreateModel(GLTFModel* model_out);

	int lightmap_width = -1;
	int lightmap_height = -1;
	float lightmap_texels_per_unit = 128.0f;
	void CreateAtlas(float texelsPerUnit = 128.0f);

};

