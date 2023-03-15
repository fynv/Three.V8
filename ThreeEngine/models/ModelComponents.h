#pragma once

#include <memory>
#include <vector>
#include <glm.hpp>
#include <gtx/quaternion.hpp>
#include "renderers/GLUtils.h"
#include "core/CWBVH.h"

typedef std::unique_ptr<GLBuffer> Attribute;

class IndexTextureBuffer : public GLBuffer
{
public:
	int type_indices;
	unsigned tex_id;
	IndexTextureBuffer(size_t size, int type_indices);
	~IndexTextureBuffer();
};

typedef std::unique_ptr<IndexTextureBuffer> Index;

struct GeometrySet
{
	Attribute pos_buf;
	Attribute normal_buf;
	Attribute tangent_buf;
	Attribute bitangent_buf;
};

class Primitive
{
public:
	int num_pos = 0;	
	std::vector<GeometrySet> geometry;
	Attribute color_buf;
	Attribute uv_buf;
	Attribute joints_buf;
	Attribute weights_buf;

	int num_face = 0;
	int type_indices = 2; // 1:uchar; 2: ushort; 4: uint
	Index index_buf;

	int num_wires;
	Index wire_ind_buf;
	void compute_wires();

	int num_targets = 0;
	GeometrySet targets;
	Attribute none_zero_buf;

	int material_idx = -1;

	// keep a cpu copy for ray-cast
	glm::vec3 min_pos = { FLT_MAX, FLT_MAX, FLT_MAX };
	glm::vec3 max_pos = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
	std::unique_ptr<std::vector<glm::vec4>> cpu_pos;
	std::unique_ptr<std::vector<uint8_t>> cpu_indices;

	std::unique_ptr<CWBVH> cwbvh;
};

class Node
{
public:
	std::vector<int> children;
	glm::vec3 translation;
	glm::quat rotation;
	glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
	glm::mat4 g_trans;
};

class Skin
{
public:
	std::vector<int> joints;
	std::vector<glm::mat4> inverseBindMatrices;
	std::unique_ptr<GLDynBuffer> buf_rela_mat;
};

class Mesh
{
public:
	Mesh();
	int node_id = -1;
	int skin_id = -1;
	std::unique_ptr<GLDynBuffer> model_constant;
	std::vector<Primitive> primitives;
	std::vector<float> weights;
	std::unique_ptr<GLDynBuffer> buf_weights;
	bool needUpdateMorphTargets = false;

};
