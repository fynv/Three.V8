#pragma once

#include <memory>
#include <unordered_map>
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
	std::unordered_map<uint64_t, std::unique_ptr<TestRoutine>> routine_map;
	TestRoutine* get_routine(const TestRoutine::Options& options);
	void render_primitive(const TestRoutine::RenderParams& params);
};

