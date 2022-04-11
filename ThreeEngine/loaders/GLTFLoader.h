#pragma once

class GLTFModel;
class AnimationClip;
class GLTFLoader
{
public:
	static void LoadModelFromFile(GLTFModel* model, const char* filename);
	static void LoadAnimationsFromFile(std::vector<AnimationClip>& animations, const char* filename);

	static void LoadModelFromMemory(GLTFModel* model, unsigned char* data, size_t size);
	static void LoadAnimationsFromMemory(std::vector<AnimationClip>& animations, unsigned char* data, size_t size);
};