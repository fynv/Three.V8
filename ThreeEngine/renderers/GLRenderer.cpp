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

void GLRenderer::render(int width, int height, Scene& scene, Camera& camera)
{
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

	Camera* p_camera = &camera;

	scene.traverse([this, p_camera](Object3D* obj) {
		obj->updateWorldMatrix(false, false);
		{
			SimpleModel* model = dynamic_cast<SimpleModel*>(obj);
			if (model)
			{
				model->updateConstant();
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
				return;
			}
		}
		{
			GLTFModel* model = dynamic_cast<GLTFModel*>(obj);			
			if (model)
			{
				model->updateConstant();
				
				std::vector<const GLTexture2D*> tex_lst(model->m_textures.size());
				for (size_t i = 0; i < tex_lst.size(); i++)
					tex_lst[i] = model->m_textures[i].get();

				std::vector<const MeshStandardMaterial*> material_lst(model->m_materials.size());
				for (size_t i = 0; i < material_lst.size(); i++)
					material_lst[i] = model->m_materials[i].get();

				for (size_t i = 0; i < model->m_meshs.size(); i++)
				{
					Mesh& mesh = model->m_meshs[i];
					
					if (mesh.needUpdateMorphTargets)
					{
						int num_targets =(int)mesh.weights.size();
						if (mesh.buf_weights == nullptr)
						{
							mesh.buf_weights = std::unique_ptr<GLDynBuffer>(new GLDynBuffer(sizeof(float) * num_targets, GL_SHADER_STORAGE_BUFFER));
						}
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
								primitive.num_targets,
								mesh.buf_weights.get(),
								&primitive
							};
							morpher->update(params);
						}
						mesh.needUpdateMorphTargets = false;
					}
					
					for (size_t j = 0; j < mesh.primitives.size(); j++)
					{
						Primitive& primitive = mesh.primitives[j];
						StandardRoutine::RenderParams params = {
							tex_lst.data(), 
							material_lst.data(),
							&p_camera->m_constant,
							&model->m_constant, 
							&primitive
						};
						render_primitive(params);
					}
				}
				return;
			}
		}

	});
}