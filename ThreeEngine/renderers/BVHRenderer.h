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
#include "renderers/bvh_routines/VisibilityUpdate.h"
#include "renderers/bvh_routines/IrradianceUpdate.h"

class Scene;
class Fog;
class Camera;
class BVHRenderTarget;
class SimpleModel;
class GLTFModel;
class DirectionalLight;
class DirectionalLightShadow;

class ProbeRayList;
class ProbeRenderTarget;

class BVHRenderer
{
public:
	void render(Scene& scene, Camera& camera, BVHRenderTarget& target);

	void render_probe_depth(Scene& scene, ProbeRayList& prl, BVHRenderTarget& target);
	void update_probe_visibility(const BVHRenderTarget& source, const ProbeRayList& prl, const ProbeGrid& probe_grid, int id_start_probe, float mix_rate = 1.0f, ProbeRenderTarget* target = nullptr);
	void update_probe_visibility(const BVHRenderTarget& source, const ProbeRayList& prl, const LODProbeGrid& probe_grid, int id_start_probe, float mix_rate = 1.0f, ProbeRenderTarget* target = nullptr);

	void render_probe(Scene& scene, ProbeRayList& prl, BVHRenderTarget& target);
	void update_probe_irradiance(const BVHRenderTarget& source, const ProbeRayList& prl, const ProbeGrid& probe_grid, int id_start_probe, float mix_rate = 1.0f, ProbeRenderTarget* target = nullptr);
	void update_probe_irradiance(const BVHRenderTarget& source, const ProbeRayList& prl, const LODProbeGrid& probe_grid, int id_start_probe, float mix_rate = 1.0f, ProbeRenderTarget* target = nullptr);

private:
	std::unique_ptr<CompWeightedOIT> oit_resolver;

	void check_bvh(SimpleModel* model);
	void check_bvh(GLTFModel* model);

	enum class Pass
	{
		Opaque,
		Alpha
	};

	std::unique_ptr<CompSkyBox> SkyBoxDraw;
	std::unique_ptr<CompHemisphere> HemisphereDraw;

	std::unordered_map<uint64_t, std::unique_ptr<BVHRoutine>> routine_map;
	BVHRoutine* get_routine(const BVHRoutine::Options& options);

	void render_primitive(const BVHRoutine::RenderParams& params, Pass pass);
	void render_model(Camera* p_camera, const Lights& lights, const Fog* fog, SimpleModel* model, Pass pass, BVHRenderTarget& target);
	void render_model(Camera* p_camera, const Lights& lights, const Fog* fog, GLTFModel* model, Pass pass, BVHRenderTarget& target);

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

	/////////////// Render to Probe //////////////////

	std::unique_ptr<CompSkyBox> ProbeSkyBoxDraw;
	std::unique_ptr<CompHemisphere> ProbeHemisphereDraw;

	std::unordered_map<uint64_t, std::unique_ptr<BVHRoutine>> probe_routine_map;
	BVHRoutine* get_probe_routine(const BVHRoutine::Options& options);

	void render_probe_primitive(const BVHRoutine::RenderParams& params, Pass pass);
	void render_probe_model(ProbeRayList& prl, const Lights& lights, const Fog* fog, SimpleModel* model, Pass pass, BVHRenderTarget& target);
	void render_probe_model(ProbeRayList& prl, const Lights& lights, const Fog* fog, GLTFModel* model, Pass pass, BVHRenderTarget& target);

	std::unique_ptr<BVHDepthOnly> ProbeDepthRenderer;
	void render_probe_depth_primitive(const BVHDepthOnly::RenderParams& params);
	void render_probe_depth_model(ProbeRayList& prl, SimpleModel* model, BVHRenderTarget& target);
	void render_probe_depth_model(ProbeRayList& prl, GLTFModel* model, BVHRenderTarget& target);

	std::unordered_map<uint64_t, std::unique_ptr<CompDrawFog>> probe_fog_draw_map;
	std::unique_ptr<CompFogRayMarching> probe_fog_ray_march;
	std::unordered_map<uint64_t, std::unique_ptr<CompFogRayMarchingEnv>> probe_fog_ray_march_map;

	void _render_probe_fog(ProbeRayList& prl, const Lights& lights, const Fog& fog, BVHRenderTarget& target);
	void _render_probe_fog_rm(ProbeRayList& prl, DirectionalLight& light, const Fog& fog, BVHRenderTarget& target);
	void _render_probe_fog_rm_env(ProbeRayList& prl, const Lights& lights, const Fog& fog, BVHRenderTarget& target);

	std::unique_ptr<VisibilityUpdate> VisibilityUpdaters[2];
	std::unique_ptr<IrradianceUpdate> IrradianceUpdaters[4];
};

