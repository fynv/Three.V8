#include "AnimationMixer.h"
#include "utils/Utils.h"
#include <glm.hpp>
#include <gtx/quaternion.hpp>

template<class T>
inline T linear_interpolate(T y0, T y1, float t)
{
	return (1.0f - t) * y0 + t * y1;
}

void AnimationMixer::addAnimation(const AnimationClip& anim)
{
	m_animations.push_back(anim);
	m_animation_dict[anim.name] = (int)(m_animations.size() - 1);
}

void AnimationMixer::startAnimation(const char* name)
{

	auto iter = m_animation_dict.find(name);
	if (iter != m_animation_dict.end())
	{
		int id_anim = iter->second;
		for (size_t i = 0; i < m_current_playing.size(); i++)
		{
			PlayBack& playback = m_current_playing[i];
			if (playback.id_anim == id_anim)
			{
				playback.time_start = time_sec();
				return;
			}
		}
		float weight = m_current_playing.size() > 0 ? 0.0f : 1.0f;
		m_current_playing.push_back({ id_anim, time_sec(), weight });
	}
}

void AnimationMixer::stopAnimation(const char* name)
{
	auto iter = m_animation_dict.find(name);
	if (iter != m_animation_dict.end())
	{
		int id_anim = iter->second;
		for (size_t i = 0; i < m_current_playing.size(); i++)
		{
			PlayBack& playback = m_current_playing[i];
			if (playback.id_anim == id_anim)
			{
				m_current_playing.erase(m_current_playing.begin() + i);
				return;
			}
		}
	}
}


void AnimationMixer::getFrame(AnimationFrame& dst_frame)
{
	if (m_current_playing.size() < 1) return;
	
	double t = time_sec();
	float acc_weight = 0.0f;
	for (size_t i = 0; i < m_current_playing.size(); i++)
	{
		PlayBack& playback = m_current_playing[i];
		AnimationClip& anim = m_animations[playback.id_anim];
		double duration = anim.duration;
		double x = 0.0;
		if (duration > 0.0)
		{
			while (t - playback.time_start >= duration)
			{
				playback.time_start += duration;
			}
			x = t - playback.time_start;
		}	

		float weight = playback.weight;
		float fac = weight / (acc_weight + weight);

		AnimationFrame src_frame;
		anim.get_frame(x, src_frame);

		size_t num_morphs = src_frame.morphs.size();
		for (size_t j = 0; j < num_morphs; j++)
		{
			MorphFrame& src_morph = src_frame.morphs[j];
			MorphFrame* dst_morph = nullptr;
			for (size_t k = 0; k < dst_frame.morphs.size(); k++)
			{
				if (dst_frame.morphs[k].name == src_morph.name)
				{
					dst_morph = &dst_frame.morphs[k];
					break;
				}
			}

			if (dst_morph == nullptr)
			{				
				dst_frame.morphs.push_back(src_morph);				
			}
			else
			{
				size_t num_elem_src = src_morph.weights.size();
				size_t num_elem_dst = dst_morph->weights.size();
				if (num_elem_dst < num_elem_src)
				{
					dst_morph->weights.resize(num_elem_src);
					memcpy(&dst_morph->weights[num_elem_dst], &src_morph.weights[num_elem_dst], sizeof(float) * (num_elem_src - num_elem_dst));					
				}

				for (size_t k = 0; k < num_elem_src; k++)
				{					
					dst_morph->weights[k] = linear_interpolate(dst_morph->weights[k], src_morph.weights[k], fac);
				}
			}
		}

		size_t num_trans = src_frame.translations.size();
		for (size_t j = 0; j < num_trans; j++)
		{
			TranslationFrame& src_trans = src_frame.translations[j];
			TranslationFrame* dst_trans = nullptr;
			for (size_t k = 0; k < dst_frame.translations.size(); k++)
			{
				if (dst_frame.translations[k].name == src_trans.name)
				{
					dst_trans = &dst_frame.translations[k];
					break;
				}
			}
			if (dst_trans == nullptr)
			{
				dst_frame.translations.push_back(src_trans);				
			}
			else
			{
				dst_trans->translation = linear_interpolate(dst_trans->translation, src_trans.translation, fac);
			}
		}

		size_t num_rots = src_frame.rotations.size();
		for (size_t j = 0; j < num_rots; j++)
		{
			RotationFrame& src_rot = src_frame.rotations[j];
			RotationFrame* dst_rot = nullptr;
			for (size_t k = 0; k < dst_frame.rotations.size(); k++)
			{
				if (dst_frame.rotations[k].name == src_rot.name)
				{
					dst_rot = &dst_frame.rotations[k];
					break;
				}
			}
			if (dst_rot == nullptr)
			{
				dst_frame.rotations.push_back(src_rot);
			}
			else
			{
				dst_rot->rotation = glm::slerp(dst_rot->rotation, src_rot.rotation, fac);
			}
		}

		size_t num_scales = src_frame.scales.size();
		for (size_t j = 0; j < num_scales; j++)
		{
			ScaleFrame& src_scale = src_frame.scales[j];
			ScaleFrame* dst_scale = nullptr;
			for (size_t k = 0; k < dst_frame.scales.size(); k++)
			{
				if (dst_frame.scales[k].name == src_scale.name)
				{
					dst_scale = &dst_frame.scales[k];
					break;
				}
			}
			if (dst_scale == nullptr)
			{
				dst_frame.scales.push_back(src_scale);
			}
			else
			{
				dst_scale->scale = linear_interpolate(dst_scale->scale, src_scale.scale, fac);
			}
		}

		acc_weight += weight;

	}
}