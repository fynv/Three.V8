#include <GL/glew.h>
#include <half.hpp>
#include "crc64/crc64.h"
#include "GLUtils.h"
#include "BVHRenderer.h"
#include "BVHRenderTarget.h"
#include "cameras/Camera.h"
#include "cameras/PerspectiveCamera.h"
#include "scenes/Scene.h"
#include "backgrounds/Background.h"
#include "backgrounds/BackgroundScene.h"
#include "models/ModelComponents.h"
#include "models/SimpleModel.h"
#include "models/GLTFModel.h"
#include "lights/DirectionalLight.h"
#include "lights/DirectionalLightShadow.h"
#include "scenes/Fog.h"
#include "lights/ProbeRayList.h"
#include "renderers/LightmapRenderTarget.h"

void BVHRenderer::check_bvh(SimpleModel* model)
{
	if (model->geometry.cwbvh == nullptr)
	{
		model->geometry.cwbvh = std::unique_ptr<CWBVH>(new CWBVH(&model->geometry, model->matrixWorld));
	}
}

void BVHRenderer::check_bvh(GLTFModel* model)
{
	if (model->batched_mesh == nullptr)
	{
		model->batch_primitives();
	}

	//for (size_t i = 0; i < model->m_meshs.size(); i++)
	{
		//Mesh& mesh = model->m_meshs[i];
		Mesh& mesh = *model->batched_mesh;
		glm::mat4 matrix = model->matrixWorld;
		if (mesh.node_id >= 0 && mesh.skin_id < 0)
		{
			Node& node = model->m_nodes[mesh.node_id];
			matrix *= node.g_trans;
		}

		for (size_t j = 0; j < mesh.primitives.size(); j++)
		{
			Primitive& primitive = mesh.primitives[j];
			if (primitive.cwbvh == nullptr)
			{
				primitive.cwbvh = std::unique_ptr<CWBVH>(new CWBVH(&primitive, matrix));
			}
		}
	}	
}

BVHRoutine* BVHRenderer::get_routine(const BVHRoutine::Options& options)
{
	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(BVHRoutine::Options));
	auto iter = routine_map.find(hash);
	if (iter == routine_map.end())
	{
		routine_map[hash] = std::unique_ptr<BVHRoutine>(new BVHRoutine(options));
	}
	return routine_map[hash].get();
}

void BVHRenderer::render_primitive(const BVHRoutine::RenderParams& params, Pass pass)
{
	const MeshStandardMaterial* material = params.material_list[params.primitive->material_idx];
	const Lights* lights = params.lights;

	BVHRoutine::Options options;
	options.target_mode = 0;
	options.alpha_mode = material->alphaMode;	
	options.specular_glossiness = material->specular_glossiness;
	options.has_color = params.primitive->color_buf != nullptr;
	options.has_color_texture = material->tex_idx_map >= 0;
	options.has_metalness_map = material->tex_idx_metalnessMap >= 0;
	options.has_roughness_map = material->tex_idx_roughnessMap >= 0;
	options.has_emissive_map = material->tex_idx_emissiveMap >= 0;
	options.has_specular_map = material->tex_idx_specularMap >= 0;
	options.has_glossiness_map = material->tex_idx_glossinessMap >= 0;
	options.num_directional_lights = lights->num_directional_lights;
	options.num_directional_shadows = lights->num_directional_shadows;
	options.has_lightmap = params.tex_lightmap != nullptr;
	if (!options.has_lightmap)
	{
		options.has_environment_map = lights->environment_map != nullptr;
		options.has_probe_grid = lights->probe_grid != nullptr;
		if (options.has_probe_grid)
		{
			options.probe_reference_recorded = lights->probe_grid->record_references;
		}
		options.has_lod_probe_grid = lights->lod_probe_grid != nullptr;
		options.has_ambient_light = lights->ambient_light != nullptr;
		options.has_hemisphere_light = lights->hemisphere_light != nullptr;
	}
	options.has_fog = params.constant_fog != nullptr;
	BVHRoutine* routine = get_routine(options);
	routine->render(params);
}

void BVHRenderer::render_model(Camera* p_camera, const Lights& lights, const Fog* fog, SimpleModel* model, Pass pass, BVHRenderTarget& target)
{
	const GLTexture2D* tex = &model->texture;
	if (model->repl_texture != nullptr)
	{
		tex = model->repl_texture;
	}

	const MeshStandardMaterial* material = &model->material;

	if (pass == Pass::Opaque)
	{
		if (material->alphaMode == AlphaMode::Blend) return;
	}
	else if (pass == Pass::Alpha)
	{
		if (material->alphaMode != AlphaMode::Blend) return;
	}

	BVHRoutine::RenderParams params;
	params.tex_list = &tex;
	params.material_list = &material;	
	params.constant_model = &model->m_constant;
	params.primitive = &model->geometry;
	params.lights = &lights;
	params.tex_lightmap = nullptr;

	if (fog != nullptr)
	{
		params.constant_fog = &fog->m_constant;
	}
	else
	{
		params.constant_fog = nullptr;
	}

	params.target = &target;
	params.constant_camera = &p_camera->m_constant;

	render_primitive(params, pass);
}

void BVHRenderer::render_model(Camera* p_camera, const Lights& lights, const Fog* fog, GLTFModel* model, Pass pass, BVHRenderTarget& target)
{
	std::vector<const GLTexture2D*> tex_lst(model->m_textures.size());
	for (size_t i = 0; i < tex_lst.size(); i++)
	{
		auto iter = model->m_repl_textures.find(i);
		if (iter != model->m_repl_textures.end())
		{
			tex_lst[i] = iter->second;
		}
		else
		{
			tex_lst[i] = model->m_textures[i].get();
		}
	}

	std::vector<const MeshStandardMaterial*> material_lst(model->m_materials.size());
	for (size_t i = 0; i < material_lst.size(); i++)
		material_lst[i] = model->m_materials[i].get();

	//for (size_t i = 0; i < model->m_meshs.size(); i++)
	{
		//Mesh& mesh = model->m_meshs[i];
		Mesh& mesh = *model->batched_mesh;

		for (size_t j = 0; j < mesh.primitives.size(); j++)
		{
			Primitive& primitive = mesh.primitives[j];		

			const MeshStandardMaterial* material = material_lst[primitive.material_idx];
			if (pass == Pass::Opaque)
			{
				if (material->alphaMode == AlphaMode::Blend) continue;
			}
			else if (pass == Pass::Alpha)
			{
				if (material->alphaMode != AlphaMode::Blend) continue;
			}

			BVHRoutine::RenderParams params;
			params.tex_list = tex_lst.data();
			params.material_list = material_lst.data();			
			params.constant_model = mesh.model_constant.get();
			params.primitive = &primitive;
			params.lights = &lights;
			params.tex_lightmap = nullptr;
			if (model->lightmap != nullptr)
			{
				params.tex_lightmap = model->lightmap->lightmap.get();
			}

			if (fog != nullptr)
			{
				params.constant_fog = &fog->m_constant;
			}
			else
			{
				params.constant_fog = nullptr;
			}

			params.target = &target;
			params.constant_camera = &p_camera->m_constant;

			render_primitive(params, pass);
		}
	}

}


