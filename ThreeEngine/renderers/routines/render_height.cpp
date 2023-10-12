#include <GL/glew.h>
#include "models/ModelComponents.h"
#include "render_height.h"

static std::string g_vertex =
R"(#version 430
layout (location = 0) in vec3 aPos;

layout (std140, binding = 0) uniform Height
{
	vec4 pos_min;
	vec4 pos_max;
};

layout (std140, binding = 1) uniform Model
{
	mat4 uModelMat;
	mat4 uNormalMat;
};

void main()
{
	vec4 wolrd_pos = uModelMat * vec4(aPos, 1.0);
	vec3 uvz = (wolrd_pos.xzy - pos_min.xzy)/(pos_max.xzy - pos_min.xzy);
	gl_Position = vec4(uvz * 2.0 - 1.0, 1.0);
}
)";


static std::string g_frag =
R"(#version 430
void main()
{

}
)";


RenderHeight::RenderHeight()
{
	GLShader vert_shader(GL_VERTEX_SHADER, g_vertex.c_str());
	GLShader frag_shader(GL_FRAGMENT_SHADER, g_frag.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(vert_shader, frag_shader));
}

void RenderHeight::render(const RenderParams& params)
{
	const GeometrySet& geo = params.primitive->geometry[params.primitive->geometry.size() - 1];
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GEQUAL);
	glDepthMask(GL_TRUE);

	glFrontFace(GL_CCW);
	glDisable(GL_CULL_FACE);

	glUseProgram(m_prog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.constant_height->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.constant_model->m_id);

	glBindBuffer(GL_ARRAY_BUFFER, geo.pos_buf->m_id);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	if (params.primitive->index_buf != nullptr)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, params.primitive->index_buf->m_id);
		if (params.primitive->type_indices == 1)
		{
			glDrawElements(GL_TRIANGLES, params.primitive->num_face * 3, GL_UNSIGNED_BYTE, nullptr);
		}
		else if (params.primitive->type_indices == 2)
		{
			glDrawElements(GL_TRIANGLES, params.primitive->num_face * 3, GL_UNSIGNED_SHORT, nullptr);
		}
		else if (params.primitive->type_indices == 4)
		{
			glDrawElements(GL_TRIANGLES, params.primitive->num_face * 3, GL_UNSIGNED_INT, nullptr);
		}
	}
	else
	{
		glDrawArrays(GL_TRIANGLES, 0, params.primitive->num_pos);
	}

	glUseProgram(0);

}
