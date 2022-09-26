#include <GL/glew.h>
#include "crc64/crc64.h"
#include "GLUtils.h"
#include "GLRenderer.h"
#include "GLRenderTarget.h"
#include "CubeRenderTarget.h"
#include "cameras/Camera.h"
#include "cameras/PerspectiveCamera.h"
#include "scenes/Scene.h"
#include "backgrounds/Background.h"
#include "models/ModelComponents.h"
#include "models/SimpleModel.h"
#include "models/GLTFModel.h"
#include "materials/MeshStandardMaterial.h"
#include "lights/DirectionalLight.h"
#include "lights/DirectionalLightShadow.h"
#include "scenes/Fog.h"

//#include <gtx/string_cast.hpp>

GLRenderer::GLRenderer()
{

}

GLRenderer::~GLRenderer()
{

}

void GLRenderer::update_simple_model(SimpleModel* model)
{
	model->updateConstant();
}

void GLRenderer::update_gltf_model(GLTFModel* model)
{
	for (size_t i = 0; i < model->m_meshs.size(); i++)
	{
		Mesh& mesh = model->m_meshs[i];

		if (mesh.needUpdateMorphTargets)
		{
			mesh.buf_weights->upload(mesh.weights.data());
			for (size_t j = 0; j < mesh.primitives.size(); j++)
			{
				Primitive& primitive = mesh.primitives[j];
				bool has_tangent = primitive.geometry[0].tangent_buf != nullptr;
				bool sparse = primitive.none_zero_buf!=nullptr;
				int idx_morpher = (has_tangent ? 1 : 0) + (sparse ? 2 : 0);
				
				if (morphers[idx_morpher] == nullptr)
				{
					morphers[idx_morpher] = std::unique_ptr<MorphUpdate>(new MorphUpdate(has_tangent, sparse));
				}
				MorphUpdate* morpher = morphers[idx_morpher].get();

				MorphUpdate::Params params = {
					mesh.buf_weights.get(),
					&primitive
				};
				morpher->update(params);
			}
			mesh.needUpdateMorphTargets = false;
		}
	}

	if (model->needUpdateSkinnedMeshes)
	{
		for (size_t i = 0; i < model->m_meshs.size(); i++)
		{
			Mesh& mesh = model->m_meshs[i];
			if (mesh.skin_id >= 0)
			{
				Skin& skin = model->m_skins[mesh.skin_id];
				for (size_t j = 0; j < mesh.primitives.size(); j++)
				{
					Primitive& primitive = mesh.primitives[j];
					bool has_tangent = primitive.geometry[0].tangent_buf != nullptr;
					SkinUpdate* skinner = nullptr;
					if (!has_tangent)
					{
						if (skinners[0] == nullptr)
						{
							skinners[0] = std::unique_ptr<SkinUpdate>(new SkinUpdate(false));
						}
						skinner = skinners[0].get();
					}
					else
					{
						if (skinners[1] == nullptr)
						{
							skinners[1] = std::unique_ptr<SkinUpdate>(new SkinUpdate(true));
						}
						skinner = skinners[1].get();
					}

					SkinUpdate::Params params = {
						&skin,
						&primitive
					};
					skinner->update(params);
				}
			}
		}
		model->needUpdateSkinnedMeshes = false;
	}	

	model->updateMeshConstants();
}


StandardRoutine* GLRenderer::get_routine(const StandardRoutine::Options& options)
{
	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(StandardRoutine::Options));
	auto iter = routine_map.find(hash);
	if (iter == routine_map.end())
	{
		routine_map[hash] = std::unique_ptr<StandardRoutine>(new StandardRoutine(options));
	}
	return routine_map[hash].get();
}

inline void toViewAABB(const glm::mat4& MV, const glm::vec3& min_pos, const glm::vec3& max_pos, glm::vec3& min_pos_out, glm::vec3& max_pos_out)
{
	glm::vec4 view_pos[8];
	view_pos[0] = MV * glm::vec4(min_pos.x, min_pos.y, min_pos.z, 1.0f);
	view_pos[1] = MV * glm::vec4(max_pos.x, min_pos.y, min_pos.z, 1.0f);
	view_pos[2] = MV * glm::vec4(min_pos.x, max_pos.y, min_pos.z, 1.0f);
	view_pos[3] = MV * glm::vec4(max_pos.x, max_pos.y, min_pos.z, 1.0f);
	view_pos[4] = MV * glm::vec4(min_pos.x, min_pos.y, max_pos.z, 1.0f);
	view_pos[5] = MV * glm::vec4(max_pos.x, min_pos.y, max_pos.z, 1.0f);
	view_pos[6] = MV * glm::vec4(min_pos.x, max_pos.y, max_pos.z, 1.0f);
	view_pos[7] = MV * glm::vec4(max_pos.x, max_pos.y, max_pos.z, 1.0f);

	min_pos_out = { FLT_MAX, FLT_MAX, FLT_MAX };
	max_pos_out = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

	for (int k = 0; k < 8; k++)
	{
		glm::vec4 pos = view_pos[k];
		if (pos.x < min_pos_out.x) min_pos_out.x = pos.x;
		if (pos.x > max_pos_out.x) max_pos_out.x = pos.x;
		if (pos.y < min_pos_out.y) min_pos_out.y = pos.y;
		if (pos.y > max_pos_out.y) max_pos_out.y = pos.y;
		if (pos.z < min_pos_out.z) min_pos_out.z = pos.z;
		if (pos.z > max_pos_out.z) max_pos_out.z = pos.z;
	}
}

inline bool visible(const glm::mat4& MV, const glm::mat4& P, const glm::vec3& min_pos, const glm::vec3& max_pos)
{
	glm::vec3 min_pos_view, max_pos_view;
	toViewAABB(MV, min_pos, max_pos, min_pos_view, max_pos_view);

	glm::mat4 invP = glm::inverse(P);
	glm::vec4 view_far = invP * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	view_far /= view_far.w;
	glm::vec4 view_near = invP * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
	view_near /= view_near.w;
	
	if (min_pos_view.z > view_near.z) return false;
	if (max_pos_view.z < view_far.z) return false;
	if (min_pos_view.z < view_far.z)
	{
		min_pos_view.z = view_far.z;
	}	

	glm::vec4 min_pos_proj = P * glm::vec4(min_pos_view.x, min_pos_view.y, min_pos_view.z, 1.0f);
	min_pos_proj /= min_pos_proj.w;

	glm::vec4 max_pos_proj = P * glm::vec4(max_pos_view.x, max_pos_view.y, min_pos_view.z, 1.0f);
	max_pos_proj /= max_pos_proj.w;

	return  max_pos_proj.x >= -1.0f && min_pos_proj.x <= 1.0f && max_pos_proj.y >= -1.0f && min_pos_proj.y <= 1.0f;
}

