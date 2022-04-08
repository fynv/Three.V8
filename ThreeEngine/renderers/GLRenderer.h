#pragma once

#include <memory>
#include <unordered_map>
#include "renderers/routines/StandardRoutine.h"

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
	std::unordered_map<uint64_t, std::unique_ptr<StandardRoutine>> routine_map;
	StandardRoutine* get_routine(const StandardRoutine::Options& options);
	void render_primitive(const StandardRoutine::RenderParams& params);
};

