#include <GL/glew.h>
#include "LightmapUpdate.h"
#include "renderers/BVHRenderTarget.h"
#include "renderers/LightmapRayList.h"
#include "models/ModelComponents.h"


static std::string g_compute =
R"(#version 430

layout (location = 0) uniform sampler2D uTexSource;

layout (std140, binding = 0) uniform LightmapRayList
{
	int uTexelBegin;
	int uTexelEnd;
	int uNumRays;
	int uTexelsPerRow;
	int uNumRows;
};

layout (location = 1) uniform usamplerBuffer uValidList;

layout (location = 2) uniform float uMixRate;

layout (binding=0, rgba16f) uniform image2D uOut;

layout(local_size_x = 64) in;

void main()
{
	int idx_texel_in = 	int(gl_GlobalInvocationID.x);
	int idx_texel_out = idx_texel_in + uTexelBegin;
	if (idx_texel_out >= uTexelEnd) return;

	vec4 col = vec4(0.0);
	for (int i=0; i<uNumRays; i++)
	{
		int x_in = (idx_texel_in % uTexelsPerRow) * uNumRays + i;
		int y_in = idx_texel_in / uTexelsPerRow;
		vec4 col_in = texelFetch(uTexSource, ivec2(x_in, y_in),0);
		col+=col_in;
	}
	col/=float(uNumRays);
	
	ivec2 texel_coord = ivec2(texelFetch(uValidList, idx_texel_out).xy);

	if (uMixRate<1.0)
	{
		vec4 last = imageLoad(uOut, texel_coord);
		col = uMixRate * col  + (1.0 - uMixRate) * last;
	}

	imageStore(uOut, texel_coord, col);
}
)";

LightmapUpdate::LightmapUpdate()
{
	GLShader comp_shader(GL_COMPUTE_SHADER, g_compute.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
}


void LightmapUpdate::update(const RenderParams& params)
{
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	const BVHRenderTarget* source = params.source;
	const LightmapRayList* lmrl = params.lmrl;

	glUseProgram(m_prog->m_id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, source->m_tex_video->tex_id);
	glUniform1i(0, 0);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, lmrl->m_constant.m_id);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, lmrl->source->valid_list->tex_id);
	glUniform1i(1, 1);

	glUniform1f(2, params.mix_rate);

	glBindImageTexture(0, params.target->lightmap->tex_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);

	int num_texels = lmrl->end - lmrl->begin;
	int num_blocks = (num_texels + 63) / 64;
	glDispatchCompute(num_blocks, 1, 1);

	glUseProgram(0);
}
