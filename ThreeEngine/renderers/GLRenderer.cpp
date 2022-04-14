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

void GLRenderer::render_primitive(const StandardRoutine::RenderParams& params)
{	
	StandardRoutine::Options options;
	options.has_color = params.primitive->color_buf != nullptr;
	const MeshStandardMaterial* material = params.material_list[params.primitive->material_idx];
	options.has_color_texture = material->tex_idx_map >= 0;
	options.has_metalness_map = material->tex_idx_metalnessMap >= 0;
	options.has_roughness_map = material->tex_idx_roughnessMap >= 0;
	options.has_normal_map = material->tex_idx_normalMap >= 0;
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

void GLRenderer::render_simple_model(Camera* p_camera, SimpleModel* model)
{	
	const GLTexture2D* tex = &model->texture;
	const MeshStandardMaterial* material = &model->material;

	StandardRoutine::RenderParams params = {
		&tex,
		&material,
		&p_camera->m_constant,
		&model->m_constant,
		&model->geometry
	};
	render_primitive(params);
}

void GLRenderer::render_gltf_model(Camera* p_camera, GLTFModel* model)
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
			StandardRoutine::RenderParams params = {
				tex_lst.data(),
				material_lst.data(),
				&p_camera->m_constant,
				mesh.model_constant.get(),
				&primitive
			};
			render_primitive(params);
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

	scene.traverse([this](Object3D* obj) {
		obj->updateWorldMatrix(false, false);
		{
			SimpleModel* model = dynamic_cast<SimpleModel*>(obj);
			if (model)
			{
				update_simple_model(model);
				return;
			}
		}
		{
			GLTFModel* model = dynamic_cast<GLTFModel*>(obj);
			if (model)
			{	
				update_gltf_model(model);
				return;
			}
		}
	});

	Camera* p_camera = &camera;
	scene.traverse([this, p_camera](Object3D* obj) {		
		{
			SimpleModel* model = dynamic_cast<SimpleModel*>(obj);
			if (model)
			{
				render_simple_model(p_camera, model);
				return;
			}
		}
		{
			GLTFModel* model = dynamic_cast<GLTFModel*>(obj);			
			if (model)
			{
				render_gltf_model(p_camera, model);
				return;
			}
		}

	});

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}