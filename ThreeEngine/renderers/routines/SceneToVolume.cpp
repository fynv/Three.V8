#include <GL/glew.h>
#include "models/ModelComponents.h"
#include "SceneToVolume.h"

static std::string g_vertex_x =
R"(#version 430

layout (location = 0) in vec3 aPos;

layout (std140, binding = 0) uniform Model
{
	mat4 uModelMat;
	mat4 uNormalMat;
};

layout (location = 0) uniform vec3 uCoverageMin;
layout (location = 1) uniform vec3 uCoverageMax;

void main()
{
	vec4 wolrd_pos = uModelMat * vec4(aPos, 1.0);
	vec3 clip_pos = (wolrd_pos.xyz - uCoverageMin)/(uCoverageMax - uCoverageMin) * 2.0 - 1.0;
	gl_Position = vec4(clip_pos.y, clip_pos.z, clip_pos.x, 1.0);
}
)";


static std::string g_frag_x =
R"(#version 430

layout (location = 2) uniform int uDiv;
layout (binding=1, r8ui) uniform uimage3D uTexVol;

void main()
{
	vec3 coord = gl_FragCoord.xyz;
	coord.z = floor(coord.z * float(uDiv) + 0.5);
	ivec3 icoord = ivec3(coord);
	if (icoord.z>=0 && icoord.z<uDiv)
	{
		uint v = imageLoad(uTexVol, icoord.zxy).x;
		v+=1;
		imageStore(uTexVol, icoord.zxy, uvec4(v));	
	}
}
)";


static std::string g_vertex_y =
R"(#version 430

layout (location = 0) in vec3 aPos;

layout (std140, binding = 0) uniform Model
{
	mat4 uModelMat;
	mat4 uNormalMat;
};

layout (location = 0) uniform vec3 uCoverageMin;
layout (location = 1) uniform vec3 uCoverageMax;

void main()
{
	vec4 wolrd_pos = uModelMat * vec4(aPos, 1.0);
	vec3 clip_pos = (wolrd_pos.xyz - uCoverageMin)/(uCoverageMax - uCoverageMin) * 2.0 - 1.0;
	gl_Position = vec4(clip_pos.z, clip_pos.x, clip_pos.y, 1.0);
}
)";


static std::string g_frag_y =
R"(#version 430

layout (location = 2) uniform int uDiv;
layout (binding=1, r8ui) uniform uimage3D uTexVol;

void main()
{	
	vec3 coord = gl_FragCoord.xyz;
	coord.z = floor(coord.z * float(uDiv) + 0.5);
	ivec3 icoord = ivec3(coord);
	if (icoord.z>=0 && icoord.z<uDiv)
	{
		uint v = imageLoad(uTexVol, icoord.yzx).x;
		v+=2;
		imageStore(uTexVol, icoord.yzx, uvec4(v));	
	}
}
)";


static std::string g_vertex_z =
R"(#version 430

layout (location = 0) in vec3 aPos;

layout (std140, binding = 0) uniform Model
{
	mat4 uModelMat;
	mat4 uNormalMat;
};

layout (location = 0) uniform vec3 uCoverageMin;
layout (location = 1) uniform vec3 uCoverageMax;

void main()
{
	vec4 wolrd_pos = uModelMat * vec4(aPos, 1.0);
	vec3 clip_pos = (wolrd_pos.xyz - uCoverageMin)/(uCoverageMax - uCoverageMin) * 2.0 - 1.0;
	gl_Position = vec4(clip_pos.x, clip_pos.y, clip_pos.z, 1.0);
}
)";


static std::string g_frag_z =
R"(#version 430

layout (location = 2) uniform int uDiv;
layout (binding=1, r8ui) uniform uimage3D uTexVol;

void main()
{
	vec3 coord = gl_FragCoord.xyz;
	coord.z = floor(coord.z * float(uDiv) + 0.5);
	ivec3 icoord = ivec3(coord);
	if (icoord.z>=0 && icoord.z<uDiv)
	{
		uint v = imageLoad(uTexVol, icoord.xyz).x;
		v+=4;
		imageStore(uTexVol, icoord.xyz, uvec4(v));	
	}
}
)";

SceneToVolume::SceneToVolume()
	: target_x(false, false)
	, target_y(false, false)
	, target_z(false, false)
{
	{
		GLShader vert_shader(GL_VERTEX_SHADER, g_vertex_x.c_str());
		GLShader frag_shader(GL_FRAGMENT_SHADER, g_frag_x.c_str());
		m_prog_x = (std::unique_ptr<GLProgram>)(new GLProgram(vert_shader, frag_shader));
	}

	{
		GLShader vert_shader(GL_VERTEX_SHADER, g_vertex_y.c_str());
		GLShader frag_shader(GL_FRAGMENT_SHADER, g_frag_y.c_str());
		m_prog_y = (std::unique_ptr<GLProgram>)(new GLProgram(vert_shader, frag_shader));
	}

	{
		GLShader vert_shader(GL_VERTEX_SHADER, g_vertex_z.c_str());
		GLShader frag_shader(GL_FRAGMENT_SHADER, g_frag_z.c_str());
		m_prog_z = (std::unique_ptr<GLProgram>)(new GLProgram(vert_shader, frag_shader));
	}
}

SceneToVolume::~SceneToVolume()
{

}

void SceneToVolume::render(const RenderParams& params)
{
	const GeometrySet& geo = params.primitive->geometry[params.primitive->geometry.size() - 1];
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glDisable(GL_SCISSOR_TEST);

	// x
	{
		target_x.update_framebuffers(params.divisions.y, params.divisions.z);
		target_x.bind_buffer();

		glViewport(0, 0, params.divisions.y, params.divisions.z);
		glUseProgram(m_prog_x->m_id);

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.constant_model->m_id);
		glUniform3fv(0, 1, (float*)&params.coverage_min);
		glUniform3fv(1, 1, (float*)&params.coverage_max);
		glUniform1i(2, params.divisions.x);		
		glBindImageTexture(1, params.tex_id_volume, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8UI);		

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

	// y
	{
		target_y.update_framebuffers(params.divisions.z, params.divisions.x);
		target_y.bind_buffer();

		glViewport(0, 0, params.divisions.z, params.divisions.x);
		glUseProgram(m_prog_y->m_id);

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.constant_model->m_id);
		glUniform3fv(0, 1, (float*)&params.coverage_min);
		glUniform3fv(1, 1, (float*)&params.coverage_max);
		glUniform1i(2, params.divisions.y);
		glBindImageTexture(1, params.tex_id_volume, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8UI);

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

	// z
	{
		target_z.update_framebuffers(params.divisions.x, params.divisions.y);
		target_z.bind_buffer();

		glViewport(0, 0, params.divisions.x, params.divisions.y);
		glUseProgram(m_prog_z->m_id);

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.constant_model->m_id);
		glUniform3fv(0, 1, (float*)&params.coverage_min);
		glUniform3fv(1, 1, (float*)&params.coverage_max);
		glUniform1i(2, params.divisions.z);
		glBindImageTexture(1, params.tex_id_volume, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8UI);

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
}

