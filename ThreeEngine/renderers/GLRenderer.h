#pragma once

#include <memory>
#include <unordered_map>
#include "renderers/routines/StandardRoutine.h"
#include "renderers/routines/MorphUpdate.h"
#include "renderers/routines/SkinUpdate.h"

class Scene;
class Camera;
class Caches;
class SimpleModel;
class GLTFModel;
class GLRenderer
{
public:
	GLRenderer();
	~GLRenderer();
	void render(int width, int height, Scene& scene, Camera& camera);

private:
	int m_width = -1;
	int m_height = -1;
	unsigned m_tex_msaa = -1;
	unsigned m_rbo_msaa = -1;
	unsigned m_fbo_msaa = -1;
	void _update_framebuffers(int width, int height);

	std::unordered_map<uint64_t, std::unique_ptr<StandardRoutine>> routine_map;
	StandardRoutine* get_routine(const StandardRoutine::Options& options);
	void render_primitive(const StandardRoutine::RenderParams& params);
	
	void update_simple_model(SimpleModel* model);
	void update_gltf_model(GLTFModel* model);
	void render_simple_model(Camera* p_camera, SimpleModel* model);
	void render_gltf_model(Camera* p_camera, GLTFModel* model);

	std::unique_ptr<MorphUpdate> morphers[2];
	std::unique_ptr<SkinUpdate> skinners[2];
};

