#include "GLTFModelInstance.h"
#include "GLTFModel.h"

struct ModelConst
{
	glm::mat4 ModelMat;
	glm::mat4 NormalMat;
};

GLTFModelInstance::GLTFModelInstance(GLTFModel* model)
	: m_model(model)
{
	m_mesh_constants.resize(model->m_meshs.size());
	for (size_t i = 0; i < model->m_meshs.size(); i++)
	{
		m_mesh_constants[i] = std::unique_ptr<GLBuffer>(new GLBuffer(sizeof(ModelConst)));
	}

	m_node_info.resize(model->m_nodes.size());
	for (size_t i = 0; i < model->m_nodes.size(); i++)
	{
		NodeInfo& info = m_node_info[i];
		const Node& node = model->m_nodes[i];
		info.translation = node.translation;
		info.rotation = node.rotation;
		info.scale = node.scale;
		info.g_trans = node.g_trans;
	}
	calculate_bounding_box();
}

void GLTFModelInstance::calculate_bounding_box()
{
	m_min_pos = { FLT_MAX, FLT_MAX, FLT_MAX };
	m_max_pos = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

	size_t num_meshes = m_model->m_meshs.size();
	for (size_t i = 0; i < num_meshes; i++)
	{
		Mesh& mesh = m_model->m_meshs[i];

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
			NodeInfo& info = m_node_info[mesh.node_id];
			glm::mat4 mesh_mat = info.g_trans;

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

void GLTFModelInstance::updateNodes()
{
	std::list<int> node_queue;
	size_t num_roots = m_model->m_roots.size();
	for (size_t i = 0; i < num_roots; i++)
	{
		int idx_root = m_model->m_roots[i];
		NodeInfo& info = m_node_info[idx_root];
		info.g_trans = glm::identity<glm::mat4>();
		node_queue.push_back(idx_root);
	}

	while (!node_queue.empty())
	{
		int id_node = node_queue.front();
		node_queue.pop_front();
		Node& node = m_model->m_nodes[id_node];
		NodeInfo& info = m_node_info[id_node];

		glm::mat4 local = glm::identity<glm::mat4>();
		local = glm::translate(local, info.translation);
		local *= glm::toMat4(info.rotation);
		local = glm::scale(local, info.scale);
		info.g_trans *= local;

		for (size_t i = 0; i < node.children.size(); i++)
		{
			int id_child = node.children[i];
			NodeInfo& info_child = m_node_info[id_child];
			info_child.g_trans = info.g_trans;
			node_queue.push_back(id_child);
		}
	}

}

void GLTFModelInstance::updateMeshConstants()
{
	size_t num_mesh = m_mesh_constants.size();
	for (size_t i = 0; i < num_mesh; i++)
	{
		Mesh& mesh = m_model->m_meshs[i];
		glm::mat4 matrix = matrixWorld;
		if (mesh.node_id >= 0 && mesh.skin_id < 0)
		{
			NodeInfo& info = m_node_info[mesh.node_id];
			matrix *= info.g_trans;
		}
		ModelConst c;
		c.ModelMat = matrix;
		c.NormalMat = glm::transpose(glm::inverse(matrix));
		m_mesh_constants[i]->upload(&c);
	}
}
