#include <algorithm>
#include "Animation.h"

template<class T>
inline T linear_interpolate(T y0, T y1, float t)
{
	return (1.0f - t)* y0 + t * y1;
}

template<class T>
inline T cubic_interpolate(T y0, T y1, T s0, T s1, float t, float t2, float t3, float td)
{
	return (2.0f * t3 - 3.0f * t2 + 1) * y0 + (-2.0f * t3 + 3.0f * t2) * y1	+ td * ((t3 - 2.0f * t2 + t) * s0 + (t3 - t2) * s1);
}

void MorphTrack::get_frame(float x, MorphFrame& frame)
{
	frame.name = name;
	frame.weights.resize(num_targets);
	auto iter = std::upper_bound(times.begin(), times.end(), x);	

	if (iter == times.begin())
	{
		if (interpolation == Interpolation::STEP || interpolation == Interpolation::LINEAR)
		{
			memcpy(frame.weights.data(), values.data(), sizeof(float) * num_targets);
		}
		else if (interpolation == Interpolation::CUBICSPLINE)
		{
			memcpy(frame.weights.data(), values.data() + num_targets, sizeof(float) * num_targets);
		}
	}
	else if (iter == times.end())
	{
		if (interpolation == Interpolation::STEP || interpolation == Interpolation::LINEAR)
		{
			memcpy(frame.weights.data(), values.data() + num_targets*(times.size()-1), sizeof(float) * num_targets);
		}
		else if (interpolation == Interpolation::CUBICSPLINE)
		{
			memcpy(frame.weights.data(), values.data() + num_targets + num_targets * (times.size() - 1)*3, sizeof(float) * num_targets);
		}
	}
	else
	{
		float x0 = *(iter - 1);
		float x1 = *iter;
		if (interpolation == Interpolation::STEP)
		{
			memcpy(frame.weights.data(), values.data() + num_targets*(iter- times.begin()-1), sizeof(float) * num_targets);
		}
		else if (interpolation == Interpolation::LINEAR)
		{
			const float* p0 = values.data() + num_targets * (iter - times.begin() - 1);
			const float* p1 = values.data() + num_targets * (iter - times.begin());
			float t = (x - x0) / (x1 - x0);
			for (int i = 0; i < num_targets; i++)
			{
				frame.weights[i] = linear_interpolate(p0[i], p1[i], t);
			}
		}
		else if (interpolation == Interpolation::CUBICSPLINE)
		{
			const float* p0 = values.data() + num_targets * (iter - times.begin() - 1) * 3;
			const float* p1 = values.data() + num_targets * (iter - times.begin()) * 3;
			float t = (x - x0) / (x1 - x0);
			float t2 = t * t;
			float t3 = t2 * t;
			float td = x1 - x0;
			for (int i = 0; i < num_targets; i++)
			{
				float y0 = p0[num_targets + i];
				float s0 = p0[num_targets*2 + i];
				float s1 = p1[i];
				float y1 = p1[num_targets + i];

				frame.weights[i] = cubic_interpolate(y0, y1, s0, s1, t, t2, t3, td);
			}
		}
	}
}

void TranslationTrack::get_frame(float x, TranslationFrame& frame)
{
	frame.name = name;	

	auto iter = std::upper_bound(times.begin(), times.end(), x);
	if (iter == times.begin())
	{
		if (interpolation == Interpolation::STEP || interpolation == Interpolation::LINEAR)
		{
			frame.translation = values[0];
		}
		else if (interpolation == Interpolation::CUBICSPLINE)
		{
			frame.translation = values[1];
		}
	}
	else if (iter == times.end())
	{
		if (interpolation == Interpolation::STEP || interpolation == Interpolation::LINEAR)
		{
			frame.translation = values[times.size() - 1];
		}
		else if (interpolation == Interpolation::CUBICSPLINE)
		{
			frame.translation = values[1 + (times.size() - 1) * 3];			
		}	
	}
	else
	{
		float x0 = *(iter - 1);
		float x1 = *iter;
		if (interpolation == Interpolation::STEP)
		{
			frame.translation = values[iter - times.begin() - 1];
		}
		else if (interpolation == Interpolation::LINEAR)
		{
			const glm::vec3& y0 = values[iter - times.begin() - 1];
			const glm::vec3& y1 = values[iter - times.begin()];
			float t = (x - x0) / (x1 - x0);
			frame.translation = linear_interpolate(y0, y1, t);
		}
		else if (interpolation == Interpolation::CUBICSPLINE)
		{
			float t = (x - x0) / (x1 - x0);
			float t2 = t * t;
			float t3 = t2 * t;
			float td = x1 - x0;

			const glm::vec3* p0 = values.data() + (iter - times.begin() - 1) * 3;
			const glm::vec3* p1 = values.data() + (iter - times.begin()) * 3;
			const glm::vec3& y0 = p0[1];
			const glm::vec3& s0 = p0[2];
			const glm::vec3& s1 = p1[0];
			const glm::vec3& y1 = p1[1];

			frame.translation = cubic_interpolate(y0, y1, s0, s1, t, t2, t3, td);
		}
	}
}

