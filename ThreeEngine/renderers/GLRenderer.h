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
#include "renderers/routines/BaseColorRoutine.h"
#include "renderers/routines/LightingRoutine.h"
#include "renderers/routines/AlphaDem.h"

#include "volume/routines/DrawIsosurface.h"
#include "volume/routines/IsosurfaceDirectionalShadow.h"

#include "renderers/routines/Picking.h"

class Scene;
class Fog;
class Camera;
class GLRenderTarget;
class GLPickingTarget;
class CubeRenderTarget;
class SimpleModel;
class GLTFModel;
class DirectionalLight;
class DirectionalLightShadow;

class VolumeIsosurfaceModel;

class GLRenderer
{
public:
	struct Layer
	{
		Scene* scene;
		Camera* camera;
	};

	struct CubeLayer
	{
		Scene* scene;
		glm::vec3 position;
		float zNear, zFar;
	};


	GLRenderer();
	~GLRenderer();

	void render(Scene& scene, Camera& camera, GLRenderTarget& target);	
	void render_picking(Scene& scene, Camera& camera, GLPickingTarget& target);

	void renderCube(Scene& scene, CubeRenderTarget& target, glm::vec3& position, float zNear, float zFar);
	
	void renderLayers(size_t num_layers, Layer* layers, GLRenderTarget& target);
	void renderLayersToCube(size_t num_layers, CubeLayer* layers, CubeRenderTarget& target);

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

	void _pre_render(Scene& scene);
	void _render_scene(Scene& scene, Camera& camera, GLRenderTarget& target);
	void _render(Scene& scene, Camera& camera, GLRenderTarget& target);
	void _render_scene_to_cube(Scene& scene, CubeRenderTarget& target, glm::vec3& position, float zNear, float zFar);
	void _render_cube(Scene& scene, CubeRenderTarget& target, glm::vec3& position, float zNear, float zFar);	

	std::unordered_map<uint64_t, std::unique_ptr<Picking>> picking_map;
	Picking* get_picking(const Picking::Options& options);

	void render_picking_primitive(const Picking::RenderParams& params);
	void render_picking_model(Camera* p_camera, SimpleModel* model, GLPickingTarget& target);
	void render_picking_model(Camera* p_camera, GLTFModel* model, GLPickingTarget& target);
	
	std::unordered_map<uint64_t, std::unique_ptr<DrawFog>> fog_draw_map;
	std::unique_ptr<FogRayMarching> fog_ray_march[2];
	
	void _render_fog(const Camera& camera, const Lights& lights, const Fog& fog, GLRenderTarget& target);
	void _render_fog_rm(const Camera& camera, DirectionalLight& light, const Fog& fog, GLRenderTarget& target);	

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

};

