#include <GL/glew.h>
#include "TestRoutine.h"
#include "materials/MeshStandardMaterial.h"

const char* TestRoutine::s_vertex_shader =
R"(#version 430
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aUV;

layout (std140, binding = 0) uniform Camera
{
	mat4 uProjMat;
	mat4 uViewMat;	
};

layout (std140, binding = 1) uniform Model
{
	mat4 uModelMat;
	mat4 uNormalMat;
};

/*layout (std140, binding = 0) uniform RendererUniform
{
	mat4 uProjMat;
	mat4 uViewMat;
	mat4 uModelMat;
	mat4 uNormalMat;
};*/

layout (location = 0) out vec3 vPos;
layout (location = 1) out vec3 vNorm;
layout (location = 2) out vec2 vUV;
void main()
{
	vec4 wolrd_pos = uModelMat * vec4(aPos, 1.0);
	gl_Position = uProjMat*(uViewMat*wolrd_pos);
	vPos = wolrd_pos.xyz;
	vec4 world_norm = uNormalMat * vec4(aNorm, 0.0);
	vNorm =  normalize(world_norm.xyz);
	vUV = aUV;
}
)";

const char* TestRoutine::s_frag_shader =
R"(
#version 430
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;
layout (location = 2) in vec2 vUV;
layout (std140, binding = 2) uniform Material
{
	vec3 uColor;
};
layout (location = 0) uniform sampler2D uTexColor;
out vec4 outColor;void main()
{
   vec3 norm = normalize(vNorm);
   vec3 light_dir = normalize(vec3(1.0f, 2.0f, 1.0f));
   float l_diffuse = clamp(dot(light_dir, norm), 0.0, 1.0);
   float l_ambient = 0.3;
   vec3 col = uColor * texture(uTexColor, vUV).xyz;
   col *= (l_diffuse + l_ambient);
   outColor = vec4(col, 1.0);
}
)";

TestRoutine::TestRoutine()
	: m_vert_shader(GL_VERTEX_SHADER, s_vertex_shader)
	, m_frag_shader(GL_FRAGMENT_SHADER, s_frag_shader)
{
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(m_vert_shader, m_frag_shader));
}

void TestRoutine::render(const GLTexture2D** tex_list, const Material** material_list, const GLDynBuffer* constant_camera, const GLDynBuffer* constant_model, const Primitive& primitive)
{
	const MeshStandardMaterial& material = *(MeshStandardMaterial*)material_list[primitive.material_idx];
	const GLTexture2D& tex = *tex_list[material.tex_idx_map];
	const GeometrySet& geo = primitive.geometry[primitive.geometry.size() - 1];	

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glUseProgram(m_prog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, constant_camera->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, constant_model->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, material.constant_material.m_id);

	glBindBuffer(GL_ARRAY_BUFFER, geo.pos_buf->m_id);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, geo.normal_buf->m_id);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, primitive.uv_buf->m_id);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex.tex_id);
	glUniform1i(0, 0);

	if (primitive.index_buf != nullptr)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, primitive.index_buf->m_id);
		if (primitive.type_indices == 2)
		{
			glDrawElements(GL_TRIANGLES, primitive.num_face * 3, GL_UNSIGNED_SHORT, nullptr);
		}
		else if (primitive.type_indices == 4)
		{
			glDrawElements(GL_TRIANGLES, primitive.num_face * 3, GL_UNSIGNED_INT, nullptr);
		}
	}
	else
	{
		glDrawArrays(GL_TRIANGLES, 0, primitive.num_pos);
	}

	glUseProgram(0);
}

const char* TestRoutine2::s_vertex_shader =
R"(#version 430
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec3 aColor;
layout (std140, binding = 0) uniform Camera
{
	mat4 uProjMat;
	mat4 uViewMat;	
};

layout (std140, binding = 1) uniform Model
{
	mat4 uModelMat;
	mat4 uNormalMat;
};

layout (location = 0) out vec3 vPos;
layout (location = 1) out vec3 vNorm;
layout (location = 2) out vec3 vColor;
void main()
{
	vec4 wolrd_pos = uModelMat * vec4(aPos, 1.0);
	gl_Position = uProjMat*(uViewMat*wolrd_pos);
	vPos = wolrd_pos.xyz;
	vec4 world_norm = uNormalMat * vec4(aNorm, 0.0);
	vNorm =  normalize(world_norm.xyz);
	vColor = aColor;
}
)";

const char* TestRoutine2::s_frag_shader =
R"(#version 430
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;
layout (location = 2) in vec3 vColor;
out vec4 outColor;void main()
{
   vec3 norm = normalize(vNorm);
   vec3 light_dir = normalize(vec3(1.0f, 2.0f, 1.0f));
   float l_diffuse = clamp(dot(light_dir, norm), 0.0, 1.0);
   float l_ambient = 0.3;
   vec3 col = vColor;
   col *= (l_diffuse + l_ambient);
   outColor = vec4(col, 1.0);
}
)";

TestRoutine2::TestRoutine2()
	: m_vert_shader(GL_VERTEX_SHADER, s_vertex_shader)
	, m_frag_shader(GL_FRAGMENT_SHADER, s_frag_shader)
{
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(m_vert_shader, m_frag_shader));
}

void TestRoutine2::render(const GLTexture2D** tex_list, const Material** material_list, const GLDynBuffer* constant_camera, const GLDynBuffer* constant_model, const Primitive& primitive)
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glUseProgram(m_prog->m_id);

	const GeometrySet& geo = primitive.geometry[primitive.geometry.size() - 1];

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, constant_camera->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, constant_model->m_id);

	glBindBuffer(GL_ARRAY_BUFFER, geo.pos_buf->m_id);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, geo.normal_buf->m_id);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, primitive.color_buf->m_id);
	glVertexAttribPointer(2, primitive.type_color, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(2);

	if (primitive.index_buf != nullptr)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, primitive.index_buf->m_id);
		if (primitive.type_indices == 2)
		{
			glDrawElements(GL_TRIANGLES, primitive.num_face * 3, GL_UNSIGNED_SHORT, nullptr);
		}
		else if (primitive.type_indices == 4)
		{
			glDrawElements(GL_TRIANGLES, primitive.num_face * 3, GL_UNSIGNED_INT, nullptr);
		}
	}
	else
	{
		glDrawArrays(GL_TRIANGLES, 0, primitive.num_pos);
	}
	glUseProgram(0);
}