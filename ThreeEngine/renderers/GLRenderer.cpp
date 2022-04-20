#include <GL/glew.h>
#include "crc64/crc64.h"
#include "GLUtils.h"
#include "GLRenderer.h"
#include "cameras/Camera.h"
#include "scenes/Scene.h"
#include "backgrounds/Background.h"
#include "models/ModelComponents.h"
#include "models/SimpleModel.h"
#include "models/GLTFModel.h"
#include "materials/MeshStandardMaterial.h"
#include "lights/DirectionalLight.h"
#include "lights/DirectionalLightShadow.h"

//#include <gtx/string_cast.hpp>

GLRenderer::GLRenderer()
{

}

GLRenderer::~GLRenderer()
{
	if (m_fbo_msaa != -1)
		glDeleteFramebuffers(1, &m_fbo_msaa);
	if (m_tex_msaa != -1)
		glDeleteTextures(1, &m_tex_msaa);
	if (m_rbo_msaa != -1)
		glDeleteRenderbuffers(1, &m_rbo_msaa);
}


void GLRenderer::_update_framebuffers(int width, int height)
{
	if (m_width != width || m_height != height)
	{
		if (m_fbo_msaa == -1)
		{
			glGenFramebuffers(1, &m_fbo_msaa);
			glGenTextures(1, &m_tex_msaa);
			glGenRenderbuffers(1, &m_rbo_msaa);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_msaa);

		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, m_tex_msaa);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_SRGB8_ALPHA8, width, height, true);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, m_tex_msaa, 0);

		glBindRenderbuffer(GL_RENDERBUFFER, m_rbo_msaa);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT24, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo_msaa);

		m_width = width;
		m_height = height;
	}
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
				MorphUpdate* morpher = nullptr;
				if (!has_tangent)
				{
					if (morphers[0] == nullptr)
					{
						morphers[0] = std::unique_ptr<MorphUpdate>(new MorphUpdate(false));
					}
					morpher = morphers[0].get();
				}
				else
				{
					if (morphers[1] == nullptr)
					{
						morphers[1] = std::unique_ptr<MorphUpdate>(new MorphUpdate(true));
					}
					morpher = morphers[1].get();
				}

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

void GLRenderer::render_primitive(const StandardRoutine::RenderParams& params, Pass pass)
{
	const MeshStandardMaterial* material = params.material_list[params.primitive->material_idx];
	const Lights* lights = params.lights;	

	StandardRoutine::Options options;
	options.alpha_mode = material->alphaMode;
	options.is_highlight_pass = pass == Pass::Highlight;
	options.has_color = params.primitive->color_buf != nullptr;
	options.has_color_texture = material->tex_idx_map >= 0;
	options.has_metalness_map = material->tex_idx_metalnessMap >= 0;
	options.has_roughness_map = material->tex_idx_roughnessMap >= 0;
	options.has_normal_map = material->tex_idx_normalMap >= 0;
	options.num_directional_lights = lights->num_directional_lights;
	options.num_directional_shadows = (int)lights->directional_shadow_texs.size();
	StandardRoutine* routine = get_routine(options);
	routine->render(params);
}


void GLRenderer::render_model(Camera* p_camera, const Lights& lights, SimpleModel* model, Pass pass)
{	
	const GLTexture2D* tex = &model->texture;
	const MeshStandardMaterial* material = &model->material;

	if (pass == Pass::Opaque)
	{
		if (material->alphaMode == AlphaMode::Blend) return;
	}
	else if (pass == Pass::Opaque || pass == Pass::Highlight)
	{
		if (material->alphaMode != AlphaMode::Blend) return;
	}

	StandardRoutine::RenderParams params;
	params.tex_list = &tex;
	params.material_list = &material;
	params.constant_camera = &p_camera->m_constant;
	params.constant_model = &model->m_constant;
	params.primitive = &model->geometry;
	params.lights = &lights;
	render_primitive(params, pass);
}