void BVHRenderer::render_depth_primitive(const BVHDepthOnly::RenderParams& params)
{
	const Primitive* prim = params.primitive;
	if (DepthRenderer == nullptr)
	{
		DepthRenderer = std::unique_ptr<BVHDepthOnly>(new BVHDepthOnly);
	}
	DepthRenderer->render(params);
}


void BVHRenderer::render_depth_model(Camera* p_camera, SimpleModel* model, BVHRenderTarget& target)
{
	const MeshStandardMaterial* material = &model->material;
	if (material->alphaMode != AlphaMode::Opaque) return;

	BVHDepthOnly::RenderParams params;
	params.material_list = &material;
	params.primitive = &model->geometry;
	params.target = &target;
	params.constant_camera = &p_camera->m_constant;
	render_depth_primitive(params);
}


void BVHRenderer::render_depth_model(Camera* p_camera, GLTFModel* model, BVHRenderTarget& target)
{
	std::vector<const MeshStandardMaterial*> material_lst(model->m_materials.size());
	for (size_t i = 0; i < material_lst.size(); i++)
		material_lst[i] = model->m_materials[i].get();

	//for (size_t i = 0; i < model->m_meshs.size(); i++)
	{
		//Mesh& mesh = model->m_meshs[i];
		Mesh& mesh = *model->batched_mesh;

		for (size_t j = 0; j < mesh.primitives.size(); j++)
		{
			Primitive& primitive = mesh.primitives[j];

			const MeshStandardMaterial* material = material_lst[primitive.material_idx];
			if (material->alphaMode != AlphaMode::Opaque) continue;

			BVHDepthOnly::RenderParams params;
			params.material_list = material_lst.data();
			params.primitive = &primitive;
			params.target = &target;
			params.constant_camera = &p_camera->m_constant;
			render_depth_primitive(params);
		}
	}
}

void BVHRenderer::_render_fog(const Camera& camera, const Lights& lights, const Fog& fog, BVHRenderTarget& target)
{
	CompDrawFog::Options options;	
	options.target_mode = 0;
	options.has_ambient_light = lights.ambient_light != nullptr;
	options.has_hemisphere_light = lights.hemisphere_light != nullptr;
	options.has_environment_map = lights.environment_map != nullptr;

	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(CompDrawFog::Options));
	auto iter = fog_draw_map.find(hash);
	if (iter == fog_draw_map.end())
	{
		fog_draw_map[hash] = std::unique_ptr<CompDrawFog>(new CompDrawFog(options));
	}
	CompDrawFog* fog_draw = fog_draw_map[hash].get();

	CompDrawFog::RenderParams params;		
	params.constant_fog = &fog.m_constant;
	params.lights = &lights;
	params.target = &target;

	fog_draw->render(params);

}

void BVHRenderer::_render_fog_rm(const Camera& camera, DirectionalLight& light, const Fog& fog, BVHRenderTarget& target)
{
	if (fog_ray_march == nullptr)
	{
		fog_ray_march = std::unique_ptr<CompFogRayMarching>(new CompFogRayMarching);
	}

	light.updateConstant();
	CompFogRayMarching::RenderParams params;	
	params.fog = &fog;
	params.constant_diretional_light = &light.m_constant;
	if (light.shadow != nullptr)
	{
		params.constant_diretional_shadow = &light.shadow->constant_shadow;
		params.tex_shadow = light.shadow->m_lightTex;
	}
	else
	{
		params.constant_diretional_shadow = nullptr;
		params.tex_shadow = -1;
	}
	params.target = &target;
	params.camera = &camera;
	fog_ray_march->render(params);

}


void BVHRenderer::_render_fog_rm_env(const Camera& camera, const Lights& lights, const Fog& fog, BVHRenderTarget& target)
{
	CompFogRayMarchingEnv::Options options;	
	options.target_mode = 0;
	options.has_probe_grid = lights.probe_grid != nullptr;
	if (options.has_probe_grid)
	{
		options.probe_reference_recorded = lights.probe_grid->record_references;
	}
	options.has_lod_probe_grid = lights.lod_probe_grid != nullptr;

	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(CompFogRayMarchingEnv::Options));
	auto iter = fog_ray_march_map.find(hash);
	if (iter == fog_ray_march_map.end())
	{
		fog_ray_march_map[hash] = std::unique_ptr<CompFogRayMarchingEnv>(new CompFogRayMarchingEnv(options));
	}
	CompFogRayMarchingEnv* fog_draw = fog_ray_march_map[hash].get();

	CompFogRayMarchingEnv::RenderParams params;	
	params.constant_fog = &fog.m_constant;
	params.lights = &lights;
	params.target = &target;
	params.constant_camera = &camera.m_constant;

	fog_draw->render(params);

}