void GLRenderer::render_primitive(const StandardRoutine::RenderParams& params, Pass pass)
{
	const MeshStandardMaterial* material = params.material_list[params.primitive->material_idx];
	const Lights* lights = params.lights;	

	if (pass == Pass::Opaque && material->alphaMode== AlphaMode::Opaque &&  material->tone_shading > 0)
	{
		if (WireDraw == nullptr)
		{
			WireDraw = std::unique_ptr<DrawWire>(new DrawWire);
		}
		DrawWire::RenderParams params2;
		params2.constant_camera = params.constant_camera;
		params2.constant_model = params.constant_model;
		params2.primitive = params.primitive;
		params2.radius = material->wire_width;
		WireDraw->render(params2);		
	}
	
	StandardRoutine::Options options;
	options.alpha_mode = material->alphaMode;
	options.is_highlight_pass = pass == Pass::Highlight;
	options.has_color = params.primitive->color_buf != nullptr;
	options.has_color_texture = material->tex_idx_map >= 0;
	options.has_metalness_map = material->tex_idx_metalnessMap >= 0;
	options.has_roughness_map = material->tex_idx_roughnessMap >= 0;
	options.has_normal_map = material->tex_idx_normalMap >= 0;
	options.has_emissive_map = material->tex_idx_emissiveMap >= 0;
	options.num_directional_lights = lights->num_directional_lights;
	options.num_directional_shadows = lights->num_directional_shadows;
	options.has_environment_map = lights->environment_map != nullptr;
	options.has_ambient_light = lights->ambient_light != nullptr;
	options.has_hemisphere_light = lights->hemisphere_light != nullptr;
	options.has_fog = params.constant_fog != nullptr;
	options.tone_shading = material->tone_shading;
	StandardRoutine* routine = get_routine(options);
	routine->render(params);	
}


void GLRenderer::render_model(Camera* p_camera, const Lights& lights, const Fog* fog, SimpleModel* model, Pass pass)
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
	else if (pass == Pass::Alpha || pass == Pass::Highlight)
	{
		if (material->alphaMode != AlphaMode::Blend) return;
	}

	if (material->tone_shading > 0 && model->geometry.wire_ind_buf==nullptr)
	{
		model->geometry.compute_wires();
	}

	StandardRoutine::RenderParams params;
	params.tex_list = &tex;
	params.material_list = &material;
	params.constant_camera = &p_camera->m_constant;
	params.constant_model = &model->m_constant;
	params.primitive = &model->geometry;
	params.lights = &lights;

	if (fog != nullptr)
	{
		params.constant_fog = &fog->m_constant;
	}
	else
	{
		params.constant_fog = nullptr;
	}

	render_primitive(params, pass);
}

void GLRenderer::render_model(Camera* p_camera, const Lights& lights, const Fog* fog, GLTFModel* model, Pass pass)
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

	for (size_t i = 0; i < model->m_meshs.size(); i++)
	{
		Mesh& mesh = model->m_meshs[i];
		glm::mat4 matrix = model->matrixWorld;
		if (mesh.node_id >= 0 && mesh.skin_id < 0)
		{
			Node& node = model->m_nodes[mesh.node_id];
			matrix *= node.g_trans;
		}
		glm::mat4 MV = p_camera->matrixWorldInverse * matrix;

		for (size_t j = 0; j < mesh.primitives.size(); j++)
		{
			Primitive& primitive = mesh.primitives[j];			
			if (!visible(MV, p_camera->projectionMatrix, primitive.min_pos, primitive.max_pos)) continue;

			const MeshStandardMaterial* material = material_lst[primitive.material_idx];
			if (pass == Pass::Opaque)
			{
				if (material->alphaMode == AlphaMode::Blend) continue;
			}
			else if (pass == Pass::Alpha || pass == Pass::Highlight)
			{
				if (material->alphaMode != AlphaMode::Blend) continue;
			}
			if (material->tone_shading > 0 && primitive.wire_ind_buf==nullptr)
			{
				primitive.compute_wires();
			}
			StandardRoutine::RenderParams params;
			params.tex_list = tex_lst.data();
			params.material_list = material_lst.data();
			params.constant_camera = &p_camera->m_constant;
			params.constant_model = mesh.model_constant.get();
			params.primitive = &primitive;
			params.lights = &lights;

			if (fog != nullptr)
			{
				params.constant_fog = &fog->m_constant;
			}
			else
			{
				params.constant_fog = nullptr;
			}
			render_primitive(params, pass);
		}
	}

	if (pass == Pass::Opaque && model->m_show_skeleton)
	{
		glm::mat4 mat_proj = p_camera->projectionMatrix;
		glm::mat4 mat_camera = p_camera->matrixWorldInverse;
		glm::mat4 mat_model = model->matrixWorld;
		glm::mat4 model_view = mat_camera * mat_model;
		glMatrixLoadfEXT(GL_PROJECTION, (float*)&mat_proj);
		glMatrixLoadfEXT(GL_MODELVIEW, (float*)&model_view);

		glDisable(GL_DEPTH_TEST);
		glColor3f(0.0f, 1.0f, 0.0f);
		glBegin(GL_LINES);

		int num_nodes = (int)model->m_nodes.size();
		for (int i = 0; i < num_nodes; i++)
		{
			Node& node1 = model->m_nodes[i];	
			glm::vec3 p1 = node1.g_trans[3];

			int num_children = node1.children.size();
			for (int j = 0; j < num_children; j++)
			{
				Node& node2 = model->m_nodes[node1.children[j]];
				glm::vec3 p2 = node2.g_trans[3];

				if (p1 != p2)
				{
					glVertex3fv((float*)&p1);
					glVertex3fv((float*)&p2);
				}
			}

		}

		glEnd();
	}
}

