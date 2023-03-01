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
layout (location = 2) uniform vec2 uScale;

layout (location = 0) out vec3 vWorldPos;

void main()
{
	vec4 wolrd_pos = uModelMat * vec4(aPos, 1.0);
	vec3 clip_pos = (wolrd_pos.xyz - uCoverageMin)/(uCoverageMax - uCoverageMin) * 2.0 - 1.0;
	gl_Position = vec4(clip_pos.yz * uScale, clip_pos.x, 1.0);
	vWorldPos = wolrd_pos.xyz;
}
)";


static std::string g_frag_x =
R"(#version 430

layout (location = 0) in vec3 vWorldPos;

layout (location = 3) uniform int uDiv;
layout (binding=1, r8) uniform highp writeonly image3D uTexVol;

void main()
{
	vec3 coord = gl_FragCoord.xyz;
	coord.z = clamp(floor(coord.z * float(uDiv)), 0.0, float(uDiv) - 1.0);
	ivec3 icoord = ivec3(coord);

	imageStore(uTexVol, icoord.zxy + ivec3(0,-1,-1), vec4(1.0));
	imageStore(uTexVol, icoord.zxy + ivec3(0,-1,0), vec4(1.0));
	imageStore(uTexVol, icoord.zxy + ivec3(0,0,-1), vec4(1.0));
	imageStore(uTexVol, icoord.zxy + ivec3(0,0,0), vec4(1.0));	
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
layout (location = 2) uniform vec2 uScale;

layout (location = 0) out vec3 vWorldPos;

void main()
{
	vec4 wolrd_pos = uModelMat * vec4(aPos, 1.0);
	vec3 clip_pos = (wolrd_pos.xyz - uCoverageMin)/(uCoverageMax - uCoverageMin) * 2.0 - 1.0;
	gl_Position = vec4(clip_pos.zx * uScale, clip_pos.y, 1.0);
	vWorldPos = wolrd_pos.xyz;
}
)";


static std::string g_frag_y =
R"(#version 430

layout (location = 0) in vec3 vWorldPos;

layout (location = 3) uniform int uDiv;
layout (binding=1, r8) uniform highp writeonly image3D uTexVol;

void main()
{
	vec3 coord = gl_FragCoord.xyz;
	coord.z = clamp(floor(coord.z * float(uDiv)), 0.0, float(uDiv) - 1.0);
	ivec3 icoord = ivec3(coord);
	
	imageStore(uTexVol, icoord.yzx + ivec3(0,-1,-1), vec4(1.0));
	imageStore(uTexVol, icoord.yzx + ivec3(0,-1,0), vec4(1.0));
	imageStore(uTexVol, icoord.yzx + ivec3(0,0,-1), vec4(1.0));
	imageStore(uTexVol, icoord.yzx + ivec3(0,0,0), vec4(1.0));
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
layout (location = 2) uniform vec2 uScale;

layout (location = 0) out vec3 vWorldPos;

void main()
{
	vec4 wolrd_pos = uModelMat * vec4(aPos, 1.0);
	vec3 clip_pos = (wolrd_pos.xyz - uCoverageMin)/(uCoverageMax - uCoverageMin) * 2.0 - 1.0;
	gl_Position = vec4(clip_pos.xy * uScale, clip_pos.z, 1.0);
	vWorldPos = wolrd_pos.xyz;
}
)";


static std::string g_frag_z =
R"(#version 430

layout (location = 0) in vec3 vWorldPos;

layout (location = 3) uniform int uDiv;
layout (binding=1, r8) uniform highp writeonly image3D uTexVol;

void main()
{
	vec3 coord = gl_FragCoord.xyz;
	coord.z = clamp(floor(coord.z * float(uDiv)), 0.0, float(uDiv) - 1.0);
	ivec3 icoord = ivec3(coord);
	
	imageStore(uTexVol, icoord.xyz + ivec3(0,-1,-1), vec4(1.0));
	imageStore(uTexVol, icoord.xyz + ivec3(0,-1,0), vec4(1.0));
	imageStore(uTexVol, icoord.xyz + ivec3(0,0,-1), vec4(1.0));
	imageStore(uTexVol, icoord.xyz + ivec3(0,0,0), vec4(1.0));	
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
		glm::vec2 scale = glm::vec2(params.divisions.y, params.divisions.z)/glm::vec2(params.divisions.y + 1, params.divisions.z + 1);

		target_x.update_framebuffers(params.divisions.y + 1, params.divisions.z+1);
		target_x.bind_buffer();

		glViewport(0, 0, params.divisions.y + 1, params.divisions.z + 1);
		glUseProgram(m_prog_x->m_id);

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.constant_model->m_id);
		glUniform3fv(0, 1, (float*)&params.coverage_min);
		glUniform3fv(1, 1, (float*)&params.coverage_max);
		glUniform2fv(2, 1, (float*)&scale);
		glUniform1i(3, params.divisions.x);
		glBindImageTexture(1, params.tex_id_volume, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R8);

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
		glm::vec2 scale = glm::vec2(params.divisions.z, params.divisions.x) / glm::vec2(params.divisions.z + 1, params.divisions.x + 1);

		target_y.update_framebuffers(params.divisions.z + 1, params.divisions.x + 1);
		target_y.bind_buffer();

		glViewport(0, 0, params.divisions.z + 1, params.divisions.x + 1);
		glUseProgram(m_prog_y->m_id);

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.constant_model->m_id);
		glUniform3fv(0, 1, (float*)&params.coverage_min);
		glUniform3fv(1, 1, (float*)&params.coverage_max);
		glUniform2fv(2, 1, (float*)&scale);
		glUniform1i(3, params.divisions.y);
		glBindImageTexture(1, params.tex_id_volume, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R8);

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
		glm::vec2 scale = glm::vec2(params.divisions.x, params.divisions.y) / glm::vec2(params.divisions.x + 1, params.divisions.y + 1);

		target_z.update_framebuffers(params.divisions.x + 1, params.divisions.y + 1);
		target_z.bind_buffer();

		glViewport(0, 0, params.divisions.x + 1, params.divisions.y + 1);
		glUseProgram(m_prog_z->m_id);

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.constant_model->m_id);
		glUniform3fv(0, 1, (float*)&params.coverage_min);
		glUniform3fv(1, 1, (float*)&params.coverage_max);
		glUniform2fv(2, 1, (float*)&scale);
		glUniform1i(3, params.divisions.z);
		glBindImageTexture(1, params.tex_id_volume, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R8);

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