void BVHRenderer::render(Scene& scene, Camera& camera, BVHRenderTarget& target)
{
	bool has_alpha = false;
	bool has_opaque = false;

	for (size_t i = 0; i < scene.simple_models.size(); i++)
	{
		SimpleModel* model = scene.simple_models[i];
		const MeshStandardMaterial* material = &model->material;
		if (material->alphaMode == AlphaMode::Blend)
		{
			has_alpha = true;
		}
		else
		{
			has_opaque = true;
		}
	}

	for (size_t i = 0; i < scene.gltf_models.size(); i++)
	{
		GLTFModel* model = scene.gltf_models[i];
		size_t num_materials = model->m_materials.size();
		for (size_t i = 0; i < num_materials; i++)
		{
			const MeshStandardMaterial* material = model->m_materials[i].get();
			if (material->alphaMode == AlphaMode::Blend)
			{
				has_alpha = true;
			}
			else
			{
				has_opaque = true;
			}
		}
	}

	while (scene.background != nullptr)
	{
		{
			ColorBackground* bg = dynamic_cast<ColorBackground*>(scene.background);
			if (bg != nullptr)
			{
				glm::vec4 color = { bg->color.r, bg->color.g, bg->color.b, 1.0f };
				glClearTexImage(target.m_tex_video->tex_id, 0, GL_RGBA, GL_FLOAT, &color);
				break;
			}
		}
		{
			CubeBackground* bg = dynamic_cast<CubeBackground*>(scene.background);
			if (bg != nullptr)
			{
				if (SkyBoxDraw == nullptr)
				{
					SkyBoxDraw = std::unique_ptr<CompSkyBox>(new CompSkyBox);
				}
				SkyBoxDraw->render(&camera.m_constant, &bg->cubemap, &target);
				break;
			}
		} 
		{
			HemisphereBackground* bg = dynamic_cast<HemisphereBackground*>(scene.background);
			if (bg != nullptr)
			{
				bg->updateConstant();
				if (HemisphereDraw == nullptr)
				{
					HemisphereDraw = std::unique_ptr<CompHemisphere>(new CompHemisphere);
				}
				HemisphereDraw->render(&camera.m_constant, &bg->m_constant, &target);
				break;
			}
		}
		{
			BackgroundScene* bg = dynamic_cast<BackgroundScene*>(scene.background);
			PerspectiveCamera* ref_cam = dynamic_cast<PerspectiveCamera*>(&camera);
			if (bg != nullptr && bg->scene != nullptr && ref_cam != nullptr)
			{
				BackgroundScene::Camera cam(bg, ref_cam);
				cam.updateMatrixWorld(false);
				cam.updateConstant();
				render(*bg->scene, cam, target);
			}

		}
		break;
	}


	for (size_t i = 0; i < scene.simple_models.size(); i++)
	{
		SimpleModel* model = scene.simple_models[i];
		check_bvh(model);
	}

	for (size_t i = 0; i < scene.gltf_models.size(); i++)
	{
		GLTFModel* model = scene.gltf_models[i];
		check_bvh(model);
	}

	Lights& lights = scene.lights;

	Fog* fog = nullptr;
	if (scene.fog != nullptr && scene.fog->density > 0)
	{
		fog = scene.fog;
	}


	float max_depth = FLT_MAX;
	glClearTexImage(target.m_tex_depth->tex_id, 0, GL_RED, GL_FLOAT, &max_depth);


	if (has_opaque)
	{
		// depth-prepass
		for (size_t i = 0; i < scene.simple_models.size(); i++)
		{
			SimpleModel* model = scene.simple_models[i];
			render_depth_model(&camera, model, target);
		}

		for (size_t i = 0; i < scene.gltf_models.size(); i++)
		{
			GLTFModel* model = scene.gltf_models[i];
			render_depth_model(&camera, model, target);
		}

		// opaque
		for (size_t i = 0; i < scene.simple_models.size(); i++)
		{
			SimpleModel* model = scene.simple_models[i];
			render_model(&camera, lights, fog, model, Pass::Opaque, target);
		}

		for (size_t i = 0; i < scene.gltf_models.size(); i++)
		{
			GLTFModel* model = scene.gltf_models[i];
			render_model(&camera, lights, fog, model, Pass::Opaque, target);
		}
	}

	if (has_alpha)
	{
		target.update_oit_buffers();

		if (oit_resolver == nullptr)
		{
			oit_resolver = std::unique_ptr<CompWeightedOIT>(new CompWeightedOIT);
		}
		oit_resolver->PreDraw(target.m_OITBuffers);

		for (size_t i = 0; i < scene.simple_models.size(); i++)
		{
			SimpleModel* model = scene.simple_models[i];
			render_model(&camera, lights, fog, model, Pass::Alpha, target);
		}

		for (size_t i = 0; i < scene.gltf_models.size(); i++)
		{
			GLTFModel* model = scene.gltf_models[i];
			render_model(&camera, lights, fog, model, Pass::Alpha, target);
		}
		
		oit_resolver->PostDraw(&target);
	}

	if (fog != nullptr)
	{
		fog->updateConstant();
		_render_fog(camera, lights, *fog, target);

		for (size_t i = 0; i < scene.directional_lights.size(); i++)
		{
			DirectionalLight* light = scene.directional_lights[i];
			_render_fog_rm(camera, *light, *fog, target);
		}

		if (lights.probe_grid != nullptr || lights.lod_probe_grid != nullptr)
		{
			_render_fog_rm_env(camera, lights, *fog, target);
		}
	}
}

/////////////// Render to Probe //////////////////

void BVHRenderer::render_probe_depth_primitive(const BVHDepthOnly::RenderParams& params)
{
	const Primitive* prim = params.primitive;
	if (ProbeDepthRenderer == nullptr)
	{
		ProbeDepthRenderer = std::unique_ptr<BVHDepthOnly>(new BVHDepthOnly(1));
	}
	ProbeDepthRenderer->render(params);
}

void BVHRenderer::render_probe_depth_model(ProbeRayList& prl, SimpleModel* model, BVHRenderTarget& target)
{
	const MeshStandardMaterial* material = &model->material;
	if (material->alphaMode != AlphaMode::Opaque) return;

	BVHDepthOnly::RenderParams params;
	params.material_list = &material;
	params.primitive = &model->geometry;
	params.target = &target;
	params.prl = &prl;
	render_probe_depth_primitive(params);
}

void BVHRenderer::render_probe_depth_model(ProbeRayList& prl, GLTFModel* model, BVHRenderTarget& target)
{
	std::vector<const MeshStandardMaterial*> material_lst(model->m_materials.size());
	for (size_t i = 0; i < material_lst.size(); i++)
		material_lst[i] = model->m_materials[i].get();

	//for (size_t i = 0; i < model->m_meshs.size(); i++)
	{
		//Mesh& mesh = model->m_meshs[i];
		Mesh& mesh = *model->batched_mesh;

		for (size_t j = 0; j < mesh.primitives.size(); j++)
		{
			Primitive& primitive = mesh.primitives[j];

			const MeshStandardMaterial* material = material_lst[primitive.material_idx];
			if (material->alphaMode != AlphaMode::Opaque) continue;

			BVHDepthOnly::RenderParams params;
			params.material_list = material_lst.data();
			params.primitive = &primitive;
			params.target = &target;
			params.prl = &prl;
			render_probe_depth_primitive(params);
		}
	}
}

void BVHRenderer::render_probe_depth(Scene& scene, ProbeRayList& prl, BVHRenderTarget& target)
{
	for (size_t i = 0; i < scene.simple_models.size(); i++)
	{
		SimpleModel* model = scene.simple_models[i];
		check_bvh(model);
	}

	for (size_t i = 0; i < scene.gltf_models.size(); i++)
	{
		GLTFModel* model = scene.gltf_models[i];
		check_bvh(model);
	}

	float max_half = std::numeric_limits<half_float::half>::max();	
	glClearTexImage(target.m_tex_depth->tex_id, 0, GL_RED, GL_FLOAT, &max_half);

	for (size_t i = 0; i < scene.simple_models.size(); i++)
	{
		SimpleModel* model = scene.simple_models[i];
		render_probe_depth_model(prl, model, target);
	}

	for (size_t i = 0; i < scene.gltf_models.size(); i++)
	{
		GLTFModel* model = scene.gltf_models[i];
		render_probe_depth_model(prl, model, target);
	}

}

