#pragma once

class GLTFModel;
class GLTFLoader
{
public:
	static void LoadModelFromFile(GLTFModel* model, const char* filename);
};