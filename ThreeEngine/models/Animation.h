#pragma once

#include <string>
#include <vector>

// Frame
class MorphFrame
{
public:
	std::string name;
	std::vector<float> weights;
};

class AnimationFrame
{
public:
	std::vector<MorphFrame> morphs;
};

// Tracks
enum class Interpolation
{
	STEP,
	LINEAR,
	CUBICSPLINE
};

class MorphTrack
{
public:
	std::string name;
	
	int num_targets = 0;
	Interpolation interpolation = Interpolation::LINEAR;

	std::vector<float> times;
	std::vector<float> values;	

	void get_frame(float x, MorphFrame& frame);
};


// Clip
class AnimationClip
{
public:
	std::string name;
	double start = 0.0f;
	double end = FLT_MAX;

	std::vector<MorphTrack> morphs;

	void get_frame(float x, AnimationFrame& frame);
};
