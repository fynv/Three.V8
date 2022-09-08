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

class Scene;
class Fog;
class Camera;
class GLRenderTarget;
class CubeRenderTarget;
class SimpleModel;
class GLTFModel;
class DirectionalLight;
class DirectionalLightShadow;
class GLRenderer
{
public:
	GLRenderer();
	~GLRenderer();
	void render(Scene& scene, Camera& camera, GLRenderTarget& target);	
	void renderCube(Scene& scene, CubeRenderTarget& target, glm::vec3& position, float zNear, float zFar);

private:
	std::unique_ptr<WeightedOIT> oit_resolvers[2];

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

	std::unique_ptr<DrawWire> WireDraw;

	void render_primitive(const StandardRoutine::RenderParams& params, Pass pass);
	void render_model(Camera* p_camera, const Lights& lights, const Fog* fog, SimpleModel* model, Pass pass);
	void render_model(Camera* p_camera, const Lights& lights, const Fog* fog, GLTFModel* model, Pass pass);

	std::unique_ptr<MorphUpdate> morphers[4];
	std::unique_ptr<SkinUpdate> skinners[2];

	// directional shadow maps
	std::unordered_map<uint64_t, std::unique_ptr<DirectionalShadowCast>> directional_shadow_caster_map;
	DirectionalShadowCast* get_shadow_caster(const DirectionalShadowCast::Options& options);

	void render_shadow_primitive(const DirectionalShadowCast::RenderParams& params);
	void render_shadow_model(DirectionalLightShadow* shadow, SimpleModel* model);
	void render_shadow_model(DirectionalLightShadow* shadow, GLTFModel* model);

	std::unique_ptr<DrawTexture> TextureVisualizer;
	std::unique_ptr<DrawSkyBox> SkyBoxDraw;
	std::unique_ptr<DrawHemisphere> HemisphereDraw;

	struct PreRender
	{
		// lists
		std::vector<SimpleModel*> simple_models;
		std::vector<GLTFModel*> gltf_models;
		std::vector<DirectionalLight*> directional_lights;

	};

	void _pre_render(Scene& scene, PreRender& pre);
	void _render(Scene& scene, Camera& camera, GLRenderTarget& target, PreRender& pre);
	void _render_cube(Scene& scene, CubeRenderTarget& target, glm::vec3& position, float zNear, float zFar, const PreRender& pre);
	
	std::unordered_map<uint64_t, std::unique_ptr<DrawFog>> fog_draw_map;
	std::unique_ptr<FogRayMarching> fog_ray_march[2];
	
	void _render_fog(const Camera& camera, const Lights& lights, const Fog& fog, GLRenderTarget& target);
	void _render_fog_rm(const Camera& camera, DirectionalLight& light, const Fog& fog, GLRenderTarget& target);

	
};