DirectionalShadowCast* GLRenderer::get_shadow_caster(const DirectionalShadowCast::Options& options)
{
	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(DirectionalShadowCast::Options));
	auto iter = directional_shadow_caster_map.find(hash);
	if (iter == directional_shadow_caster_map.end())
	{
		directional_shadow_caster_map[hash] = std::unique_ptr<DirectionalShadowCast>(new DirectionalShadowCast(options));
	}
	return directional_shadow_caster_map[hash].get();
}

void GLRenderer::render_shadow_primitive(const DirectionalShadowCast::RenderParams& params)
{		
	const MeshStandardMaterial* material = params.material_list[params.primitive->material_idx];

	DirectionalShadowCast::Options options;
	options.alpha_mode = material->alphaMode;
	options.has_color = params.primitive->color_buf != nullptr;
	options.has_color_texture = material->tex_idx_map >= 0;
	DirectionalShadowCast* shadow_caster = get_shadow_caster(options);
	shadow_caster->render(params);
}

void GLRenderer::render_shadow_model(DirectionalLightShadow* shadow, SimpleModel* model)
{
	glm::mat4 view_matrix = glm::inverse(shadow->m_light->matrixWorld);
	if (!visible(view_matrix * model->matrixWorld, shadow->m_light_proj_matrix, model->geometry.min_pos, model->geometry.max_pos)) return;

	const GLTexture2D* tex = &model->texture;
	if (model->repl_texture != nullptr)
	{
		tex = model->repl_texture;
	}

	const MeshStandardMaterial* material = &model->material;

	DirectionalShadowCast::RenderParams params;
	params.tex_list = &tex;
	params.material_list = &material;
	params.constant_shadow = &shadow->constant_shadow;
	params.constant_model = &model->m_constant;
	params.primitive = &model->geometry;
	render_shadow_primitive(params);
}

void GLRenderer::render_shadow_model(DirectionalLightShadow* shadow, GLTFModel* model)
{
	glm::mat4 view_matrix = glm::inverse(shadow->m_light->matrixWorld);
	
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

	for (size_t i = 0; i < model->m_meshs.size(); i++)
	{
		Mesh& mesh = model->m_meshs[i];
		glm::mat4 matrix = model->matrixWorld;
		if (mesh.node_id >= 0 && mesh.skin_id < 0)
		{
			Node& node = model->m_nodes[mesh.node_id];
			matrix *= node.g_trans;
		}
		glm::mat4 MV = view_matrix * matrix;

		for (size_t j = 0; j < mesh.primitives.size(); j++)
		{
			Primitive& primitive = mesh.primitives[j];
			if (!visible(MV, shadow->m_light_proj_matrix, primitive.min_pos, primitive.max_pos)) continue;

			const MeshStandardMaterial* material = material_lst[primitive.material_idx];

			DirectionalShadowCast::RenderParams params;
			params.tex_list = tex_lst.data();
			params.material_list = material_lst.data();
			params.constant_shadow = &shadow->constant_shadow;
			params.constant_model = mesh.model_constant.get();
			params.primitive = &primitive;		
			render_shadow_primitive(params);
		}
	}
}

void GLRenderer::_render_fog(const Camera& camera, const Lights& lights, const Fog& fog, GLRenderTarget& target)
{
	DrawFog::Options options;
	options.msaa = target.msaa();
	options.has_ambient_light = lights.ambient_light != nullptr;
	options.has_hemisphere_light = lights.hemisphere_light != nullptr;
	options.has_environment_map = lights.environment_map != nullptr;

	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(DrawFog::Options));
	auto iter = fog_draw_map.find(hash);
	if (iter == fog_draw_map.end())
	{
		fog_draw_map[hash] = std::unique_ptr<DrawFog>(new DrawFog(options));
	}
	DrawFog* fog_draw = fog_draw_map[hash].get();

	DrawFog::RenderParams params;
	params.tex_depth = target.m_tex_depth.get();
	params.constant_camera = &camera.m_constant;
	params.constant_fog = &fog.m_constant;
	params.lights = &lights;

	fog_draw->render(params);
}

void GLRenderer::_render_fog_rm(const Camera& camera, DirectionalLight& light, const Fog& fog, GLRenderTarget& target)
{
	bool msaa = target.msaa();
	int idx = msaa ? 1 : 0;
	if (fog_ray_march[idx] == nullptr)
	{
		fog_ray_march[idx] = std::unique_ptr<FogRayMarching>(new FogRayMarching(msaa));
	}
	FogRayMarching* fog_rm = fog_ray_march[idx].get();

	light.updateConstant();
	FogRayMarching::RenderParams params;
	params.tex_depth = target.m_tex_depth.get();
	params.constant_camera = &camera.m_constant;
	params.constant_fog = &fog.m_constant;
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
	fog_rm->render(params);
}


