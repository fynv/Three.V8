#pragma once

#include <memory>
#include <unordered_map>
#include "renderers/routines/StandardRoutine.h"
#include "renderers/routines/SimpleRoutine.h"
#include "renderers/routines/DrawWire.h"
#include "renderers/routines/MorphUpdate.h"
#include "renderers/routines/SkinUpdate.h"
#include "renderers/routines/WeightedOIT.h"
#include "renderers/routines/DirectionalShadowCast.h"
#include "renderers/routines/CopyDepth.h"
#include "renderers/routines/DrawTexture.h"
#include "renderers/routines/DrawSkyBox.h"
#include "renderers/routines/DrawHemisphere.h"
#include "renderers/routines/DrawFog.h"
#include "renderers/routines/FogRayMarching.h"
#include "renderers/routines/FogRayMarchingEnv.h"
#include "renderers/routines/SimpleFogRayMarching.h"
#include "renderers/routines/SimpleFogRayMarchingEnv.h"

#include "volume/routines/DrawIsosurface.h"
#include "volume/routines/IsosurfaceDirectionalShadow.h"

#include "renderers/routines/Picking.h"

#include "lights/EnvironmentMapCreator.h"
#include "lights/ReflectionDistanceCreator.h"
#include "renderers/routines/DepthOnly.h"
#include "renderers/routines/SSAO.h"

#include "renderers/routines/DepthDownsample.h"
#include "renderers/routines/ReflectionCopy.h"
#include "renderers/routines/ReflectionMipmaps.h"

#include "renderers/routines/SceneToVolume.h"

#include "renderers/routines/RasterizeAtlas.h"
//#include "renderers/routines/CompressLightmap.h"
//#include "renderers/routines/DecompressLightmap.h"

#include "renderers/routines/NormalAndDepth.h"

#include "BVHRenderer.h"

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
class LODProbeGrid;
class LODProbeGridWidget;
class Reflector;

class VolumeIsosurfaceModel;

class GLRenderer
{
public:
	GLRenderer();
	~GLRenderer();

	bool m_use_ssao = false;

	void render(Scene& scene, Camera& camera, GLRenderTarget& target);	
	void render_picking(Scene& scene, Camera& camera, GLPickingTarget& target);

	void renderCube(Scene& scene, CubeRenderTarget& target, const glm::vec3& position, float zNear, float zFar, const glm::quat& rotation = glm::identity<glm::quat>());

	void updateProbe(Scene& scene, CubeRenderTarget& target, ProbeGrid& probe_grid, glm::ivec3 idx, float zNear, float zFar, float k = 1.0f);	
	void updateProbe(Scene& scene, CubeRenderTarget& target, LODProbeGrid& probe_grid, int idx, float zNear, float zFar, float k = 1.0f);

	int updateProbes(Scene& scene, ProbeGrid& probe_grid, int start_idx, int num_directions, float rate_vis = 0.5f, float rate_irr = 1.0f);
	int updateProbes(Scene& scene, LODProbeGrid& probe_grid, int start_idx, int num_directions, float rate_vis = 0.5f, float rate_irr = 1.0f);

	void renderTexture(GLTexture2D* tex, int x, int y, int width, int height, GLRenderTarget& target, bool flipY = true, float alpha = 1.0f);

	void sceneToVolume(Scene& scene, unsigned tex_id_volume, const glm::vec3& coverage_min, const glm::vec3& coverage_max, const glm::ivec3& divisions);

	void rasterize_atlas(GLTFModel* model);

	int updateLightmap(Scene& scene, Lightmap& lm, LightmapRenderTarget& src, int start_texel, int num_directions = 64, float k = 1.0f);
	void filterLightmap(Lightmap& lm, LightmapRenderTarget& src, const glm::mat4& model_mat);

//	bool compressLightmap(Scene& scene, GLTFModel* model);
//	bool decompressLightmap(Scene& scene, GLTFModel* model);

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

