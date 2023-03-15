#pragma once

#include <memory>
#include <unordered_map>
#include "renderers/bvh_routines/BVHRoutine.h"
#include "renderers/bvh_routines/BVHDepthOnly.h"
#include "renderers/bvh_routines/CompSkyBox.h"
#include "renderers/bvh_routines/CompHemisphere.h"
#include "renderers/bvh_routines/CompWeightedOIT.h"
#include "renderers/bvh_routines/CompDrawFog.h"
#include "renderers/bvh_routines/CompFogRayMarching.h"
#include "renderers/bvh_routines/CompFogRayMarchingEnv.h"

class Scene;
class Fog;
class Camera;
class BVHRenderTarget;
class SimpleModel;
class GLTFModel;
class DirectionalLight;
class DirectionalLightShadow;

class BVHRenderer
{
public:
	void render(Scene& scene, Camera& camera, BVHRenderTarget& target);

private:
	std::unique_ptr<CompWeightedOIT> oit_resolver;

	void check_bvh(SimpleModel* model);
	void check_bvh(GLTFModel* model);

	enum class Pass
	{
		Opaque,
		Alpha
	};

	std::unordered_map<uint64_t, std::unique_ptr<BVHRoutine>> routine_map;
	BVHRoutine* get_routine(const BVHRoutine::Options& options);

	void render_primitive(const BVHRoutine::RenderParams& params, Pass pass);
	void render_model(Camera* p_camera, const Lights& lights, const Fog* fog, SimpleModel* model, Pass pass, BVHRenderTarget& target);
	void render_model(Camera* p_camera, const Lights& lights, const Fog* fog, GLTFModel* model, Pass pass, BVHRenderTarget& target);

	std::unique_ptr<CompSkyBox> SkyBoxDraw;
	std::unique_ptr<CompHemisphere> HemisphereDraw;

	std::unique_ptr<BVHDepthOnly> DepthRenderer;
	void render_depth_primitive(const BVHDepthOnly::RenderParams& params);
	void render_depth_model(Camera* p_camera, SimpleModel* model, BVHRenderTarget& target);
	void render_depth_model(Camera* p_camera, GLTFModel* model, BVHRenderTarget& target);

	std::unordered_map<uint64_t, std::unique_ptr<CompDrawFog>> fog_draw_map;
	std::unique_ptr<CompFogRayMarching> fog_ray_march;
	std::unordered_map<uint64_t, std::unique_ptr<CompFogRayMarchingEnv>> fog_ray_march_map;

	void _render_fog(const Camera& camera, const Lights& lights, const Fog& fog, BVHRenderTarget& target);
	void _render_fog_rm(const Camera& camera, DirectionalLight& light, const Fog& fog, BVHRenderTarget& target);
	void _render_fog_rm_env(const Camera& camera, const Lights& lights, const Fog& fog, BVHRenderTarget& target);
};