void BVHRenderer::update_probe_visibility(const BVHRenderTarget& source, const ProbeRayList& prl, const ProbeGrid& probe_grid, int id_start_probe, float mix_rate, ProbeRenderTarget* target)
{
	if (VisibilityUpdaters[0] == nullptr)
	{
		VisibilityUpdaters[0] = std::unique_ptr<VisibilityUpdate>(new VisibilityUpdate(false));
	}
	VisibilityUpdate::RenderParams params;
	params.id_start_probe = id_start_probe;
	params.mix_rate = mix_rate;
	params.source = &source;
	params.prl = &prl;
	params.probe_grid = &probe_grid;
	params.lod_probe_grid = nullptr;
	params.target = target;
	VisibilityUpdaters[0]->update(params);
}


void BVHRenderer::update_probe_visibility(const BVHRenderTarget& source, const ProbeRayList& prl, const LODProbeGrid& probe_grid, int id_start_probe, float mix_rate, ProbeRenderTarget* target)
{
	if (VisibilityUpdaters[1] == nullptr)
	{
		VisibilityUpdaters[1] = std::unique_ptr<VisibilityUpdate>(new VisibilityUpdate(true));
	}
	VisibilityUpdate::RenderParams params;
	params.id_start_probe = id_start_probe;
	params.mix_rate = mix_rate;
	params.source = &source;
	params.prl = &prl;
	params.probe_grid = nullptr;
	params.lod_probe_grid = &probe_grid;
	params.target = target;
	VisibilityUpdaters[1]->update(params);
}


BVHRoutine* BVHRenderer::get_probe_routine(const BVHRoutine::Options& options)
{
	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(BVHRoutine::Options));
	auto iter = probe_routine_map.find(hash);
	if (iter == probe_routine_map.end())
	{
		probe_routine_map[hash] = std::unique_ptr<BVHRoutine>(new BVHRoutine(options));
	}
	return probe_routine_map[hash].get();
}

void BVHRenderer::render_probe_primitive(const BVHRoutine::RenderParams& params, Pass pass)
{
	const MeshStandardMaterial* material = params.material_list[params.primitive->material_idx];
	const Lights* lights = params.lights;

	BVHRoutine::Options options;
	options.target_mode = 1;
	options.has_lightmap = params.tex_lightmap != nullptr;
	options.alpha_mode = material->alphaMode;
	options.specular_glossiness = material->specular_glossiness;
	options.has_color = params.primitive->color_buf != nullptr;
	options.has_color_texture = material->tex_idx_map >= 0;
	options.has_metalness_map = material->tex_idx_metalnessMap >= 0;
	options.has_roughness_map = material->tex_idx_roughnessMap >= 0;
	options.has_emissive_map = material->tex_idx_emissiveMap >= 0;
	options.has_specular_map = material->tex_idx_specularMap >= 0;
	options.has_glossiness_map = material->tex_idx_glossinessMap >= 0;
	options.num_directional_lights = lights->num_directional_lights;
	options.num_directional_shadows = lights->num_directional_shadows;
	options.has_lightmap = params.tex_lightmap != nullptr;
	if (!options.has_lightmap)
	{
		options.has_environment_map = lights->environment_map != nullptr;
		options.has_probe_grid = lights->probe_grid != nullptr;
		if (options.has_probe_grid)
		{
			options.probe_reference_recorded = lights->probe_grid->record_references;
		}
		options.has_lod_probe_grid = lights->lod_probe_grid != nullptr;
		options.has_ambient_light = lights->ambient_light != nullptr;
		options.has_hemisphere_light = lights->hemisphere_light != nullptr;
	}
	options.has_fog = params.constant_fog != nullptr;
	BVHRoutine* routine = get_probe_routine(options);
	routine->render(params);
}

void BVHRenderer::render_probe_model(ProbeRayList& prl, const Lights& lights, const Fog* fog, SimpleModel* model, Pass pass, BVHRenderTarget& target)
{
	const GLTexture2D* tex = &model->texture;
	if (model->repl_texture != nullptr)
	{
		tex = model->repl_texture;
	}

	const MeshStandardMaterial* material = &model->material;

	if (pass == Pass::Opaque)
	{
		if (material->alphaMode == AlphaMode::Blend) return;
	}
	else if (pass == Pass::Alpha)
	{
		if (material->alphaMode != AlphaMode::Blend) return;
	}

	BVHRoutine::RenderParams params;
	params.tex_list = &tex;
	params.material_list = &material;
	params.constant_model = &model->m_constant;
	params.primitive = &model->geometry;
	params.lights = &lights;
	params.tex_lightmap = nullptr;

	if (fog != nullptr)
	{
		params.constant_fog = &fog->m_constant;
	}
	else
	{
		params.constant_fog = nullptr;
	}

	params.target = &target;
	params.prl = &prl;

	render_probe_primitive(params, pass);
}

void BVHRenderer::render_probe_model(ProbeRayList& prl, const Lights& lights, const Fog* fog, GLTFModel* model, Pass pass, BVHRenderTarget& target)
{
	std::vector<const GLTexture2D*> tex_lst(model->m_textures.size());
	for (size_t i = 0; i < tex_lst.size(); i++)
	{
		auto iter = model->m_repl_textures.find(i);
		if (iter != model->m_repl_textures.end())
		{
			tex_lst[i] = iter->second;
		}
		else
		{
			tex_lst[i] = model->m_textures[i].get();
		}
	}

	std::vector<const MeshStandardMaterial*> material_lst(model->m_materials.size());
	for (size_t i = 0; i < material_lst.size(); i++)
		material_lst[i] = model->m_materials[i].get();

	//for (size_t i = 0; i < model->m_meshs.size(); i++)
	{
		//Mesh& mesh = model->m_meshs[i];
		Mesh& mesh = *model->batched_mesh;

		for (size_t j = 0; j < mesh.primitives.size(); j++)
		{
			Primitive& primitive = mesh.primitives[j];

			const MeshStandardMaterial* material = material_lst[primitive.material_idx];
			if (pass == Pass::Opaque)
			{
				if (material->alphaMode == AlphaMode::Blend) continue;
			}
			else if (pass == Pass::Alpha)
			{
				if (material->alphaMode != AlphaMode::Blend) continue;
			}

			BVHRoutine::RenderParams params;
			params.tex_list = tex_lst.data();
			params.material_list = material_lst.data();
			params.constant_model = mesh.model_constant.get();
			params.primitive = &primitive;
			params.lights = &lights;
			params.tex_lightmap = nullptr;
			if (model->lightmap != nullptr)
			{
				params.tex_lightmap = model->lightmap->lightmap.get();
			}

			if (fog != nullptr)
			{
				params.constant_fog = &fog->m_constant;
			}
			else
			{
				params.constant_fog = nullptr;
			}

			params.target = &target;
			params.prl = &prl;

			render_probe_primitive(params, pass);
		}
	}
}

