#pragma once

#include <memory>
#include <vector>
#include <glm.hpp>
#include <gtx/quaternion.hpp>
#include "renderers/GLUtils.h"

typedef std::unique_ptr<GLBuffer> Attribute;
typedef std::unique_ptr<GLBuffer> Index;

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
	int type_color; // 3: rgb; 4: rgba
	Attribute color_buf;
	Attribute uv_buf;	

	int num_face = 0;
	int type_indices = 2; // 1:uchar; 2: ushort; 4: uint
	Index index_buf;

	int num_targets = 0;
	GeometrySet targets;

	int material_idx = -1;

	// keep a cpu copy for ray-cast
	std::unique_ptr<std::vector<glm::vec4>> cpu_pos;
	std::unique_ptr<std::vector<uint8_t>> cpu_indices;
};

class Mesh
{
public:
	std::vector<Primitive> primitives;
	std::vector<float> weights;
	std::unique_ptr<GLDynBuffer> buf_weights;
	bool needUpdateMorphTargets = false;
};
