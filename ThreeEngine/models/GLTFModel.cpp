#include "GLTFModel.h"
#include "utils/Utils.h"

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