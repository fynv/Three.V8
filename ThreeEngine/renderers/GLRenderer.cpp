#include <GL/glew.h>

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
				const Material* material = &model->material;				
				test.render(&tex, &material, &p_camera->m_constant, &model->m_constant, model->geometry);
				return;
			}
		}
		{
			GLTFModel* model = dynamic_cast<GLTFModel*>(obj);			
			if (model)
			{
				model->updateConstant();
				for (size_t i = 0; i < model->m_meshs.size(); i++)
				{
					Mesh& mesh = model->m_meshs[i];					
					
					for (size_t j = 0; j < mesh.primitives.size(); j++)
					{
						Primitive& primitive = mesh.primitives[j];
						test2.render(0, 0, &p_camera->m_constant, &model->m_constant, primitive);
					}
				}
				return;
			}
		}

	});
}