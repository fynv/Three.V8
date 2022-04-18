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

	StandardRoutine::Options options;
	options.alpha_mode = material->alphaMode;
	options.is_highlight_pass = pass == Pass::Highlight;
	options.has_color = params.primitive->color_buf != nullptr;
	options.has_color_texture = material->tex_idx_map >= 0;
	options.has_metalness_map = material->tex_idx_metalnessMap >= 0;
	options.has_roughness_map = material->tex_idx_roughnessMap >= 0;
	options.has_normal_map = material->tex_idx_normalMap >= 0;
	options.num_directional_lights = params.lights->num_directional_lights;
	StandardRoutine* routine = get_routine(options);
	routine->render(params);		
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

void GLRenderer::render_simple_model(Camera* p_camera, const Lights& lights, SimpleModel* model, Pass pass)
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

void GLRenderer::render_gltf_model(Camera* p_camera, const Lights& lights, GLTFModel* model, Pass pass)
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

void GLRenderer::render(int width, int height, Scene& scene, Camera& camera)
{
	_update_framebuffers(width, height);
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

	camera.updateMatrixWorld(false);
	camera.updateConstant();	

	auto* p_scene = &scene;

	scene.traverse([this, p_scene](Object3D* obj) {
		obj->updateWorldMatrix(false, false);
		{
			SimpleModel* model = dynamic_cast<SimpleModel*>(obj);
			if (model)
			{
				update_simple_model(model);

				const MeshStandardMaterial* material = &model->material;
				if (material->alphaMode == AlphaMode::Blend)
				{
					p_scene->has_alpha = true;
				}
				else
				{
					p_scene->has_opaque = true;
				}

				return;
			}
		}
		{
			GLTFModel* model = dynamic_cast<GLTFModel*>(obj);
			if (model)
			{	
				update_gltf_model(model);
				size_t num_materials = model->m_materials.size();
				for (size_t i = 0; i < num_materials; i++)
				{
					const MeshStandardMaterial* material = model->m_materials[i].get();
					if (material->alphaMode == AlphaMode::Blend)
					{
						p_scene->has_alpha = true;
					}
					else
					{
						p_scene->has_opaque = true;
					}
				}
				return;
			}
		}
	});

	std::vector<ConstDirectionalLight> directional_lights;
	auto* p_directional_lights = &directional_lights;

	scene.traverse([p_directional_lights](Object3D* obj) {
		{
			DirectionalLight* light = dynamic_cast<DirectionalLight*>(obj);
			if (light)
			{
				ConstDirectionalLight const_light;
				const_light.color = glm::vec4(light->color * light->intensity, 1.0f);
				glm::vec3 pos_target = { 0.0f, 0.0f, 0.0f };
				if (light->target != nullptr)
				{
					pos_target = light->target->matrixWorld[3];
				}
				glm::vec3 position = light->matrixWorld[3];
				const_light.direction = glm::vec4(glm::normalize(position - pos_target), 0.0f);
				p_directional_lights->push_back(const_light);
			}
		}

	});	

	Lights& lights = scene.lights;
	{
		if (lights.num_directional_lights != (int)directional_lights.size())
		{
			lights.num_directional_lights = (int)directional_lights.size();
			lights.constant_directional_lights = nullptr;
			if (lights.num_directional_lights > 0)
			{
				lights.constant_directional_lights = std::unique_ptr<GLDynBuffer>(new GLDynBuffer(directional_lights.size() * sizeof(ConstDirectionalLight), GL_UNIFORM_BUFFER));
			}
			
		}
		if (lights.num_directional_lights > 0)
		{
			uint64_t hash = crc64(0, (unsigned char*)directional_lights.data(), directional_lights.size() * sizeof(ConstDirectionalLight));
			if (hash != lights.hash_directional_lights)
			{
				lights.hash_directional_lights = hash;
				lights.constant_directional_lights->upload(directional_lights.data());
			}
		}
	}	

	auto* p_camera = &camera;
	auto* p_lights = &lights;

	if (scene.has_opaque)
	{
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		scene.traverse([this, p_camera, p_lights](Object3D* obj) {
			{
				SimpleModel* model = dynamic_cast<SimpleModel*>(obj);
				if (model)
				{
					render_simple_model(p_camera, *p_lights, model, Pass::Opaque);
					return;
				}
			}
			{
				GLTFModel* model = dynamic_cast<GLTFModel*>(obj);
				if (model)
				{
					render_gltf_model(p_camera, *p_lights, model, Pass::Opaque);
					return;
				}
			}

		});
	}

	if (scene.has_alpha)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glDepthMask(GL_FALSE);

		scene.traverse([this, p_camera, p_lights](Object3D* obj) {
			{
				SimpleModel* model = dynamic_cast<SimpleModel*>(obj);
				if (model)
				{
					render_simple_model(p_camera, *p_lights, model, Pass::Highlight);
					return;
				}
			}
			{
				GLTFModel* model = dynamic_cast<GLTFModel*>(obj);
				if (model)
				{
					render_gltf_model(p_camera, *p_lights, model, Pass::Highlight);
					return;
				}
			}
		});


		if (OITResolver == nullptr)
		{
			OITResolver = std::unique_ptr<WeightedOIT>(new WeightedOIT);
		}
		OITResolver->PreDraw(width, height, m_rbo_msaa);
		scene.traverse([this, p_camera, p_lights](Object3D* obj) {
			{
				SimpleModel* model = dynamic_cast<SimpleModel*>(obj);
				if (model)
				{
					render_simple_model(p_camera, *p_lights, model, Pass::Alpha);
					return;
				}
			}
			{
				GLTFModel* model = dynamic_cast<GLTFModel*>(obj);
				if (model)
				{
					render_gltf_model(p_camera, *p_lights, model, Pass::Alpha);
					return;
				}
			}
		});

		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_msaa);
		OITResolver->PostDraw();
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}