void RotationTrack::get_frame(float x, RotationFrame& frame)
{
	frame.name = name;

	auto iter = std::upper_bound(times.begin(), times.end(), x);
	if (iter == times.begin())
	{
		if (interpolation == Interpolation::STEP || interpolation == Interpolation::LINEAR)
		{
			frame.rotation = values[0];
		}
		else if (interpolation == Interpolation::CUBICSPLINE)
		{
			frame.rotation = values[1];
		}
	}
	else if (iter == times.end())
	{
		if (interpolation == Interpolation::STEP || interpolation == Interpolation::LINEAR)
		{
			frame.rotation = values[times.size() - 1];
		}
		else if (interpolation == Interpolation::CUBICSPLINE)
		{
			frame.rotation = values[1 + (times.size() - 1) * 3];
		}		
	}
	else
	{
		float x0 = *(iter - 1);
		float x1 = *iter;
		if (interpolation == Interpolation::STEP)
		{
			frame.rotation = values[iter - times.begin() - 1];
		}
		else if (interpolation == Interpolation::LINEAR)
		{
			const glm::quat& y0 = values[iter - times.begin() - 1];
			const glm::quat& y1 = values[iter - times.begin()];
			float t = (x - x0) / (x1 - x0);
			frame.rotation = glm::slerp(y0, y1, t);
		}
		else if (interpolation == Interpolation::CUBICSPLINE)
		{
			float t = (x - x0) / (x1 - x0);		
			const glm::quat* p0 = values.data() + (iter - times.begin() - 1) * 3;
			const glm::quat* p1 = values.data() + (iter - times.begin()) * 3;
			const glm::quat& y0 = p0[1];
			const glm::quat& y1 = p1[1];
			frame.rotation = glm::slerp(y0, y1, t);
		}
	}
}


void ScaleTrack::get_frame(float x, ScaleFrame& frame)
{
	frame.name = name;

	auto iter = std::upper_bound(times.begin(), times.end(), x);
	if (iter == times.begin())
	{
		if (interpolation == Interpolation::STEP || interpolation == Interpolation::LINEAR)
		{
			frame.scale = values[0];
		}
		else if (interpolation == Interpolation::CUBICSPLINE)
		{
			frame.scale = values[1];
		}
	}
	else if (iter == times.end())
	{
		if (interpolation == Interpolation::STEP || interpolation == Interpolation::LINEAR)
		{
			frame.scale = values[times.size() - 1];
		}
		else if (interpolation == Interpolation::CUBICSPLINE)
		{
			frame.scale = values[1 + (times.size() - 1) * 3];
		}		
	}
	else
	{
		float x0 = *(iter - 1);
		float x1 = *iter;
		if (interpolation == Interpolation::STEP)
		{
			frame.scale = values[iter - times.begin() - 1];
		}
		else if (interpolation == Interpolation::LINEAR)
		{
			const glm::vec3& y0 = values[iter - times.begin() - 1];
			const glm::vec3& y1 = values[iter - times.begin()];
			float t = (x - x0) / (x1 - x0);
			frame.scale = linear_interpolate(y0, y1, t);
		}
		else if (interpolation == Interpolation::CUBICSPLINE)
		{
			float t = (x - x0) / (x1 - x0);
			float t2 = t * t;
			float t3 = t2 * t;
			float td = x1 - x0;

			const glm::vec3* p0 = values.data() + (iter - times.begin() - 1) * 3;
			const glm::vec3* p1 = values.data() + (iter - times.begin()) * 3;
			const glm::vec3& y0 = p0[1];
			const glm::vec3& s0 = p0[2];
			const glm::vec3& s1 = p1[0];
			const glm::vec3& y1 = p1[1];

			frame.scale = cubic_interpolate(y0, y1, s0, s1, t, t2, t3, td);
		}
	}
}


void AnimationClip::get_frame(float x, AnimationFrame& frame)
{
	// morphs
	size_t num_morphs = morphs.size();
	frame.morphs.resize(num_morphs);	
	for (size_t i = 0; i < num_morphs; i++)
	{
		morphs[i].get_frame(x, frame.morphs[i]);
	}

	// translations
	size_t num_trans = translations.size();
	frame.translations.resize(num_trans);
	for (size_t i = 0; i < num_trans; i++)
	{
		translations[i].get_frame(x, frame.translations[i]);
	}

	// rotations
	size_t num_rots = rotations.size();
	frame.rotations.resize(num_rots);
	for (size_t i = 0; i < num_rots; i++)
	{
		rotations[i].get_frame(x, frame.rotations[i]);
	}

	// scales
	size_t num_scales = scales.size();
	frame.scales.resize(num_scales);
	for (size_t i = 0; i < num_scales; i++)
	{
		scales[i].get_frame(x, frame.scales[i]);
	}
}