	std::unordered_map<uint64_t, std::unique_ptr<SimpleRoutine>> simple_routine_map;
	SimpleRoutine* get_simple_routine(const SimpleRoutine::Options& options);

	std::unique_ptr<DrawWire> WireDraw;

	std::unordered_map<uint64_t, std::unique_ptr<DrawIsosurface>> IsosurfaceDraw;
	DrawIsosurface* get_isosurface_draw(const DrawIsosurface::Options& options);

	void render_primitive(const StandardRoutine::RenderParams& params, Pass pass);
	void render_primitives(const StandardRoutine::RenderParams& params, Pass pass, const std::vector<void*>& offset_lst, const std::vector<int>& count_lst); // batched
	void render_model(Camera* p_camera, const Lights& lights, const Fog* fog, SimpleModel* model, GLRenderTarget& target, Pass pass);
	void render_model(Camera* p_camera, const Lights& lights, const Fog* fog, GLTFModel* model, GLRenderTarget& target, Pass pass);
	void render_model(Camera* p_camera, const Lights& lights, const Fog* fog, VolumeIsosurfaceModel* model, GLRenderTarget& target, Pass pass);
	void render_widget(Camera* p_camera, DirectionalLight* light);
	void render_widget(Camera* p_camera, ProbeGridWidget* widget);
	void render_widget(Camera* p_camera, LODProbeGridWidget* widget);
	void render_widget(Camera* p_camera, Reflector* reflector);

	void render_primitive_simple(const SimpleRoutine::RenderParams& params, Pass pass);
	void render_primitives_simple(const SimpleRoutine::RenderParams& params, Pass pass, const std::vector<void*>& offset_lst, const std::vector<int>& count_lst);  // batched
	void render_model_simple(Camera* p_camera, const Lights& lights, const Fog* fog, SimpleModel* model, Pass pass);
	void render_model_simple(Camera* p_camera, const Lights& lights, const Fog* fog, GLTFModel* model, Pass pass);

	std::unique_ptr<MorphUpdate> morphers[4];
	std::unique_ptr<SkinUpdate> skinners[2];

	// directional shadow maps
	std::unordered_map<uint64_t, std::unique_ptr<DirectionalShadowCast>> directional_shadow_caster_map;
	DirectionalShadowCast* get_shadow_caster(const DirectionalShadowCast::Options& options);

	std::unique_ptr<IsosurfaceDirectionalShadow> isosurface_directional_shadow;
	
	std::unique_ptr<CopyDepth> copy_shadow;

	void render_shadow_primitive(const DirectionalShadowCast::RenderParams& params);
	void render_shadow_primitives(const DirectionalShadowCast::RenderParams& params, const std::vector<void*>& offset_lst, const std::vector<int>& count_lst);  // batched
	void render_shadow_model(DirectionalLightShadow* shadow, SimpleModel* model);
	void render_shadow_model(DirectionalLightShadow* shadow, GLTFModel* model);
	void render_shadow_model(DirectionalLightShadow* shadow, VolumeIsosurfaceModel* model);
	

	std::unique_ptr<DrawSkyBox> SkyBoxDraw;
	std::unique_ptr<DrawHemisphere> HemisphereDraw;

	std::unique_ptr<DepthOnly> DepthRenderer;
	void render_depth_primitive(const DepthOnly::RenderParams& params);
	void render_depth_primitives(const DepthOnly::RenderParams& params, const std::vector<void*>& offset_lst, const std::vector<int>& count_lst);  // batched
	void render_depth_model(Camera* p_camera, SimpleModel* model);
	void render_depth_model(Camera* p_camera, GLTFModel* model);	

	std::unique_ptr<NormalAndDepth> NormalDepthRenderer[2];
	void render_normal_depth_primitive(const NormalAndDepth::RenderParams& params);
	void render_normal_depth_primitives(const NormalAndDepth::RenderParams& params, const std::vector<void*>& offset_lst, const std::vector<int>& count_lst);  // batched
	void render_normal_depth_model(Camera* p_camera, SimpleModel* model);
	void render_normal_depth_model(Camera* p_camera, GLTFModel* model);