void GLRenderer::_pre_render(Scene& scene, PreRender& pre)
{
	auto* p_pre = &pre;
	scene.traverse([p_pre](Object3D* obj) {
		do
		{
			{
				SimpleModel* model = dynamic_cast<SimpleModel*>(obj);
				if (model)
				{
					p_pre->simple_models.push_back(model);
					break;
				}
			}
			{
				GLTFModel* model = dynamic_cast<GLTFModel*>(obj);
				if (model)
				{
					p_pre->gltf_models.push_back(model);
					break;
				}
			}
			{
				DirectionalLight* light = dynamic_cast<DirectionalLight*>(obj);
				if (light)
				{
					glm::vec3 pos_target = { 0.0f, 0.0f, 0.0f };
					if (light->target != nullptr)
					{
						pos_target = light->target->matrixWorld[3];
					}
					light->lookAt(pos_target);
					p_pre->directional_lights.push_back(light);
					break;
				}
			}
		} while (false);

		obj->updateWorldMatrix(false, false);
	});

	// update models
	for (size_t i = 0; i < pre.simple_models.size(); i++)
	{
		SimpleModel* model = pre.simple_models[i];
		update_simple_model(model);

		const MeshStandardMaterial* material = &model->material;
		if (material->alphaMode == AlphaMode::Blend)
		{
			scene.has_alpha = true;
		}
		else
		{
			scene.has_opaque = true;
		}
	}

	for (size_t i = 0; i < pre.gltf_models.size(); i++)
	{
		GLTFModel* model = pre.gltf_models[i];
		update_gltf_model(model);
		size_t num_materials = model->m_materials.size();
		for (size_t i = 0; i < num_materials; i++)
		{
			const MeshStandardMaterial* material = model->m_materials[i].get();
			if (material->alphaMode == AlphaMode::Blend)
			{
				scene.has_alpha = true;
			}
			else
			{
				scene.has_opaque = true;
			}
		}
	}

	// update lights
	for (size_t i = 0; i < pre.directional_lights.size(); i++)
	{
		DirectionalLight* light = pre.directional_lights[i];
		if (light->shadow != nullptr)
		{
			light->shadow->updateMatrices();
			glBindFramebuffer(GL_FRAMEBUFFER, light->shadow->m_lightFBO);
			glViewport(0, 0, light->shadow->m_map_width, light->shadow->m_map_height);
			const float one = 1.0f;
			glDepthMask(GL_TRUE);
			glClearBufferfv(GL_DEPTH, 0, &one);

			for (size_t j = 0; j < pre.simple_models.size(); j++)
			{
				SimpleModel* model = pre.simple_models[j];
				render_shadow_model(light->shadow.get(), model);
			}

			for (size_t j = 0; j < pre.gltf_models.size(); j++)
			{
				GLTFModel* model = pre.gltf_models[j];
				render_shadow_model(light->shadow.get(), model);
			}
		}
	}


	// update light constants
	Lights& lights = scene.lights;
	lights.directional_shadow_texs.clear();

	std::vector<ConstDirectionalLight> const_directional_lights(pre.directional_lights.size());
	std::vector<ConstDirectionalShadow> const_directional_shadows;
	for (size_t i = 0; i < pre.directional_lights.size(); i++)
	{
		DirectionalLight* light = pre.directional_lights[i];
		ConstDirectionalLight& const_light = const_directional_lights[i];
		light->makeConst(const_light);

		if (light->shadow != nullptr)
		{
			lights.directional_shadow_texs.push_back(light->shadow->m_lightTex);
			ConstDirectionalShadow constShadow;
			light->shadow->makeConst(constShadow);
			const_directional_shadows.push_back(std::move(constShadow));
		}
	}

	{
		if (lights.num_directional_lights != (int)const_directional_lights.size())
		{
			lights.num_directional_lights = (int)const_directional_lights.size();
			lights.constant_directional_lights = nullptr;
			if (lights.num_directional_lights > 0)
			{
				lights.constant_directional_lights = std::unique_ptr<GLDynBuffer>(new GLDynBuffer(const_directional_lights.size() * sizeof(ConstDirectionalLight), GL_UNIFORM_BUFFER));
			}
			lights.hash_directional_lights = 0;

		}
		if (lights.num_directional_lights > 0)
		{
			uint64_t hash = crc64(0, (unsigned char*)const_directional_lights.data(), const_directional_lights.size() * sizeof(ConstDirectionalLight));
			if (hash != lights.hash_directional_lights)
			{
				lights.hash_directional_lights = hash;
				lights.constant_directional_lights->upload(const_directional_lights.data());
			}
		}
	}

	{
		if (lights.num_directional_shadows != (int)const_directional_shadows.size())
		{
			lights.num_directional_shadows = (int)const_directional_shadows.size();
			lights.constant_directional_shadows = nullptr;
			if (lights.num_directional_shadows > 0)
			{
				lights.constant_directional_shadows = std::unique_ptr<GLDynBuffer>(new GLDynBuffer(const_directional_shadows.size() * sizeof(ConstDirectionalShadow), GL_UNIFORM_BUFFER));
			}
			lights.hash_directional_shadows = 0;
		}
		if (lights.num_directional_shadows > 0)
		{
			uint64_t hash = crc64(0, (unsigned char*)const_directional_shadows.data(), const_directional_shadows.size() * sizeof(ConstDirectionalShadow));
			if (hash != lights.hash_directional_shadows)
			{
				lights.hash_directional_shadows = hash;
				lights.constant_directional_shadows->upload(const_directional_shadows.data());
			}
		}
	}

	lights.environment_map = nullptr;
	lights.ambient_light = nullptr;
	lights.hemisphere_light = nullptr;

	while (scene.indirectLight)
	{
		{
			EnvironmentMap* envMap = dynamic_cast<EnvironmentMap*>(scene.indirectLight);
			if (envMap != nullptr)
			{
				envMap->updateConstant();
				lights.environment_map = envMap;
				break;
			}
		}

		{
			AmbientLight* ambientLight = dynamic_cast<AmbientLight*>(scene.indirectLight);
			if (ambientLight != nullptr)
			{
				ambientLight->updateConstant();
				lights.ambient_light = ambientLight;
				break;
			}
		}

		{
			HemisphereLight* hemisphereLight = dynamic_cast<HemisphereLight*>(scene.indirectLight);
			if (hemisphereLight != nullptr)
			{
				hemisphereLight->updateConstant();
				lights.hemisphere_light = hemisphereLight;
				break;
			}
		}
		break;
	}
}

