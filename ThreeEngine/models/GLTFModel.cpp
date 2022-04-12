#include <list>

#include "GLTFModel.h"
#include "utils/Utils.h"

struct ModelConst
{
	glm::mat4 ModelMat;
	glm::mat4 NormalMat;
};


void GLTFModel::updateMeshConstants()
{
	size_t num_mesh = m_meshs.size();
	for (size_t i = 0; i < num_mesh; i++)
	{
		Mesh& mesh = m_meshs[i];
		glm::mat4 matrix = matrixWorld;
		if (mesh.node_id >= 0 && mesh.skin_id < 0)
		{
			Node& node = m_nodes[mesh.node_id];
			matrix *= node.g_trans;
		}
		ModelConst c;
		c.ModelMat = matrix;
		c.NormalMat = glm::transpose(glm::inverse(matrix));
		mesh.model_constant->upload(&c);
	}
}

void GLTFModel::updateNodes()
{
	std::list<int> node_queue;
	size_t num_roots = m_roots.size();
	for (size_t i = 0; i < num_roots; i++)
	{
		int idx_root = m_roots[i];		
		Node& node = m_nodes[idx_root];
		node.g_trans = glm::identity<glm::mat4>();
		node_queue.push_back(idx_root);
	}

	while (!node_queue.empty())
	{
		int id_node = node_queue.front();
		node_queue.pop_front();
		Node& node = m_nodes[id_node];

		glm::mat4 local = glm::identity<glm::mat4>();
		local = glm::translate(local, node.translation);
		local *= glm::toMat4(node.rotation);
		local = glm::scale(local, node.scale);
		node.g_trans *= local;

		for (size_t i = 0; i < node.children.size(); i++)
		{
			int id_child = node.children[i];
			Node& child = m_nodes[id_child];
			child.g_trans = node.g_trans;
			node_queue.push_back(id_child);
		}
	}
}

void GLTFModel::setAnimationFrame(const AnimationFrame& frame)
{
	size_t num_morphs = frame.morphs.size();
	for (size_t i = 0; i < num_morphs; i++)
	{
		const MorphFrame& morph = frame.morphs[i];
		auto iter = m_mesh_dict.find(morph.name);
		if (iter != m_mesh_dict.end())
		{
			Mesh& mesh = m_meshs[iter->second];
			mesh.weights = morph.weights;
			mesh.needUpdateMorphTargets = true;
		}
	}
}

void GLTFModel::addAnimation(const AnimationClip& anim)
{
	m_animations.push_back(anim);
	m_animation_dict[anim.name] = (int)(m_animations.size() - 1);
}

void GLTFModel::playAnimation(const char* name)
{
	auto iter = m_animation_dict.find(name);
	if (iter != m_animation_dict.end())
	{
		int id_anim = iter->second;
		for (size_t i = 0; i < m_current_playing.size(); i++)
		{
			PlayBack& playback = m_current_playing[i];
			if (i == id_anim)
			{
				playback.time_start = time_sec();
				return;
			}
		}
		m_current_playing.push_back({ id_anim, time_sec() });
	}
}

void GLTFModel::stopAnimation(const char* name)
{
	auto iter = m_animation_dict.find(name);
	if (iter != m_animation_dict.end())
	{
		int id_anim = iter->second;
		for (size_t i = 0; i < m_current_playing.size(); i++)
		{
			PlayBack& playback = m_current_playing[i];
			if (i == id_anim)
			{
				m_current_playing.erase(m_current_playing.begin() + i);
				return;
			}
		}
	}
}

void GLTFModel::updateAnimation()
{
	double t = time_sec();
	for (size_t i = 0; i < m_current_playing.size(); i++)
	{
		PlayBack& playback = m_current_playing[i];
		AnimationClip& anim = m_animations[playback.id_anim];
		double duration = anim.end - anim.start;
		while (t - playback.time_start > duration)
		{
			playback.time_start += duration;
		}
		double pos = t - playback.time_start;
		double x = pos + anim.start;

		AnimationFrame frame;
		anim.get_frame(x, frame);
		this->setAnimationFrame(frame);
	}
}