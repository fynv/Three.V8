#pragma once

#include <memory>
#include <unordered_map>
#include "renderers/routines/StandardRoutine.h"
#include "renderers/routines/DrawWire.h"
#include "renderers/routines/MorphUpdate.h"
#include "renderers/routines/SkinUpdate.h"
#include "renderers/routines/WeightedOIT.h"
#include "renderers/routines/DirectionalShadowCast.h"
#include "renderers/routines/DrawTexture.h"
#include "renderers/routines/DrawSkyBox.h"
#include "renderers/routines/DrawHemisphere.h"
#include "renderers/routines/DrawFog.h"
#include "renderers/routines/FogRayMarching.h"
#include "renderers/routines/FogRayMarchingEnv.h"
#include "renderers/routines/BaseColorRoutine.h"
#include "renderers/routines/LightingRoutine.h"
#include "renderers/routines/AlphaDem.h"

#include "volume/routines/DrawIsosurface.h"
#include "volume/routines/IsosurfaceDirectionalShadow.h"

#include "renderers/routines/Picking.h"

#include "lights/EnvironmentMapCreator.h"
#include "renderers/routines/DepthOnly.h"

class Scene;
class Fog;
class Camera;
class GLRenderTarget;
class GLPickingTarget;
class GLSpaceProbeTarget;
class CubeRenderTarget;
class SimpleModel;
class GLTFModel;
class DirectionalLight;
class DirectionalLightShadow;
class ProbeGrid;
class ProbeGridWidget;

class VolumeIsosurfaceModel;

class GLRenderer
{
public:
	GLRenderer();
	~GLRenderer();

	void render(Scene& scene, Camera& camera, GLRenderTarget& target);	
	void render_picking(Scene& scene, Camera& camera, GLPickingTarget& target);

	void renderCube(Scene& scene, CubeRenderTarget& target, const glm::vec3& position, float zNear, float zFar, const glm::quat& rotation = glm::identity<glm::quat>());

	void updateProbe(Scene& scene, CubeRenderTarget& target, ProbeGrid& probe_grid, glm::ivec3 idx, float zNear, float zFar, float k = 1.0f);

	void renderCelluloid(Scene& scene, Camera& camera, GLRenderTarget* layer_base, GLRenderTarget* layer_light, GLRenderTarget* layer_alpha);

	void renderTexture(GLTexture2D* tex, int x, int y, int width, int height, GLRenderTarget& target);

private:
	std::unique_ptr<WeightedOIT> oit_resolvers[2];

	void update_model(SimpleModel* model);
	void update_model(GLTFModel* model);
	
	void update_model(VolumeIsosurfaceModel* model);

	enum class Pass
	{
		Opaque,
		Highlight,
		Alpha
	};

	std::unordered_map<uint64_t, std::unique_ptr<StandardRoutine>> routine_map;
	StandardRoutine* get_routine(const StandardRoutine::Options& options);

	std::unique_ptr<DrawWire> WireDraw;

	std::unordered_map<uint64_t, std::unique_ptr<DrawIsosurface>> IsosurfaceDraw;
	DrawIsosurface* get_isosurface_draw(const DrawIsosurface::Options& options);

	void render_primitive(const StandardRoutine::RenderParams& params, Pass pass);
	void render_model(Camera* p_camera, const Lights& lights, const Fog* fog, SimpleModel* model, Pass pass);
	void render_model(Camera* p_camera, const Lights& lights, const Fog* fog, GLTFModel* model, Pass pass);
	void render_model(Camera* p_camera, const Lights& lights, const Fog* fog, VolumeIsosurfaceModel* model, GLRenderTarget& target, Pass pass);
	void render_widget(Camera* p_camera, DirectionalLight* light);
	void render_widget(Camera* p_camera, ProbeGridWidget* widget);

	std::unique_ptr<MorphUpdate> morphers[4];
	std::unique_ptr<SkinUpdate> skinners[2];