void GLRenderer::_render_scene(Scene& scene, Camera& camera, GLRenderTarget& target, PreRender& pre)
{
	camera.updateMatrixWorld(false);
	camera.updateConstant();

	// model culling
	for (size_t i = 0; i < pre.simple_models.size(); i++)
	{
		SimpleModel* model = pre.simple_models[i];
		if (!visible(camera.matrixWorldInverse * model->matrixWorld, camera.projectionMatrix, model->geometry.min_pos, model->geometry.max_pos))
		{
			pre.simple_models.erase(pre.simple_models.begin() + i);
			i--;
		}
	}

	for (size_t i = 0; i < pre.gltf_models.size(); i++)
	{
		GLTFModel* model = pre.gltf_models[i];
		if (!visible(camera.matrixWorldInverse * model->matrixWorld, camera.projectionMatrix, model->m_min_pos, model->m_max_pos))
		{
			pre.gltf_models.erase(pre.gltf_models.begin() + i);
			i--;
		}
	}

	// render scene
	target.bind_buffer();
	glEnable(GL_FRAMEBUFFER_SRGB);
	glViewport(0, 0, target.m_width, target.m_height);

	while (scene.background != nullptr)
	{
		{
			ColorBackground* bg = dynamic_cast<ColorBackground*>(scene.background);
			if (bg != nullptr)
			{
				glClearColor(bg->color.r, bg->color.g, bg->color.b, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				break;
			}
		}
		{
			CubeBackground* bg = dynamic_cast<CubeBackground*>(scene.background);
			if (bg != nullptr)
			{
				if (SkyBoxDraw == nullptr)
				{
					SkyBoxDraw = std::unique_ptr<DrawSkyBox>(new DrawSkyBox);
				}
				SkyBoxDraw->render(&camera.m_constant, &bg->cubemap);
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
					HemisphereDraw = std::unique_ptr<DrawHemisphere>(new DrawHemisphere);
				}
				HemisphereDraw->render(&camera.m_constant, &bg->m_constant);
			}
		}
		break;
	}

	Lights& lights = scene.lights;

	Fog* fog = nullptr;
	if (scene.fog != nullptr && scene.fog->density > 0)
	{
		fog = scene.fog;
	}

	glDepthMask(GL_TRUE);
	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);

	if (scene.has_opaque)
	{
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);

		for (size_t i = 0; i < pre.simple_models.size(); i++)
		{
			SimpleModel* model = pre.simple_models[i];
			render_model(&camera, lights, fog, model, Pass::Opaque);
		}

		for (size_t i = 0; i < pre.gltf_models.size(); i++)
		{
			GLTFModel* model = pre.gltf_models[i];
			render_model(&camera, lights, fog, model, Pass::Opaque);
		}
	}

	if (scene.has_alpha)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glDepthMask(GL_FALSE);

		for (size_t i = 0; i < pre.simple_models.size(); i++)
		{
			SimpleModel* model = pre.simple_models[i];
			render_model(&camera, lights, fog, model, Pass::Highlight);
		}

		for (size_t i = 0; i < pre.gltf_models.size(); i++)
		{
			GLTFModel* model = pre.gltf_models[i];
			render_model(&camera, lights, fog, model, Pass::Highlight);
		}

		target.update_oit_buffers();
		if (!target.msaa())
		{
			if (oit_resolvers[0] == nullptr)
			{
				oit_resolvers[0] = std::unique_ptr<WeightedOIT>(new WeightedOIT(false));
			}
			oit_resolvers[0]->PreDraw(target.m_OITBuffers);
		}
		else
		{
			if (oit_resolvers[1] == nullptr)
			{
				oit_resolvers[1] = std::unique_ptr<WeightedOIT>(new WeightedOIT(true));
			}
			oit_resolvers[1]->PreDraw(target.m_OITBuffers);
		}

		for (size_t i = 0; i < pre.simple_models.size(); i++)
		{
			SimpleModel* model = pre.simple_models[i];
			render_model(&camera, lights, fog, model, Pass::Alpha);
		}

		for (size_t i = 0; i < pre.gltf_models.size(); i++)
		{
			GLTFModel* model = pre.gltf_models[i];
			render_model(&camera, lights, fog, model, Pass::Alpha);
		}

		target.bind_buffer();

		if (!target.msaa())
		{
			oit_resolvers[0]->PostDraw(target.m_OITBuffers);
		}
		else
		{
			oit_resolvers[1]->PostDraw(target.m_OITBuffers);
		}
	}

}

void GLRenderer::_render(Scene& scene, Camera& camera, GLRenderTarget& target, PreRender& pre)
{
	_render_scene(scene, camera, target, pre);

	Lights& lights = scene.lights;

	Fog* fog = nullptr;
	if (scene.fog != nullptr && scene.fog->density > 0)
	{
		fog = scene.fog;
	}

	if (target.msaa())
	{
		target.resolve_msaa();
	}

	if (fog!=nullptr)
	{
		fog->updateConstant();

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);

		_render_fog(camera, lights, *fog, target);

		for (size_t i = 0; i < pre.directional_lights.size(); i++)
		{
			DirectionalLight* light = pre.directional_lights[i];
			_render_fog_rm(camera, *light, *fog, target);
		}
	}
}

void GLRenderer::_render_scene_to_cube(Scene& scene, CubeRenderTarget& target, glm::vec3& position, float zNear, float zFar, const PreRender& pre)
{
	{
		PreRender _pre = pre;
		PerspectiveCamera camera(90.0f, 1.0f, zNear, zFar);
		camera.position = position;
		camera.up = { 0.0f, -1.0f, 0.0f };
		camera.lookAt(position + glm::vec3(1.0f, 0.0f, 0.0f));
		_render_scene(scene, camera, *target.m_faces[0], _pre);
	}

	{
		PreRender _pre = pre;
		PerspectiveCamera camera(90.0f, 1.0f, zNear, zFar);
		camera.position = position;
		camera.up = { 0.0f, -1.0f, 0.0f };
		camera.lookAt(position + glm::vec3(-1.0f, 0.0f, 0.0f));
		_render_scene(scene, camera, *target.m_faces[1], _pre);
	}

	{
		PreRender _pre = pre;
		PerspectiveCamera camera(90.0f, 1.0f, zNear, zFar);
		camera.position = position;
		camera.up = { 0.0f, 0.0f, 1.0f };
		camera.lookAt(position + glm::vec3(0.0f, 1.0f, 0.0f));
		_render_scene(scene, camera, *target.m_faces[2], _pre);
	}

	{
		PreRender _pre = pre;
		PerspectiveCamera camera(90.0f, 1.0f, zNear, zFar);
		camera.position = position;
		camera.up = { 0.0f, 0.0f, -1.0f };
		camera.lookAt(position + glm::vec3(0.0f, -1.0f, 0.0f));
		_render_scene(scene, camera, *target.m_faces[3], _pre);
	}

	{
		PreRender _pre = pre;
		PerspectiveCamera camera(90.0f, 1.0f, zNear, zFar);
		camera.position = position;
		camera.up = { 0.0f, -1.0f, 0.0f };
		camera.lookAt(position + glm::vec3(0.0f, 0.0f, 1.0f));
		_render_scene(scene, camera, *target.m_faces[4], _pre);
	}

	{
		PreRender _pre = pre;
		PerspectiveCamera camera(90.0f, 1.0f, zNear, zFar);
		camera.position = position;
		camera.up = { 0.0f, -1.0f, 0.0f };
		camera.lookAt(position + glm::vec3(0.0f, 0.0f, -1.0f));
		_render_scene(scene, camera, *target.m_faces[5], _pre);
	}
}