void GLRenderer::render_model(Camera* p_camera, const Lights& lights, GLTFModel* model, Pass pass)
{
	std::vector<const GLTexture2D*> tex_lst(model->m_textures.size());
	for (size_t i = 0; i < tex_lst.size(); i++)
		tex_lst[i] = model->m_textures[i].get();

	std::vector<const MeshStandardMaterial*> material_lst(model->m_materials.size());
	for (size_t i = 0; i < material_lst.size(); i++)
		material_lst[i] = model->m_materials[i].get();

	for (size_t i = 0; i < model->m_meshs.size(); i++)
	{
		Mesh& mesh = model->m_meshs[i];
		for (size_t j = 0; j < mesh.primitives.size(); j++)
		{
			Primitive& primitive = mesh.primitives[j];
			const MeshStandardMaterial* material = material_lst[primitive.material_idx];
			if (pass == Pass::Opaque)
			{
				if (material->alphaMode == AlphaMode::Blend) continue;
			}
			else if (pass == Pass::Opaque || pass == Pass::Highlight)
			{
				if (material->alphaMode != AlphaMode::Blend) continue;
			}
			StandardRoutine::RenderParams params;
			params.tex_list = tex_lst.data();
			params.material_list = material_lst.data();
			params.constant_camera = &p_camera->m_constant;
			params.constant_model = mesh.model_constant.get();
			params.primitive = &primitive;
			params.lights = &lights;
			render_primitive(params, pass);
		}
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
	const GLTexture2D* tex = &model->texture;
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
	std::vector<const GLTexture2D*> tex_lst(model->m_textures.size());
	for (size_t i = 0; i < tex_lst.size(); i++)
		tex_lst[i] = model->m_textures[i].get();

	std::vector<const MeshStandardMaterial*> material_lst(model->m_materials.size());
	for (size_t i = 0; i < material_lst.size(); i++)
		material_lst[i] = model->m_materials[i].get();

	for (size_t i = 0; i < model->m_meshs.size(); i++)
	{
		Mesh& mesh = model->m_meshs[i];
		for (size_t j = 0; j < mesh.primitives.size(); j++)
		{
			Primitive& primitive = mesh.primitives[j];
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

void GLRenderer::render(int width, int height, Scene& scene, Camera& camera)
{
	GLint video_buf_id = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &video_buf_id);

	_update_framebuffers(width, height);

	camera.updateMatrixWorld(false);
	camera.updateConstant();	

	// enumerate objects
	struct Lists
	{
		std::vector<SimpleModel*> simple_models;
		std::vector<GLTFModel*> gltf_models;
		std::vector<DirectionalLight*> directional_lights;
	};

	Lists lists;
	auto* p_lists = &lists;	

	scene.traverse([p_lists](Object3D* obj) {
		obj->updateWorldMatrix(false, false);
		{
			SimpleModel* model = dynamic_cast<SimpleModel*>(obj);
			if (model)
			{
				p_lists->simple_models.push_back(model);
				return;
			}
		}
		{
			GLTFModel* model = dynamic_cast<GLTFModel*>(obj);
			if (model)
			{
				p_lists->gltf_models.push_back(model);
				return;
			}
		}
		{
			DirectionalLight* light = dynamic_cast<DirectionalLight*>(obj);
			if (light)
			{
				p_lists->directional_lights.push_back(light);
				return;
			}
		}
	});

	// update models
	for (size_t i = 0; i < lists.simple_models.size(); i++)
	{
		SimpleModel* model = lists.simple_models[i];
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

	for (size_t i = 0; i < lists.gltf_models.size(); i++)
	{
		GLTFModel* model = lists.gltf_models[i];
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
	for (size_t i = 0; i < lists.directional_lights.size(); i++)
	{
		DirectionalLight* light = lists.directional_lights[i];
		if (light->shadow != nullptr)
		{
			light->shadow->updateMatrices();
			glBindFramebuffer(GL_FRAMEBUFFER, light->shadow->m_lightFBO);
			glViewport(0, 0, light->shadow->m_map_width, light->shadow->m_map_height);
			const float one = 1.0f;
			glDepthMask(GL_TRUE);
			glClearBufferfv(GL_DEPTH, 0, &one);

			for (size_t j = 0; j < lists.simple_models.size(); j++)
			{
				SimpleModel* model = lists.simple_models[j];
				render_shadow_model(light->shadow.get(), model);
			}

			for (size_t j = 0; j < lists.gltf_models.size(); j++)
			{
				GLTFModel* model = lists.gltf_models[j];
				render_shadow_model(light->shadow.get(), model);
			}
		}
	}


	// update light constants
	Lights& lights = scene.lights;
	lights.directional_shadow_texs.clear();

	std::vector<ConstDirectionalLight> const_directional_lights(lists.directional_lights.size());
	for (size_t i = 0; i < lists.directional_lights.size(); i++)
	{
		DirectionalLight* light = lists.directional_lights[i];
		ConstDirectionalLight& const_light = const_directional_lights[i];
		light->makeConst(const_light);

		if (light->shadow != nullptr)
		{
			lights.directional_shadow_texs.push_back(light->shadow->m_lightTex);
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

	// render scene
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_msaa);

	glEnable(GL_FRAMEBUFFER_SRGB);
	glViewport(0, 0, width, height);
	if (scene.background != nullptr)
	{
		{
			ColorBackground* bg = dynamic_cast<ColorBackground*>(scene.background);
			glClearColor(bg->color.r, bg->color.g, bg->color.b, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
		}
	}

	glDepthMask(GL_TRUE);
	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);

	auto* p_camera = &camera;
	auto* p_lights = &lights;

	if (scene.has_opaque)
	{
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);

		for (size_t i = 0; i < lists.simple_models.size(); i++)
		{
			SimpleModel* model = lists.simple_models[i];
			render_model(p_camera, *p_lights, model, Pass::Opaque);
		}

		for (size_t i = 0; i < lists.gltf_models.size(); i++)
		{
			GLTFModel* model = lists.gltf_models[i];
			render_model(p_camera, *p_lights, model, Pass::Opaque);
		}
	}

	if (scene.has_alpha)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glDepthMask(GL_FALSE);

		for (size_t i = 0; i < lists.simple_models.size(); i++)
		{
			SimpleModel* model = lists.simple_models[i];
			render_model(p_camera, *p_lights, model, Pass::Highlight);
		}

		for (size_t i = 0; i < lists.gltf_models.size(); i++)
		{
			GLTFModel* model = lists.gltf_models[i];
			render_model(p_camera, *p_lights, model, Pass::Highlight);
		}

		if (OITResolver == nullptr)
		{
			OITResolver = std::unique_ptr<WeightedOIT>(new WeightedOIT);
		}
		OITResolver->PreDraw(width, height, m_rbo_msaa);

		for (size_t i = 0; i < lists.simple_models.size(); i++)
		{
			SimpleModel* model = lists.simple_models[i];
			render_model(p_camera, *p_lights, model, Pass::Alpha);
		}

		for (size_t i = 0; i < lists.gltf_models.size(); i++)
		{
			GLTFModel* model = lists.gltf_models[i];
			render_model(p_camera, *p_lights, model, Pass::Alpha);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_msaa);
		OITResolver->PostDraw();
	}

	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, video_buf_id);
	glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, video_buf_id);

#if 0
	// visualize shadow map
	for (size_t i = 0; i < lists.directional_lights.size(); i++)
	{
		DirectionalLight* light = lists.directional_lights[i];
		if (light->shadow != nullptr)
		{
			if (TextureVisualizer == nullptr)
			{
				TextureVisualizer = std::unique_ptr<DrawTexture>(new DrawTexture);
			}
			TextureVisualizer->render(light->shadow->m_lightTex, 10, 10, 512, 512);
			break;
		}
	}
#endif
}