void BVHRenderer::_render_probe_fog(ProbeRayList& prl, const Lights& lights, const Fog& fog, BVHRenderTarget& target)
{
	CompDrawFog::Options options;
	options.target_mode = 1;
	options.has_ambient_light = lights.ambient_light != nullptr;
	options.has_hemisphere_light = lights.hemisphere_light != nullptr;
	options.has_environment_map = lights.environment_map != nullptr;

	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(CompDrawFog::Options));
	auto iter = probe_fog_draw_map.find(hash);
	if (iter == probe_fog_draw_map.end())
	{
		probe_fog_draw_map[hash] = std::unique_ptr<CompDrawFog>(new CompDrawFog(options));
	}
	CompDrawFog* fog_draw = probe_fog_draw_map[hash].get();

	CompDrawFog::RenderParams params;
	params.constant_fog = &fog.m_constant;
	params.lights = &lights;
	params.target = &target;

	fog_draw->render(params);

}

void BVHRenderer::_render_probe_fog_rm(ProbeRayList& prl, DirectionalLight& light, const Fog& fog, BVHRenderTarget& target)
{
	if (probe_fog_ray_march == nullptr)
	{
		probe_fog_ray_march = std::unique_ptr<CompFogRayMarching>(new CompFogRayMarching(1));
	}

	light.updateConstant();
	CompFogRayMarching::RenderParams params;
	params.fog = &fog;
	params.constant_diretional_light = &light.m_constant;
	if (light.shadow != nullptr)
	{
		params.constant_diretional_shadow = &light.shadow->constant_shadow;
		params.tex_shadow = light.shadow->m_lightTex;
	}
	else
	{
		params.constant_diretional_shadow = nullptr;
		params.tex_shadow = -1;
	}
	params.target = &target;
	params.prl = &prl;
	probe_fog_ray_march->render(params);

}

void BVHRenderer::_render_probe_fog_rm_env(ProbeRayList& prl, const Lights& lights, const Fog& fog, BVHRenderTarget& target)
{
	CompFogRayMarchingEnv::Options options;
	options.target_mode = 1;
	options.has_probe_grid = lights.probe_grid != nullptr;
	if (options.has_probe_grid)
	{
		options.probe_reference_recorded = lights.probe_grid->record_references;
	}
	options.has_lod_probe_grid = lights.lod_probe_grid != nullptr;

	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(CompFogRayMarchingEnv::Options));
	auto iter = probe_fog_ray_march_map.find(hash);
	if (iter == probe_fog_ray_march_map.end())
	{
		probe_fog_ray_march_map[hash] = std::unique_ptr<CompFogRayMarchingEnv>(new CompFogRayMarchingEnv(options));
	}
	CompFogRayMarchingEnv* fog_draw = probe_fog_ray_march_map[hash].get();

	CompFogRayMarchingEnv::RenderParams params;
	params.constant_fog = &fog.m_constant;
	params.lights = &lights;
	params.target = &target;
	params.prl = &prl;

	fog_draw->render(params);

}

void BVHRenderer::render_probe(Scene& scene, ProbeRayList& prl, BVHRenderTarget& target)
{
	bool has_alpha = false;
	bool has_opaque = false;

	for (size_t i = 0; i < scene.simple_models.size(); i++)
	{
		SimpleModel* model = scene.simple_models[i];
		const MeshStandardMaterial* material = &model->material;
		if (material->alphaMode == AlphaMode::Blend)
		{
			has_alpha = true;
		}
		else
		{
			has_opaque = true;
		}
	}

	for (size_t i = 0; i < scene.gltf_models.size(); i++)
	{
		GLTFModel* model = scene.gltf_models[i];
		size_t num_materials = model->m_materials.size();
		for (size_t i = 0; i < num_materials; i++)
		{
			const MeshStandardMaterial* material = model->m_materials[i].get();
			if (material->alphaMode == AlphaMode::Blend)
			{
				has_alpha = true;
			}
			else
			{
				has_opaque = true;
			}
		}
	}

	while (scene.background != nullptr)
	{
		{
			ColorBackground* bg = dynamic_cast<ColorBackground*>(scene.background);
			if (bg != nullptr)
			{
				glm::vec4 color = { bg->color.r, bg->color.g, bg->color.b, 1.0f };
				glClearTexImage(target.m_tex_video->tex_id, 0, GL_RGBA, GL_FLOAT, &color);
				break;
			}
		}
		{
			CubeBackground* bg = dynamic_cast<CubeBackground*>(scene.background);
			if (bg != nullptr)
			{
				if (ProbeSkyBoxDraw == nullptr)
				{
					ProbeSkyBoxDraw = std::unique_ptr<CompSkyBox>(new CompSkyBox(1));
				}
				ProbeSkyBoxDraw->render(&prl, &bg->cubemap, &target);
				break;
			}
		}
		{
			HemisphereBackground* bg = dynamic_cast<HemisphereBackground*>(scene.background);
			if (bg != nullptr)
			{
				bg->updateConstant();
				if (ProbeHemisphereDraw == nullptr)
				{
					ProbeHemisphereDraw = std::unique_ptr<CompHemisphere>(new CompHemisphere(1));
				}
				ProbeHemisphereDraw->render(&prl, &bg->m_constant, &target);
				break;
			}
		}
		{
			BackgroundScene* bg = dynamic_cast<BackgroundScene*>(scene.background);
			if (bg != nullptr && bg->scene != nullptr)
			{				
				render_probe(*bg->scene, prl, target);
			}

		}

		break;
	}

	for (size_t i = 0; i < scene.simple_models.size(); i++)
	{
		SimpleModel* model = scene.simple_models[i];
		check_bvh(model);
	}

	for (size_t i = 0; i < scene.gltf_models.size(); i++)
	{
		GLTFModel* model = scene.gltf_models[i];
		check_bvh(model);
	}

	Lights& lights = scene.lights;

	Fog* fog = nullptr;
	if (scene.fog != nullptr && scene.fog->density > 0)
	{
		fog = scene.fog;
	}


	float max_half = std::numeric_limits<half_float::half>::max();
	glClearTexImage(target.m_tex_depth->tex_id, 0, GL_RED, GL_FLOAT, &max_half);

	if (has_opaque)
	{
		// depth-prepass
		for (size_t i = 0; i < scene.simple_models.size(); i++)
		{
			SimpleModel* model = scene.simple_models[i];
			render_probe_depth_model(prl, model, target);
		}

		for (size_t i = 0; i < scene.gltf_models.size(); i++)
		{
			GLTFModel* model = scene.gltf_models[i];
			render_probe_depth_model(prl, model, target);
		}

		// opaque
		for (size_t i = 0; i < scene.simple_models.size(); i++)
		{
			SimpleModel* model = scene.simple_models[i];
			render_probe_model(prl, lights, fog, model, Pass::Opaque, target);
		}

		for (size_t i = 0; i < scene.gltf_models.size(); i++)
		{
			GLTFModel* model = scene.gltf_models[i];
			render_probe_model(prl, lights, fog, model, Pass::Opaque, target);
		}
	}

	if (has_alpha)
	{
		target.update_oit_buffers();

		if (oit_resolver == nullptr)
		{
			oit_resolver = std::unique_ptr<CompWeightedOIT>(new CompWeightedOIT);
		}
		oit_resolver->PreDraw(target.m_OITBuffers);

		for (size_t i = 0; i < scene.simple_models.size(); i++)
		{
			SimpleModel* model = scene.simple_models[i];
			render_probe_model(prl, lights, fog, model, Pass::Alpha, target);
		}

		for (size_t i = 0; i < scene.gltf_models.size(); i++)
		{
			GLTFModel* model = scene.gltf_models[i];
			render_probe_model(prl, lights, fog, model, Pass::Alpha, target);
		}

		oit_resolver->PostDraw(&target);
	}

	if (fog != nullptr)
	{
		fog->updateConstant();
		_render_probe_fog(prl, lights, *fog, target);

		for (size_t i = 0; i < scene.directional_lights.size(); i++)
		{
			DirectionalLight* light = scene.directional_lights[i];
			_render_probe_fog_rm(prl, *light, *fog, target);
		}

		if (lights.probe_grid != nullptr || lights.lod_probe_grid != nullptr)
		{
			_render_probe_fog_rm_env(prl, lights, *fog, target);
		}
	}
}