void GLRenderer::_render_cube(Scene& scene, CubeRenderTarget& target, glm::vec3& position, float zNear, float zFar, const PreRender& pre)
{
	{
		PreRender _pre = pre;
		PerspectiveCamera camera(90.0f, 1.0f, zNear, zFar);
		camera.position = position;
		camera.up = { 0.0f, -1.0f, 0.0f };
		camera.lookAt(position + glm::vec3(1.0f, 0.0f, 0.0f));
		_render(scene, camera, *target.m_faces[0], _pre);
	}

	{
		PreRender _pre = pre;
		PerspectiveCamera camera(90.0f, 1.0f, zNear, zFar);
		camera.position = position;
		camera.up = { 0.0f, -1.0f, 0.0f };
		camera.lookAt(position + glm::vec3(-1.0f, 0.0f, 0.0f));
		_render(scene, camera, *target.m_faces[1], _pre);
	}

	{
		PreRender _pre = pre;
		PerspectiveCamera camera(90.0f, 1.0f, zNear, zFar);
		camera.position = position;
		camera.up = { 0.0f, 0.0f, 1.0f };
		camera.lookAt(position + glm::vec3(0.0f, 1.0f, 0.0f));
		_render(scene, camera, *target.m_faces[2], _pre);
	}

	{
		PreRender _pre = pre;
		PerspectiveCamera camera(90.0f, 1.0f, zNear, zFar);
		camera.position = position;
		camera.up = { 0.0f, 0.0f, -1.0f };
		camera.lookAt(position + glm::vec3(0.0f, -1.0f, 0.0f));
		_render(scene, camera, *target.m_faces[3], _pre);
	}

	{
		PreRender _pre = pre;
		PerspectiveCamera camera(90.0f, 1.0f, zNear, zFar);
		camera.position = position;
		camera.up = { 0.0f, -1.0f, 0.0f };
		camera.lookAt(position + glm::vec3(0.0f, 0.0f, 1.0f));
		_render(scene, camera, *target.m_faces[4], _pre);
	}

	{
		PreRender _pre = pre;
		PerspectiveCamera camera(90.0f, 1.0f, zNear, zFar);
		camera.position = position;
		camera.up = { 0.0f, -1.0f, 0.0f };
		camera.lookAt(position + glm::vec3(0.0f, 0.0f, -1.0f));
		_render(scene, camera, *target.m_faces[5], _pre);
	}
}

void GLRenderer::render(Scene& scene, Camera& camera, GLRenderTarget& target)
{
	PreRender pre;
	_pre_render(scene, pre);
	_render(scene, camera, target, pre);	
}

void GLRenderer::renderCube(Scene& scene, CubeRenderTarget& target, glm::vec3& position, float zNear, float zFar)
{
	PreRender pre;
	_pre_render(scene, pre);
	_render_cube(scene, target, position, zNear, zFar, pre);
}


void GLRenderer::renderLayers(size_t num_layers, Layer* layers, GLRenderTarget& target)
{
	for (size_t i = 0; i < num_layers; i++)
	{
		Layer& layer = layers[i];
		_pre_render(*layer.scene, layer.pre);
		if (i < num_layers - 1)
		{
			_render_scene(*layer.scene, *layer.camera, target, layer.pre);
		}
		else
		{
			_render(*layer.scene, *layer.camera, target, layer.pre);
		}
	}
}

void GLRenderer::renderLayersToCube(size_t num_layers, CubeLayer* layers, CubeRenderTarget& target)
{
	for (size_t i = 0; i < num_layers; i++)
	{
		CubeLayer& layer = layers[i];
		_pre_render(*layer.scene, layer.pre);
		if (i < num_layers - 1)
		{
			_render_scene_to_cube(*layer.scene, target, layer.position, layer.zNear, layer.zFar, layer.pre);
		}
		else
		{
			_render_cube(*layer.scene, target, layer.position, layer.zNear, layer.zFar, layer.pre);
		}
	}

}



void GLRenderer::render_primitive_base(const BaseColorRoutine::RenderParams& params)
{
	const MeshStandardMaterial* material = params.material_list[params.primitive->material_idx];	

	if (material->tone_shading > 0)
	{
		if (WireDraw == nullptr)
		{
			WireDraw = std::unique_ptr<DrawWire>(new DrawWire);
		}
		DrawWire::RenderParams params2;
		params2.constant_camera = params.constant_camera;
		params2.constant_model = params.constant_model;
		params2.primitive = params.primitive;
		params2.radius = material->wire_width;
		WireDraw->render(params2);
	}

	BaseColorRoutine::Options options;	
	options.alpha_mode = material->alphaMode;
	options.has_color = params.primitive->color_buf != nullptr;
	options.has_color_texture = material->tex_idx_map >= 0;	

	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(BaseColorRoutine::Options));
	auto iter = base_routine_map.find(hash);
	if (iter == base_routine_map.end())
	{
		base_routine_map[hash] = std::unique_ptr<BaseColorRoutine>(new BaseColorRoutine(options));
	}
	BaseColorRoutine* routine = base_routine_map[hash].get();
	routine->render(params);

}

void GLRenderer::render_model_base(Camera* p_camera, SimpleModel* model)
{
	const GLTexture2D* tex = &model->texture;
	if (model->repl_texture != nullptr)
	{
		tex = model->repl_texture;
	}

	const MeshStandardMaterial* material = &model->material;

	if (material->alphaMode == AlphaMode::Blend) return;

	if (material->tone_shading > 0 && model->geometry.wire_ind_buf == nullptr)
	{
		model->geometry.compute_wires();
	}

	BaseColorRoutine::RenderParams params;
	params.tex_list = &tex;
	params.material_list = &material;
	params.constant_camera = &p_camera->m_constant;
	params.constant_model = &model->m_constant;
	params.primitive = &model->geometry;	
	
	render_primitive_base(params);
}

