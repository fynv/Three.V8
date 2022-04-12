#pragma once

#include <string>
#include <vector>
#include <glm.hpp>
#include <gtx/quaternion.hpp>

// Frame
class MorphFrame
{
public:
	std::string name;
	std::vector<float> weights;
};

class TranslationFrame
{
public:
	std::string name;	
	glm::vec3 translation;	
};

class RotationFrame
{
public:
	std::string name;
	glm::quat rotation;
};

class ScaleFrame
{
public:
	std::string name;
	glm::vec3 scale;
};

class AnimationFrame
{
public:
	std::vector<MorphFrame> morphs;
	std::vector<TranslationFrame> translations;
	std::vector<RotationFrame> rotations;
	std::vector<ScaleFrame> scales;
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

class TranslationTrack
{
public:
	std::string name;
	Interpolation interpolation = Interpolation::LINEAR;

	std::vector<float> times;
	std::vector<glm::vec3> values;

	void get_frame(float x, TranslationFrame& frame);
};

class RotationTrack
{
public:
	std::string name;
	Interpolation interpolation = Interpolation::LINEAR;

	std::vector<float> times;
	std::vector<glm::quat> values;

	void get_frame(float x, RotationFrame& frame);
};

class ScaleTrack
{
public:
	std::string name;
	Interpolation interpolation = Interpolation::LINEAR;

	std::vector<float> times;
	std::vector<glm::vec3> values;

	void get_frame(float x, ScaleFrame& frame);

};

// Clip
class AnimationClip
{
public:
	std::string name;
	double duration = 0.0f;

	std::vector<MorphTrack> morphs;
	std::vector<TranslationTrack> translations;
	std::vector<RotationTrack> rotations;
	std::vector<ScaleTrack> scales;

	void get_frame(float x, AnimationFrame& frame);
};
