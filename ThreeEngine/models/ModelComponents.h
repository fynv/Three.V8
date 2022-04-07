#pragma once

#include <memory>
#include <vector>
#include <glm.hpp>
#include <gtx/quaternion.hpp>
#include "renderers/GLUtils.h"

typedef std::unique_ptr<GLBuffer> Attribute;

struct GeometrySet
{
	Attribute pos_buf;
	Attribute normal_buf;
};

class Primitive
{
public:
	bool has_blendshape = false;

	int num_pos = 0;	
	std::vector<GeometrySet> geometry;
	int type_color; // 3: rgb; 4: rgba
	Attribute color_buf;
	Attribute uv_buf;	

	int num_face = 0;
	int type_indices = 2; // 1:uchar; 2: ushort; 4: uint
	std::unique_ptr<GLBuffer> index_buf;
	std::vector<GeometrySet> targets;

	int material_idx = -1;

	// keep a cpu copy for ray-cast
	std::unique_ptr<std::vector<glm::vec3>> cpu_pos;
	std::unique_ptr<std::vector<uint8_t>> cpu_indices;
};

class Mesh
{
public:
	std::vector<Primitive> primitives;
	std::vector<float> weights;
};

class Node
{
public:
	int skin_idx = -1;
	int mesh_idx = -1;
	std::vector<int> children;
	glm::quat rotation;
	glm::vec3 scale;
	glm::vec3 translation;

	glm::mat4 matrix;


};
