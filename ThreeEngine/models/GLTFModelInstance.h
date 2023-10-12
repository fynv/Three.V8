#pragma once

#include <memory>
#include <vector>
#include "core/Object3D.h"
#include "renderers/GLUtils.h"

class GLTFModel;
class GLTFModelInstance : public Object3D
{
public:
	GLTFModel* m_model;

	std::vector<std::unique_ptr<GLBuffer>> m_mesh_constants;
	
	struct NodeInfo
	{			
		glm::vec3 translation;
		glm::quat rotation;
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
		glm::mat4 g_trans;
	};

	std::vector<NodeInfo> m_node_info;

	GLTFModelInstance(GLTFModel* model);

	glm::vec3 m_min_pos = { FLT_MAX, FLT_MAX, FLT_MAX };
	glm::vec3 m_max_pos = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
	void calculate_bounding_box();

	void updateNodes();
	void updateMeshConstants();
};

