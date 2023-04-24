#include <string>
#include <GL/glew.h>
#include "Cube2Octa.h"

static std::string g_compute =
R"(#version 430
layout (location = 0) uniform samplerCube uTexCube;
layout (binding=0, rgba8) uniform highp writeonly image2D uImgOcta;

layout(local_size_x = 8, local_size_y = 8) in;


vec2 signNotZero(in vec2 v)
{
	return vec2((v.x >= 0.0) ? 1.0 : -1.0, (v.y >= 0.0)? 1.0: -1.0);
}

vec3 oct_to_vec3(in vec2 e)
{
	vec3 v = vec3(vec2(e.x, e.y), 1.0 - abs(e.x) - abs(e.y));
	if (v.z < 0.0)
	{
		vec2 tmp = (1.0 - abs(vec2(v.y, v.x))) * signNotZero(vec2(v.x, v.y));
		v.x = tmp.x;
		v.y = tmp.y;
	}
	return normalize(v);
}


void main()
{
	ivec2 size_out = imageSize(uImgOcta);
	ivec2 id_out = ivec3(gl_GlobalInvocationID).xy;
	if (id_out.x>= size_out.x || id_out.y >=size_out.y) return;

	vec2 v2 = (vec2(id_out) + 0.5)/vec2(size_out);
	vec3 dir = oct_to_vec3(v2*2.0 - 1.0);	
	vec3 col = texture(uTexCube, dir).xyz;
	imageStore(uImgOcta, id_out, vec4(col, 1.0));
}
)";

Cube2Octa::Cube2Octa()
{
	GLShader comp_shader(GL_COMPUTE_SHADER, g_compute.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
}

void Cube2Octa::convert(const GLCubemap* cube, GLTexture2D* octa, int width, int height)
{
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	
	glUseProgram(m_prog->m_id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cube->tex_id);
	glUniform1i(0, 0);
	glBindImageTexture(0, octa->tex_id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);

	int groups[2] = {(width + 7) / 8, (height + 7) / 8};
	glDispatchCompute(groups[0], groups[1], 1);
	glUseProgram(0);
}

