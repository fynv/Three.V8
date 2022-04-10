#pragma once

class GLTFModel;
class AnimationClip;
class GLTFLoader
{
public:
	static void LoadModelFromFile(GLTFModel* model, const char* filename);
	static void LoadAnimationsFromFile(std::vector<AnimationClip>& animations, const char* filename);
};