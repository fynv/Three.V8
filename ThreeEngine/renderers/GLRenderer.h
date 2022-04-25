#pragma once

#include <memory>
#include <unordered_map>
#include "renderers/routines/StandardRoutine.h"
#include "renderers/routines/MorphUpdate.h"
#include "renderers/routines/SkinUpdate.h"
#include "renderers/routines/WeightedOIT.h"
#include "renderers/routines/DirectionalShadowCast.h"
#include "renderers/routines/DrawTexture.h"
#include "renderers/routines/DrawSkyBox.h"
#include "renderers/routines/DrawHemisphere.h"

class Scene;
class Camera;
class Caches;
class SimpleModel;
class GLTFModel;
class DirectionalLightShadow;
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

	void update_simple_model(SimpleModel* model);
	void update_gltf_model(GLTFModel* model);

	enum class Pass
	{
		Opaque,
		Highlight,
		Alpha
	};

	std::unordered_map<uint64_t, std::unique_ptr<StandardRoutine>> routine_map;
	StandardRoutine* get_routine(const StandardRoutine::Options& options);

	void render_primitive(const StandardRoutine::RenderParams& params, Pass pass);
	void render_model(Camera* p_camera, const Lights& lights, SimpleModel* model, Pass pass);
	void render_model(Camera* p_camera, const Lights& lights, GLTFModel* model, Pass pass);

	std::unique_ptr<MorphUpdate> morphers[2];
	std::unique_ptr<SkinUpdate> skinners[2];

	std::unique_ptr<WeightedOIT> OITResolver;

	// directional shadow maps
	std::unordered_map<uint64_t, std::unique_ptr<DirectionalShadowCast>> directional_shadow_caster_map;
	DirectionalShadowCast* get_shadow_caster(const DirectionalShadowCast::Options& options);

	void render_shadow_primitive(const DirectionalShadowCast::RenderParams& params);
	void render_shadow_model(DirectionalLightShadow* shadow, SimpleModel* model);
	void render_shadow_model(DirectionalLightShadow* shadow, GLTFModel* model);


	std::unique_ptr<DrawTexture> TextureVisualizer;
	std::unique_ptr<DrawSkyBox> SkyBoxDraw;
	std::unique_ptr<DrawHemisphere> HemisphereDraw;
	
};

