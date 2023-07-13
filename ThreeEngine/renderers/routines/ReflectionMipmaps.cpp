#include <string>	
#include <GL/glew.h>
#include "ReflectionMipmaps.h"


static std::string g_comp =
R"(#version 430

layout(location = 0) uniform sampler2D tex_src;
layout (location = 1) uniform float lod;

layout(binding = 0, rgba16f) uniform image2D tex_dst;

layout(local_size_x = 8, local_size_y = 8) in;

void main()
{
	ivec2 size = imageSize(tex_dst);
	ivec2 id = ivec3(gl_GlobalInvocationID).xy;
	if (id.x>=size.x || id.y>=size.y) return;

	vec2 pixel_size = vec2(1.0,1.0)/vec2(size);

	vec2 uv = (vec2(id)+0.5)/vec2(size);
	vec3 color = vec3(0.0);

	color += 0.0625 * textureLod(tex_src, uv + vec2(-pixel_size.x, -pixel_size.y), lod).xyz;
	color += 0.125 * textureLod(tex_src, uv + vec2(0.0, -pixel_size.y), lod).xyz;
	color += 0.0625 * textureLod(tex_src, uv + vec2(pixel_size.x, -pixel_size.y), lod).xyz;

	color += 0.125 * textureLod(tex_src, uv + vec2(-pixel_size.x, 0.0), lod).xyz;
	color += 0.25 * textureLod(tex_src, uv, lod).xyz;
	color += 0.125 * textureLod(tex_src, uv + vec2(pixel_size.x, 0.0), lod).xyz;

	color += 0.0625 * textureLod(tex_src, uv + vec2(-pixel_size.x, pixel_size.y), lod).xyz;
	color += 0.125 * textureLod(tex_src, uv + vec2(0.0, pixel_size.y), lod).xyz;
	color += 0.0625 * textureLod(tex_src, uv + vec2(pixel_size.x, pixel_size.y), lod).xyz;	

	imageStore(tex_dst, id, vec4(color,1.0));
}
)";


ReflectionMipmaps::ReflectionMipmaps()
{
	GLShader comp_shader(GL_COMPUTE_SHADER, g_comp.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
}

void ReflectionMipmaps::downsample(unsigned tex, int level, int width, int height)
{
	glUseProgram(m_prog->m_id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(0, 0);
	glUniform1f(1, (float)level);

	glBindImageTexture(0, tex, level+1, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);

	glDispatchCompute((width + 7) / 8, (height + 7) / 8, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

}