void BVHRenderer::update_probe_irradiance(const BVHRenderTarget& source, const ProbeRayList& prl, const ProbeGrid& probe_grid, int id_start_probe, float mix_rate, ProbeRenderTarget* target)
{
	bool use_target = target != nullptr;
	int idx_updater = use_target ? 1 : 0;
	if (IrradianceUpdaters[idx_updater] == nullptr)
	{
		IrradianceUpdaters[idx_updater] = std::unique_ptr<IrradianceUpdate>(new IrradianceUpdate(false, use_target));
	}
	IrradianceUpdate::RenderParams params;
	params.id_start_probe = id_start_probe;
	params.mix_rate = mix_rate;
	params.source = &source;
	params.prl = &prl;
	params.probe_grid = &probe_grid;
	params.lod_probe_grid = nullptr;
	params.target = target;
	IrradianceUpdaters[idx_updater]->update(params);
}

void BVHRenderer::update_probe_irradiance(const BVHRenderTarget& source, const ProbeRayList& prl, const LODProbeGrid& probe_grid, int id_start_probe, float mix_rate, ProbeRenderTarget* target)
{
	bool use_target = target != nullptr;
	int idx_updater = use_target ? 3 : 2;
	if (IrradianceUpdaters[idx_updater] == nullptr)
	{
		IrradianceUpdaters[idx_updater] = std::unique_ptr<IrradianceUpdate>(new IrradianceUpdate(true, use_target));
	}
	IrradianceUpdate::RenderParams params;
	params.id_start_probe = id_start_probe;
	params.mix_rate = mix_rate;
	params.source = &source;
	params.prl = &prl;
	params.probe_grid = nullptr;
	params.lod_probe_grid = &probe_grid;
	params.target = target;
	IrradianceUpdaters[idx_updater]->update(params);

}

void BVHRenderer::render_lightmap_depth_primitive(const BVHDepthOnly::RenderParams& params)
{
	const Primitive* prim = params.primitive;
	if (LightmapDepthRenderer == nullptr)
	{
		LightmapDepthRenderer = std::unique_ptr<BVHDepthOnly>(new BVHDepthOnly(2));
	}
	LightmapDepthRenderer->render(params);
}

void BVHRenderer::render_lightmap_depth_model(LightmapRayList& lmrl, SimpleModel* model, BVHRenderTarget& target)
{
	const MeshStandardMaterial* material = &model->material;
	if (material->alphaMode != AlphaMode::Opaque) return;

	BVHDepthOnly::RenderParams params;
	params.material_list = &material;
	params.primitive = &model->geometry;
	params.target = &target;
	params.lmrl = &lmrl;
	render_lightmap_depth_primitive(params);
}

void BVHRenderer::render_lightmap_depth_model(LightmapRayList& lmrl, GLTFModel* model, BVHRenderTarget& target)
{
	std::vector<const MeshStandardMaterial*> material_lst(model->m_materials.size());
	for (size_t i = 0; i < material_lst.size(); i++)
		material_lst[i] = model->m_materials[i].get();

	//for (size_t i = 0; i < model->m_meshs.size(); i++)
	{
		//Mesh& mesh = model->m_meshs[i];
		Mesh& mesh = *model->batched_mesh;

		for (size_t j = 0; j < mesh.primitives.size(); j++)
		{
			Primitive& primitive = mesh.primitives[j];

			const MeshStandardMaterial* material = material_lst[primitive.material_idx];
			if (material->alphaMode != AlphaMode::Opaque) continue;

			BVHDepthOnly::RenderParams params;
			params.material_list = material_lst.data();
			params.primitive = &primitive;
			params.target = &target;
			params.lmrl = &lmrl;
			render_lightmap_depth_primitive(params);
		}
	}
}


BVHRoutine* BVHRenderer::get_lightmap_routine(const BVHRoutine::Options& options)
{
	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(BVHRoutine::Options));
	auto iter = lightmap_routine_map.find(hash);
	if (iter == lightmap_routine_map.end())
	{
		lightmap_routine_map[hash] = std::unique_ptr<BVHRoutine>(new BVHRoutine(options));
	}
	return lightmap_routine_map[hash].get();
}


void BVHRenderer::render_lightmap_primitive(const BVHRoutine::RenderParams& params, Pass pass)
{
	const MeshStandardMaterial* material = params.material_list[params.primitive->material_idx];
	const Lights* lights = params.lights;

	BVHRoutine::Options options;
	options.target_mode = 2;
	options.has_lightmap = params.tex_lightmap != nullptr;
	options.alpha_mode = material->alphaMode;
	options.specular_glossiness = material->specular_glossiness;
	options.has_color = params.primitive->color_buf != nullptr;
	options.has_color_texture = material->tex_idx_map >= 0;
	options.has_metalness_map = material->tex_idx_metalnessMap >= 0;
	options.has_roughness_map = material->tex_idx_roughnessMap >= 0;
	options.has_emissive_map = material->tex_idx_emissiveMap >= 0;
	options.has_specular_map = material->tex_idx_specularMap >= 0;
	options.has_glossiness_map = material->tex_idx_glossinessMap >= 0;
	options.num_directional_lights = lights->num_directional_lights;
	options.num_directional_shadows = lights->num_directional_shadows;
	options.has_lightmap = params.tex_lightmap != nullptr;
	if (!options.has_lightmap)
	{
		options.has_environment_map = lights->environment_map != nullptr;
		options.has_probe_grid = lights->probe_grid != nullptr;
		if (options.has_probe_grid)
		{
			options.probe_reference_recorded = lights->probe_grid->record_references;
		}
		options.has_lod_probe_grid = lights->lod_probe_grid != nullptr;
		options.has_ambient_light = lights->ambient_light != nullptr;
		options.has_hemisphere_light = lights->hemisphere_light != nullptr;
	}
	options.has_fog = params.constant_fog != nullptr;
	BVHRoutine* routine = get_lightmap_routine(options);
	routine->render(params);
}