void GLRenderer::render_model_base(Camera* p_camera, GLTFModel* model)
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

	for (size_t i = 0; i < model->m_meshs.size(); i++)
	{
		Mesh& mesh = model->m_meshs[i];
		glm::mat4 matrix = model->matrixWorld;
		if (mesh.node_id >= 0 && mesh.skin_id < 0)
		{
			Node& node = model->m_nodes[mesh.node_id];
			matrix *= node.g_trans;
		}
		glm::mat4 MV = p_camera->matrixWorldInverse * matrix;

		for (size_t j = 0; j < mesh.primitives.size(); j++)
		{
			Primitive& primitive = mesh.primitives[j];
			if (!visible(MV, p_camera->projectionMatrix, primitive.min_pos, primitive.max_pos)) continue;

			const MeshStandardMaterial* material = material_lst[primitive.material_idx];
			if (material->alphaMode == AlphaMode::Blend) continue;			
			if (material->tone_shading > 0 && primitive.wire_ind_buf == nullptr)
			{
				primitive.compute_wires();
			}
			BaseColorRoutine::RenderParams params;
			params.tex_list = tex_lst.data();
			params.material_list = material_lst.data();
			params.constant_camera = &p_camera->m_constant;
			params.constant_model = mesh.model_constant.get();
			params.primitive = &primitive;
			
			render_primitive_base(params);
		}
	}
}


void GLRenderer::render_primitive_lighting(const LightingRoutine::RenderParams& params)
{
	const MeshStandardMaterial* material = params.material_list[params.primitive->material_idx];
	const Lights* lights = params.lights;

	LightingRoutine::Options options;
	options.alpha_mode = material->alphaMode;
	options.has_color = params.primitive->color_buf != nullptr;
	options.has_color_texture = material->tex_idx_map >= 0;
	options.has_metalness_map = material->tex_idx_metalnessMap >= 0;
	options.has_roughness_map = material->tex_idx_roughnessMap >= 0;
	options.has_normal_map = material->tex_idx_normalMap >= 0;
	options.num_directional_lights = lights->num_directional_lights;
	options.num_directional_shadows = lights->num_directional_shadows;
	options.has_environment_map = lights->environment_map != nullptr;
	options.has_ambient_light = lights->ambient_light != nullptr;
	options.has_hemisphere_light = lights->hemisphere_light != nullptr;
	options.has_fog = params.constant_fog != nullptr;
	options.tone_shading = material->tone_shading;

	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(LightingRoutine::Options));
	auto iter = lighting_routine_map.find(hash);
	if (iter == lighting_routine_map.end())
	{
		lighting_routine_map[hash] = std::unique_ptr<LightingRoutine>(new LightingRoutine(options));
	}

	LightingRoutine* routine = lighting_routine_map[hash].get();
	routine->render(params);
}

void GLRenderer::render_model_lighting(Camera* p_camera, const Lights& lights, const Fog* fog, SimpleModel* model)
{
	const GLTexture2D* tex = &model->texture;
	if (model->repl_texture != nullptr)
	{
		tex = model->repl_texture;
	}

	const MeshStandardMaterial* material = &model->material;

	if (material->alphaMode == AlphaMode::Blend) return;

	LightingRoutine::RenderParams params;
	params.tex_list = &tex;
	params.material_list = &material;
	params.constant_camera = &p_camera->m_constant;
	params.constant_model = &model->m_constant;
	params.primitive = &model->geometry;
	params.lights = &lights;

	if (fog != nullptr)
	{
		params.constant_fog = &fog->m_constant;
	}
	else
	{
		params.constant_fog = nullptr;
	}

	render_primitive_lighting(params);
}


void GLRenderer::render_model_lighting(Camera* p_camera, const Lights& lights, const Fog* fog, GLTFModel* model)
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

	for (size_t i = 0; i < model->m_meshs.size(); i++)
	{
		Mesh& mesh = model->m_meshs[i];
		glm::mat4 matrix = model->matrixWorld;
		if (mesh.node_id >= 0 && mesh.skin_id < 0)
		{
			Node& node = model->m_nodes[mesh.node_id];
			matrix *= node.g_trans;
		}
		glm::mat4 MV = p_camera->matrixWorldInverse * matrix;

		for (size_t j = 0; j < mesh.primitives.size(); j++)
		{
			Primitive& primitive = mesh.primitives[j];
			if (!visible(MV, p_camera->projectionMatrix, primitive.min_pos, primitive.max_pos)) continue;

			const MeshStandardMaterial* material = material_lst[primitive.material_idx];
			if (material->alphaMode == AlphaMode::Blend) continue;

			LightingRoutine::RenderParams params;
			params.tex_list = tex_lst.data();
			params.material_list = material_lst.data();
			params.constant_camera = &p_camera->m_constant;
			params.constant_model = mesh.model_constant.get();
			params.primitive = &primitive;
			params.lights = &lights;

			if (fog != nullptr)
			{
				params.constant_fog = &fog->m_constant;
			}
			else
			{
				params.constant_fog = nullptr;
			}
			render_primitive_lighting(params);
		}
	}

}