	void _pre_render(Scene& scene);
	void probe_space_center(Scene& scene, Camera& camera, GLSpaceProbeTarget& target, int width, int height, glm::vec3& ave, float& sum_weight);
	glm::vec3 probe_space_center_cube(Scene& scene, const glm::vec3& position, float zNear, float zFar, IndirectLight& light);

	std::unique_ptr<SSAO> SSAORenderer[2];
	void _ssao(const Camera& camera, GLRenderTarget& target);

	void _render_scene(Scene& scene, Camera& camera, GLRenderTarget& target, bool widgets = false);
	void _render(Scene& scene, Camera& camera, GLRenderTarget& target, bool widgets = false);
	void _render_scene_simple(Scene& scene, Camera& camera, GLRenderTarget& target);
	void _render_simple(Scene& scene, Camera& camera, GLRenderTarget& target);
	void _render_cube(Scene& scene, CubeRenderTarget& target, const glm::vec3& position, float zNear, float zFar, const glm::quat& rotation = glm::identity<glm::quat>());

	std::unordered_map<uint64_t, std::unique_ptr<Picking>> picking_map;
	Picking* get_picking(const Picking::Options& options);

	void render_picking_primitive(const Picking::RenderParams& params);
	void render_picking_model(Camera* p_camera, SimpleModel* model, GLPickingTarget& target);
	void render_picking_model(Camera* p_camera, GLTFModel* model, GLPickingTarget& target);
	
	std::unordered_map<uint64_t, std::unique_ptr<DrawFog>> fog_draw_map;
	std::unordered_map<uint64_t, std::unique_ptr<FogRayMarching>> fog_ray_march;	
	std::unordered_map<uint64_t, std::unique_ptr<FogRayMarchingEnv>> fog_ray_march_map;
	
	void _render_fog(const Camera& camera, const Lights& lights, const Fog& fog, GLRenderTarget& target);
	void _render_fog_rm(const Camera& camera, DirectionalLight& light, const Fog& fog, GLRenderTarget& target);	
	void _render_fog_rm_env(const Camera& camera, const Lights& lights, const Fog& fog, GLRenderTarget& target);

	std::unique_ptr<SimpleFogRayMarching> fog_ray_march_simple[2];
	std::unordered_map<uint64_t, std::unique_ptr<SimpleFogRayMarchingEnv>> fog_ray_march_map_simple;
	void _render_fog_rm_simple(const Camera& camera, DirectionalLight& light, const Fog& fog, GLRenderTarget& target);
	void _render_fog_rm_env_simple(const Camera& camera, const Lights& lights, const Fog& fog, GLRenderTarget& target);
	
	std::unique_ptr<DrawTexture> TextureDraw;

	std::unique_ptr<EnvironmentMapCreator> EnvCreator;
	std::unique_ptr<ReflectionDistanceCreator> ReflDisCreator;

	std::unique_ptr<DepthDownsample> DepthDownsampler;
	std::unique_ptr<ReflectionCopy> ReflectionCopier;
	std::unique_ptr<ReflectionMipmaps> ReflectionMipmapper;

	std::unique_ptr<SceneToVolume> SceneVolumeConvert;
	void scene_to_volume_primitive(const SceneToVolume::RenderParams& params);
	void scene_to_volume_model(SimpleModel* model, SceneToVolume::RenderParams& params);
	void scene_to_volume_model(GLTFModel* model, SceneToVolume::RenderParams& params);

	std::unique_ptr<RasterizeAtlas> atlas_rasterizer[2];
	void rasterize_atlas_primitive(const RasterizeAtlas::RenderParams& params);

//	std::unique_ptr<CompressLightmap> lightmap_compressor[2];
//	std::unique_ptr<DecompressLightmap> lightmap_decompressor[2];

	BVHRenderer bvh_renderer;		
};