void BVHRenderer::render_lightmap_model(LightmapRayList& lmrl, const Lights& lights, const Fog* fog, SimpleModel* model, Pass pass, BVHRenderTarget& target)
{
	const GLTexture2D* tex = &model->texture;
	if (model->repl_texture != nullptr)
	{
		tex = model->repl_texture;
	}

	const MeshStandardMaterial* material = &model->material;

	if (pass == Pass::Opaque)
	{
		if (material->alphaMode == AlphaMode::Blend) return;
	}
	else if (pass == Pass::Alpha)
	{
		if (material->alphaMode != AlphaMode::Blend) return;
	}

	BVHRoutine::RenderParams params;
	params.tex_list = &tex;
	params.material_list = &material;
	params.constant_model = &model->m_constant;
	params.primitive = &model->geometry;
	params.lights = &lights;
	params.tex_lightmap = nullptr;	

	if (fog != nullptr)
	{
		params.constant_fog = &fog->m_constant;
	}
	else
	{
		params.constant_fog = nullptr;
	}

	params.target = &target;
	params.lmrl = &lmrl;

	render_lightmap_primitive(params, pass);
}


void BVHRenderer::render_lightmap_model(LightmapRayList& lmrl, const Lights& lights, const Fog* fog, GLTFModel* model, Pass pass, BVHRenderTarget& target)
{
	std::vector<const GLTexture2D*> tex_lst(model->m_textures.size());
	for (size_t i = 0; i < tex_lst.size(); i++)
	{
		auto iter = model->m_repl_textures.find(i);
		if (iter != model->m_repl_textures.end())
		{
			tex_lst[i] = iter->second;
		}
		else
		{
			tex_lst[i] = model->m_textures[i].get();
		}
	}

	std::vector<const MeshStandardMaterial*> material_lst(model->m_materials.size());
	for (size_t i = 0; i < material_lst.size(); i++)
		material_lst[i] = model->m_materials[i].get();

	//for (size_t i = 0; i < model->m_meshs.size(); i++)
	{
		//Mesh& mesh = model->m_meshs[i];
		Mesh& mesh = *model->batched_mesh;

		for (size_t j = 0; j < mesh.primitives.size(); j++)
		{
			Primitive& primitive = mesh.primitives[j];

			const MeshStandardMaterial* material = material_lst[primitive.material_idx];
			if (pass == Pass::Opaque)
			{
				if (material->alphaMode == AlphaMode::Blend) continue;
			}
			else if (pass == Pass::Alpha)
			{
				if (material->alphaMode != AlphaMode::Blend) continue;
			}

			BVHRoutine::RenderParams params;
			params.tex_list = tex_lst.data();
			params.material_list = material_lst.data();
			params.constant_model = mesh.model_constant.get();
			params.primitive = &primitive;
			params.lights = &lights;
			params.tex_lightmap = nullptr;
			if (model->lightmap != nullptr)
			{
				params.tex_lightmap = model->lightmap->lightmap.get();
			}

			if (fog != nullptr)
			{
				params.constant_fog = &fog->m_constant;
			}
			else
			{
				params.constant_fog = nullptr;
			}

			params.target = &target;
			params.lmrl = &lmrl;

			render_lightmap_primitive(params, pass);
		}
	}
}

void BVHRenderer::_render_lightmap_fog(LightmapRayList& lmrl, const Lights& lights, const Fog& fog, BVHRenderTarget& target)
{
	CompDrawFog::Options options;
	options.target_mode = 2;
	options.has_ambient_light = lights.ambient_light != nullptr;
	options.has_hemisphere_light = lights.hemisphere_light != nullptr;
	options.has_environment_map = lights.environment_map != nullptr;

	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(CompDrawFog::Options));
	auto iter = lightmap_fog_draw_map.find(hash);
	if (iter == lightmap_fog_draw_map.end())
	{
		lightmap_fog_draw_map[hash] = std::unique_ptr<CompDrawFog>(new CompDrawFog(options));
	}
	CompDrawFog* fog_draw = lightmap_fog_draw_map[hash].get();

	CompDrawFog::RenderParams params;
	params.constant_fog = &fog.m_constant;
	params.lights = &lights;
	params.target = &target;

	fog_draw->render(params);
}

void BVHRenderer::_render_lightmap_fog_rm(LightmapRayList& lmrl, DirectionalLight& light, const Fog& fog, BVHRenderTarget& target)
{
	if (lightmap_fog_ray_march == nullptr)
	{
		lightmap_fog_ray_march = std::unique_ptr<CompFogRayMarching>(new CompFogRayMarching(2));
	}

	light.updateConstant();
	CompFogRayMarching::RenderParams params;
	params.fog = &fog;
	params.constant_diretional_light = &light.m_constant;
	if (light.shadow != nullptr)
	{
		params.constant_diretional_shadow = &light.shadow->constant_shadow;
		params.tex_shadow = light.shadow->m_lightTex;
	}
	else
	{
		params.constant_diretional_shadow = nullptr;
		params.tex_shadow = -1;
	}
	params.target = &target;
	params.lmrl = &lmrl;
	lightmap_fog_ray_march->render(params);
}

void BVHRenderer::_render_lightmap_fog_rm_env(LightmapRayList& lmrl, const Lights& lights, const Fog& fog, BVHRenderTarget& target)
{
	CompFogRayMarchingEnv::Options options;
	options.target_mode = 2;
	options.has_probe_grid = lights.probe_grid != nullptr;
	if (options.has_probe_grid)
	{
		options.probe_reference_recorded = lights.probe_grid->record_references;
	}
	options.has_lod_probe_grid = lights.lod_probe_grid != nullptr;

	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(CompFogRayMarchingEnv::Options));
	auto iter = lightmap_fog_ray_march_map.find(hash);
	if (iter == lightmap_fog_ray_march_map.end())
	{
		lightmap_fog_ray_march_map[hash] = std::unique_ptr<CompFogRayMarchingEnv>(new CompFogRayMarchingEnv(options));
	}
	CompFogRayMarchingEnv* fog_draw = lightmap_fog_ray_march_map[hash].get();

	CompFogRayMarchingEnv::RenderParams params;
	params.constant_fog = &fog.m_constant;
	params.lights = &lights;
	params.target = &target;
	params.lmrl = &lmrl;

	fog_draw->render(params);
}

