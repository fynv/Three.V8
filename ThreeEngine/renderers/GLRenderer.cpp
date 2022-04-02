#include <GL/glew.h>

#include "GLUtils.h"
#include "GLRenderer.h"
#include "cameras/Camera.h"
#include "scenes/Scene.h"
#include "backgrounds/Background.h"
#include "geometries/Geometry1.h"
#include "models/SimpleModel.h"
#include "materials/MeshStandardMaterial.h"

//#include <gtx/string_cast.hpp>

class TestRoutine
{
public:
	struct ConstRenderer
	{	
		glm::mat4 ProjMat;
		glm::mat4 ViewMat;
		glm::mat4 ModelMat;		
		glm::mat4 NormalMat;
	};

	TestRoutine()
		: m_vert_shader(GL_VERTEX_SHADER, s_vertex_shader)
		, m_frag_shader(GL_FRAGMENT_SHADER, s_frag_shader)
		, m_constant_render(sizeof(ConstRenderer), GL_UNIFORM_BUFFER)
	{
		m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(m_vert_shader, m_frag_shader));
	}

	void render(const GLTexture2D& tex, const MeshStandardMaterial& material, const Geometry1& geo, const glm::mat4& matProj, const glm::mat4& matView, const glm::mat4& matModel)
	{
		ConstRenderer constRender;
		constRender.ProjMat = matProj;
		constRender.ViewMat = matView;
		constRender.ModelMat = matModel;
		constRender.NormalMat = glm::transpose(glm::inverse(matModel));
		m_constant_render.upload(&constRender);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		
		glUseProgram(m_prog->m_id);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_constant_render.m_id);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, material.constant_material.m_id);

		glBindBuffer(GL_ARRAY_BUFFER, geo.pos_buf->m_id);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, geo.normal_buf->m_id);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, geo.uv_buf->m_id);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(2);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex.tex_id);
		glUniform1i(0, 0);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo.ind_buf->m_id);
		glDrawElements(GL_TRIANGLES, geo.num_face * 3, GL_UNSIGNED_INT, nullptr);

		glUseProgram(0);
	}	

private:
	static const char* s_vertex_shader;
	static const char* s_frag_shader;
	std::unique_ptr<GLProgram> m_prog;
	GLShader m_vert_shader;
	GLShader m_frag_shader;
	GLDynBuffer m_constant_render;
};


const char* TestRoutine::s_vertex_shader =
"#version 430\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aNorm;\n"
"layout (location = 2) in vec2 aUV;\n"

"layout (std140, binding = 0) uniform RendererUniform\n"
"{\n"
"	mat4 uProjMat;\n"
"	mat4 uViewMat;\n"
"	mat4 uModelMat;\n"
"	mat4 uNormalMat;\n"
"};\n"

"layout (location = 0) out vec3 vPos;\n"
"layout (location = 1) out vec3 vNorm;\n"
"layout (location = 2) out vec2 vUV;\n"
"layout (location = 3) out vec3 vViewNorm;\n"
"void main()\n"
"{\n"
"	vec4 wolrd_pos = uModelMat * vec4(aPos, 1.0);\n"
"	gl_Position = uProjMat*(uViewMat*wolrd_pos);\n"
"	vPos = wolrd_pos.xyz;\n"
"	vec4 world_norm = uNormalMat * vec4(aNorm, 0.0);\n"
"	vNorm =  normalize(world_norm.xyz);\n"
"	vUV = aUV;\n"
"	vec4 view_norm = uViewMat * vec4(vNorm, 0.0);\n"
"	vViewNorm = view_norm.xyz;\n"
"}\n";

const char* TestRoutine::s_frag_shader =
"#version 430\n"
"layout (location = 0) in vec3 vPos;\n"
"layout (location = 1) in vec3 vNorm;\n"
"layout (location = 2) in vec2 vUV;\n"
"layout (location = 3) in vec3 vViewNorm;\n"

"layout (std140, binding = 1) uniform MaterialUniform\n"
"{\n"
"	vec3 uColor;\n"
"};\n"

"layout (location = 0) uniform sampler2D uTexColor;\n"

"out vec4 outColor;"
"void main()\n"
"{\n"
"   vec3 norm = normalize(vNorm);\n"
"	if (vViewNorm.z<0.0) norm = -norm;\n"
"   vec3 light_dir = normalize(vec3(1.0f, 2.0f, 1.0f));\n"
"   float l_diffuse = clamp(dot(light_dir, norm), 0.0, 1.0);\n"
"   float l_ambient = 0.3;\n"
"   vec3 col = uColor * texture(uTexColor, vUV).xyz;\n"
"   col *= (l_diffuse + l_ambient);\n"
"   outColor = vec4(col, 1.0);\n"
"}\n";

class Caches
{
public:
	TestRoutine test;
};


GLRenderer::GLRenderer()
{

}

GLRenderer::~GLRenderer()
{
}

Caches* GLRenderer::s_caches = nullptr;

Caches* GLRenderer::GetCaches()
{
	if (s_caches == nullptr)
	{
		s_caches = new Caches;
	}
	return s_caches;
}

void GLRenderer::ClearCaches()
{	
	delete s_caches;
	s_caches = nullptr;
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
	const glm::mat4& matProj = camera.projectionMatrix;
	const glm::mat4& matView = camera.matrixWorldInverse;
	Caches* caches = GetCaches();

	scene.traverse([matProj, matView, caches](Object3D* obj) {
		obj->updateWorldMatrix(false, false);
		{
			SimpleModel* model = dynamic_cast<SimpleModel*>(obj);
			if (model)
			{					
				caches->test.render(model->texture, model->material, model->geometry, matProj, matView, model->matrixWorld);
				return;
			}
		}

	});
}