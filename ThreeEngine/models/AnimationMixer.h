#pragma once

#include <vector>
#include <unordered_map>
#include "models/Animation.h"

class AnimationMixer
{
public:
	std::vector<AnimationClip> m_animations;
	std::unordered_map<std::string, int> m_animation_dict;
	void addAnimation(const AnimationClip& anim);

	struct PlayBack
	{
		int id_anim;
		double time_start;
		float weight;
	};
	std::vector<PlayBack> m_current_playing;

	void startAnimation(const char* name);
	void stopAnimation(int i);

	void getFrame(AnimationFrame& frame);

};