void BVHRenderer::render_lightmap(Scene& scene, LightmapRayList& lmrl, BVHRenderTarget& target)
{
	bool has_alpha = false;
	bool has_opaque = false;

	for (size_t i = 0; i < scene.simple_models.size(); i++)
	{
		SimpleModel* model = scene.simple_models[i];
		const MeshStandardMaterial* material = &model->material;
		if (material->alphaMode == AlphaMode::Blend)
		{
			has_alpha = true;
		}
		else
		{
			has_opaque = true;
		}
	}

	for (size_t i = 0; i < scene.gltf_models.size(); i++)
	{
		GLTFModel* model = scene.gltf_models[i];
		size_t num_materials = model->m_materials.size();
		for (size_t i = 0; i < num_materials; i++)
		{
			const MeshStandardMaterial* material = model->m_materials[i].get();
			if (material->alphaMode == AlphaMode::Blend)
			{
				has_alpha = true;
			}
			else
			{
				has_opaque = true;
			}
		}
	}

	while (scene.background != nullptr)
	{
		{
			ColorBackground* bg = dynamic_cast<ColorBackground*>(scene.background);
			if (bg != nullptr)
			{
				glm::vec4 color = { bg->color.r, bg->color.g, bg->color.b, 1.0f };
				glClearTexImage(target.m_tex_video->tex_id, 0, GL_RGBA, GL_FLOAT, &color);
				break;
			}
		}
		{
			CubeBackground* bg = dynamic_cast<CubeBackground*>(scene.background);
			if (bg != nullptr)
			{				
				if (LightmapSkyBoxDraw == nullptr)
				{
					LightmapSkyBoxDraw = std::unique_ptr<CompSkyBox>(new CompSkyBox(2));
				}
				LightmapSkyBoxDraw->render(&lmrl, &bg->cubemap, &target);
				break;
			}
		}
		{
			HemisphereBackground* bg = dynamic_cast<HemisphereBackground*>(scene.background);
			if (bg != nullptr)
			{
				bg->updateConstant();
				if (LightmapHemisphereDraw == nullptr)
				{
					LightmapHemisphereDraw = std::unique_ptr<CompHemisphere>(new CompHemisphere(2));
				}
				LightmapHemisphereDraw->render(&lmrl, &bg->m_constant, &target);
				break;
			}
		}
		{
			BackgroundScene* bg = dynamic_cast<BackgroundScene*>(scene.background);
			if (bg != nullptr && bg->scene != nullptr)
			{
				render_lightmap(*bg->scene, lmrl, target);
			}

		}
		break;
	}

	for (size_t i = 0; i < scene.simple_models.size(); i++)
	{
		SimpleModel* model = scene.simple_models[i];
		check_bvh(model);
	}

	for (size_t i = 0; i < scene.gltf_models.size(); i++)
	{
		GLTFModel* model = scene.gltf_models[i];
		check_bvh(model);
	}

	Lights& lights = scene.lights;


	Fog* fog = nullptr;
	if (scene.fog != nullptr && scene.fog->density > 0)
	{
		fog = scene.fog;
	}

	float max_half = std::numeric_limits<half_float::half>::max();
	glClearTexImage(target.m_tex_depth->tex_id, 0, GL_RED, GL_FLOAT, &max_half);

	if (has_opaque)
	{
		// depth-prepass
		for (size_t i = 0; i < scene.simple_models.size(); i++)
		{
			SimpleModel* model = scene.simple_models[i];
			render_lightmap_depth_model(lmrl, model, target);
		}

		for (size_t i = 0; i < scene.gltf_models.size(); i++)
		{
			GLTFModel* model = scene.gltf_models[i];
			render_lightmap_depth_model(lmrl, model, target);
		}

		// opaque
		for (size_t i = 0; i < scene.simple_models.size(); i++)
		{
			SimpleModel* model = scene.simple_models[i];
			render_lightmap_model(lmrl, lights, fog, model, Pass::Opaque, target);
		}

		for (size_t i = 0; i < scene.gltf_models.size(); i++)
		{
			GLTFModel* model = scene.gltf_models[i];
			render_lightmap_model(lmrl, lights, fog, model, Pass::Opaque, target);
		}
	}

	if (has_alpha)
	{
		target.update_oit_buffers();

		if (oit_resolver == nullptr)
		{
			oit_resolver = std::unique_ptr<CompWeightedOIT>(new CompWeightedOIT);
		}
		oit_resolver->PreDraw(target.m_OITBuffers);

		for (size_t i = 0; i < scene.simple_models.size(); i++)
		{
			SimpleModel* model = scene.simple_models[i];
			render_lightmap_model(lmrl, lights, fog, model, Pass::Alpha, target);
		}

		for (size_t i = 0; i < scene.gltf_models.size(); i++)
		{
			GLTFModel* model = scene.gltf_models[i];
			render_lightmap_model(lmrl, lights, fog, model, Pass::Alpha, target);
		}

		oit_resolver->PostDraw(&target);
	}

	if (fog != nullptr)
	{
		fog->updateConstant();
		_render_lightmap_fog(lmrl, lights, *fog, target);

		for (size_t i = 0; i < scene.directional_lights.size(); i++)
		{
			DirectionalLight* light = scene.directional_lights[i];
			_render_lightmap_fog_rm(lmrl, *light, *fog, target);
		}

		if (lights.probe_grid != nullptr || lights.lod_probe_grid != nullptr)
		{
			_render_lightmap_fog_rm_env(lmrl, lights, *fog, target);
		}
	}
}


void BVHRenderer::update_lightmap(const BVHRenderTarget& source, const LightmapRayList& lmrl, const Lightmap& lightmap, int id_start_texel, float mix_rate)
{
	if (LightmapUpdater == nullptr)
	{
		LightmapUpdater = std::unique_ptr<LightmapUpdate>(new LightmapUpdate);
	}

	LightmapUpdate::RenderParams params;
	params.mix_rate = mix_rate;
	params.source = &source;
	params.lmrl = &lmrl;
	params.target = &lightmap;
	LightmapUpdater->update(params);
}

void BVHRenderer::filter_lightmap(const LightmapRenderTarget& atlas, const Lightmap& lightmap, const glm::mat4& model_mat)
{
	int width = lightmap.width;
	int height = lightmap.height;
	float texel_size = 1.0f / (float)(lightmap.texels_per_unit);
	Lightmap tmp(width, height);

	if (LightmapFiltering == nullptr)
	{
		LightmapFiltering = std::unique_ptr<LightmapFilter>(new LightmapFilter);
	}

	for (int i = 0; i < 4; i++)
	{
		{
			LightmapFilter::RenderParams params;
			params.width = width;
			params.height = height;
			params.texel_size = texel_size;
			params.light_map_in = lightmap.lightmap.get();
			params.light_map_out = tmp.lightmap.get();
			params.atlas_position = atlas.m_tex_position.get();
			params.model_matrix = model_mat;
			LightmapFiltering->filter(params);
		}

		{
			LightmapFilter::RenderParams params;
			params.width = width;
			params.height = height;
			params.texel_size = texel_size;
			params.light_map_in = tmp.lightmap.get();
			params.light_map_out = lightmap.lightmap.get();
			params.atlas_position = atlas.m_tex_position.get();
			params.model_matrix = model_mat;
			LightmapFiltering->filter(params);
		}
	}
}