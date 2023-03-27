#include <GL/glew.h>
#include <glm.hpp>
#include "LightmapFilter.h"

static std::string g_compute =
R"(#version 430

layout (location = 0) uniform sampler2D uTexSource;
layout (location = 1) uniform sampler2D uTexPosition;
layout (binding=0, rgba16f) uniform image2D uOut;

layout (location = 2) uniform float uTexelSize;

layout(local_size_x = 8, local_size_y = 8) in;

void main()
{
	ivec2 size = imageSize(uOut);
	ivec2 id = ivec3(gl_GlobalInvocationID).xy;	
	if (id.x>= size.x || id.y >=size.y) return;

	vec4 pos0 = texelFetch(uTexPosition, id, 0);
	if (pos0.w < 0.5) return;

	vec3 acc_col = vec3(0.0);
	float acc_weight = 0.0;
	
	for (int dy = -1; dy<=1; dy++)
	{
		for (int dx = -1; dx<=1; dx++)
		{			
			ivec2 id1 = id + ivec2(dx, dy);
			vec4 pos1 =  texelFetch(uTexPosition, id1, 0);
			if (pos1.w < 0.5) continue;

			float k = length(pos1.xyz - pos0.xyz)/uTexelSize;
			float w = pow(0.5, k);
			if (w < 0.001) continue;

			vec3 col = texelFetch(uTexSource, id1, 0).xyz;
			acc_col += col * w;
			acc_weight += w;	
		}
	}
	
	imageStore(uOut, id, vec4(acc_col/acc_weight, 1.0));
}
)";

LightmapFilter::LightmapFilter()
{
	GLShader comp_shader(GL_COMPUTE_SHADER, g_compute.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
}

void LightmapFilter::filter(const RenderParams& params)
{
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	int width = params.width;
	int height = params.height;

	glUseProgram(m_prog->m_id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, params.light_map_in->tex_id);
	glUniform1i(0, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, params.atlas_position->tex_id);
	glUniform1i(1, 1);

	glBindImageTexture(0, params.light_map_out->tex_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);

	glUniform1f(2, params.texel_size);

	glm::ivec2 blocks = { (width + 7) / 8, (height + 7) / 8 };
	glDispatchCompute(blocks.x, blocks.y, 1);

	glUseProgram(0);
}