	// directional shadow maps
	std::unordered_map<uint64_t, std::unique_ptr<DirectionalShadowCast>> directional_shadow_caster_map;
	DirectionalShadowCast* get_shadow_caster(const DirectionalShadowCast::Options& options);

	std::unique_ptr<IsosurfaceDirectionalShadow> isosurface_directional_shadow;

	void render_shadow_primitive(const DirectionalShadowCast::RenderParams& params);
	void render_shadow_model(DirectionalLightShadow* shadow, SimpleModel* model);
	void render_shadow_model(DirectionalLightShadow* shadow, GLTFModel* model);
	void render_shadow_model(DirectionalLightShadow* shadow, VolumeIsosurfaceModel* model);
	

	std::unique_ptr<DrawSkyBox> SkyBoxDraw;
	std::unique_ptr<DrawHemisphere> HemisphereDraw;

	std::unique_ptr<DepthOnly> DepthRenderer;
	void render_depth_primitive(const DepthOnly::RenderParams& params);
	void render_depth_model(Camera* p_camera, SimpleModel* model);
	void render_depth_model(Camera* p_camera, GLTFModel* model);

	void _pre_render(Scene& scene);
	void probe_space_center(Scene& scene, Camera& camera, GLSpaceProbeTarget& target, int width, int height, glm::vec3& ave, float& sum_weight);
	glm::vec3 probe_space_center_cube(Scene& scene, const glm::vec3& position, float zNear, float zFar, IndirectLight& light);
	void _render_scene(Scene& scene, Camera& camera, GLRenderTarget& target, bool widgets = false);
	void _render(Scene& scene, Camera& camera, GLRenderTarget& target, bool widgets = false);
	void _render_cube(Scene& scene, CubeRenderTarget& target, const glm::vec3& position, float zNear, float zFar, const glm::quat& rotation = glm::identity<glm::quat>());

	std::unordered_map<uint64_t, std::unique_ptr<Picking>> picking_map;
	Picking* get_picking(const Picking::Options& options);

	void render_picking_primitive(const Picking::RenderParams& params);
	void render_picking_model(Camera* p_camera, SimpleModel* model, GLPickingTarget& target);
	void render_picking_model(Camera* p_camera, GLTFModel* model, GLPickingTarget& target);
	
	std::unordered_map<uint64_t, std::unique_ptr<DrawFog>> fog_draw_map;
	std::unique_ptr<FogRayMarching> fog_ray_march[2];
	std::unordered_map<uint64_t, std::unique_ptr<FogRayMarchingEnv>> fog_ray_march_map;
	
	void _render_fog(const Camera& camera, const Lights& lights, const Fog& fog, GLRenderTarget& target);
	void _render_fog_rm(const Camera& camera, DirectionalLight& light, const Fog& fog, GLRenderTarget& target);	
	void _render_fog_rm_env(const Camera& camera, const Lights& lights, const Fog& fog, GLRenderTarget& target);

	std::unordered_map<uint64_t, std::unique_ptr<BaseColorRoutine>> base_routine_map;
	void render_primitive_base(const BaseColorRoutine::RenderParams& params);
	void render_model_base(Camera* p_camera, SimpleModel* model);
	void render_model_base(Camera* p_camera, GLTFModel* model);

	std::unordered_map<uint64_t, std::unique_ptr<LightingRoutine>> lighting_routine_map;
	void render_primitive_lighting(const LightingRoutine::RenderParams& params);
	void render_model_lighting(Camera* p_camera, const Lights& lights, const Fog* fog, SimpleModel* model);
	void render_model_lighting(Camera* p_camera, const Lights& lights, const Fog* fog, GLTFModel* model);

	std::unique_ptr<AlphaDem> alpha_demodulate;

	std::unique_ptr<DrawTexture> TextureDraw;

	std::unique_ptr<EnvironmentMapCreator> EnvCreator;

};