void GLRenderer::renderCelluloid(Scene& scene, Camera& camera, GLRenderTarget* layer_base, GLRenderTarget* layer_light, GLRenderTarget* layer_alpha)
{
	PreRender pre;
	_pre_render(scene, pre);

	camera.updateMatrixWorld(false);
	camera.updateConstant();

	// model culling
	for (size_t i = 0; i < pre.simple_models.size(); i++)
	{
		SimpleModel* model = pre.simple_models[i];
		if (!visible(camera.matrixWorldInverse * model->matrixWorld, camera.projectionMatrix, model->geometry.min_pos, model->geometry.max_pos))
		{
			pre.simple_models.erase(pre.simple_models.begin() + i);
			i--;
		}
	}

	for (size_t i = 0; i < pre.gltf_models.size(); i++)
	{
		GLTFModel* model = pre.gltf_models[i];
		if (!visible(camera.matrixWorldInverse * model->matrixWorld, camera.projectionMatrix, model->m_min_pos, model->m_max_pos))
		{
			pre.gltf_models.erase(pre.gltf_models.begin() + i);
			i--;
		}
	}

	// render base-color
	if (layer_base != nullptr)
	{
		layer_base->bind_buffer();
		glEnable(GL_FRAMEBUFFER_SRGB);
		glViewport(0, 0, layer_base->m_width, layer_base->m_height);

		while (scene.background != nullptr)
		{
			{
				ColorBackground* bg = dynamic_cast<ColorBackground*>(scene.background);
				if (bg != nullptr)
				{
					glClearColor(bg->color.r, bg->color.g, bg->color.b, 1.0f);
					glClear(GL_COLOR_BUFFER_BIT);
					break;
				}
			}
			{
				CubeBackground* bg = dynamic_cast<CubeBackground*>(scene.background);
				if (bg != nullptr)
				{
					if (SkyBoxDraw == nullptr)
					{
						SkyBoxDraw = std::unique_ptr<DrawSkyBox>(new DrawSkyBox);
					}
					SkyBoxDraw->render(&camera.m_constant, &bg->cubemap);
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
						HemisphereDraw = std::unique_ptr<DrawHemisphere>(new DrawHemisphere);
					}
					HemisphereDraw->render(&camera.m_constant, &bg->m_constant);
				}
			}
			break;
		}
	
		glDepthMask(GL_TRUE);
		glClearDepth(1.0f);
		glClear(GL_DEPTH_BUFFER_BIT);

		glDisable(GL_BLEND);		

		for (size_t i = 0; i < pre.simple_models.size(); i++)
		{
			SimpleModel* model = pre.simple_models[i];
			render_model_base(&camera, model);
		}

		for (size_t i = 0; i < pre.gltf_models.size(); i++)
		{
			GLTFModel* model = pre.gltf_models[i];
			render_model_base(&camera,  model);
		}

		if (layer_base->msaa())
		{
			layer_base->resolve_msaa();
		}
	}

	Lights& lights = scene.lights;

	Fog* fog = nullptr;
	if (scene.fog != nullptr && scene.fog->density > 0)
	{
		fog = scene.fog;
	}

	// render lighting
	if (layer_light != nullptr)
	{	
		layer_light->bind_buffer();
		glEnable(GL_FRAMEBUFFER_SRGB);
		glViewport(0, 0, layer_light->m_width, layer_light->m_height);

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glDepthMask(GL_TRUE);
		glClearDepth(1.0f);
		glClear(GL_DEPTH_BUFFER_BIT);

		glDisable(GL_BLEND);

		for (size_t i = 0; i < pre.simple_models.size(); i++)
		{
			SimpleModel* model = pre.simple_models[i];
			render_model_lighting(&camera, lights, fog, model);
		}

		for (size_t i = 0; i < pre.gltf_models.size(); i++)
		{
			GLTFModel* model = pre.gltf_models[i];
			render_model_lighting(&camera, lights, fog, model);
		}

		if (layer_light->msaa())
		{
			layer_light->resolve_msaa();
		}
	}

	// render_alpha
	if (layer_alpha != nullptr)
	{
		GLRenderTarget t_alpha(false, true);
		t_alpha.bind_buffer();
		t_alpha.update_framebuffers(layer_alpha->m_width, layer_alpha->m_height);

		glEnable(GL_FRAMEBUFFER_SRGB);
		glViewport(0, 0, t_alpha.m_width, t_alpha.m_height);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glDepthMask(GL_TRUE);
		glClearDepth(1.0f);
		glClear(GL_DEPTH_BUFFER_BIT);

		glDisable(GL_BLEND);

		// depth pass
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		for (size_t i = 0; i < pre.simple_models.size(); i++)
		{
			SimpleModel* model = pre.simple_models[i];
			render_model_base(&camera, model);
		}

		for (size_t i = 0; i < pre.gltf_models.size(); i++)
		{
			GLTFModel* model = pre.gltf_models[i];
			render_model_base(&camera, model);
		}

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		// alpha pass 
		t_alpha.update_oit_buffers();
		if (oit_resolvers[1] == nullptr)
		{
			oit_resolvers[1] = std::unique_ptr<WeightedOIT>(new WeightedOIT(true));
		}
		oit_resolvers[1]->PreDraw(t_alpha.m_OITBuffers);
		

		for (size_t i = 0; i < pre.simple_models.size(); i++)
		{
			SimpleModel* model = pre.simple_models[i];
			render_model(&camera, lights, fog, model, Pass::Alpha);
		}

		for (size_t i = 0; i < pre.gltf_models.size(); i++)
		{
			GLTFModel* model = pre.gltf_models[i];
			render_model(&camera, lights, fog, model, Pass::Alpha);
		}

		t_alpha.bind_buffer();
		oit_resolvers[1]->PostDraw(t_alpha.m_OITBuffers);
		t_alpha.resolve_msaa();

		// fog pass
		if (fog != nullptr)
		{
			fog->updateConstant();

			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_DEPTH_TEST);

			_render_fog(camera, lights, *fog, t_alpha);

			for (size_t i = 0; i < pre.directional_lights.size(); i++)
			{
				DirectionalLight* light = pre.directional_lights[i];
				_render_fog_rm(camera, *light, *fog, t_alpha);
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, layer_alpha->m_fbo_video);
		if (alpha_demodulate == nullptr)
		{
			alpha_demodulate = std::unique_ptr<AlphaDem>(new AlphaDem);
		}
		alpha_demodulate->render(t_alpha.m_tex_video->tex_id, 0, 0, layer_alpha->m_width, layer_alpha->m_height);
	}
}

void GLRenderer::renderTexture(GLTexture2D* tex, int x, int y, int width, int height, GLRenderTarget& target)
{
	if (TextureDraw == nullptr)
	{
		TextureDraw = std::unique_ptr<DrawTexture>(new DrawTexture(false, true));
	}	
	glBindFramebuffer(GL_FRAMEBUFFER, target.m_fbo_video);
	TextureDraw->render(tex->tex_id, x, target.m_height - (y + height), width, height);
}

