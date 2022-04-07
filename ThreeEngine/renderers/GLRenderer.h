#pragma once

#include "renderers/routines/TestRoutine.h"

class Scene;
class Camera;
class Caches;
class GLRenderer
{
public:
	GLRenderer();
	~GLRenderer();
	void render(int width, int height, Scene& scene, Camera& camera);

private:
	TestRoutine test;
	TestRoutine2 test2;
};

