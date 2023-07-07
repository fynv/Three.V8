#include <string>
#include <GL/glew.h>
#include "DepthDownsample.h"

static std::string g_vertex =
R"(#version 430
layout (location = 0) out vec2 vUV;
void main()
{
	vec2 grid = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	vec2 vpos = grid * vec2(2.0, 2.0) + vec2(-1.0, -1.0);
	gl_Position = vec4(vpos, 1.0, 1.0);
	vUV = vec2(grid.x, grid.y);
}
)";

static std::string g_frag =
R"(#version 430

layout (location = 0) in vec2 vUV;
layout (location = 0) uniform sampler2DMS uDepthTex;
layout (location = 0) out vec4 outColor;

void main()
{
	ivec2 coord = ivec2(gl_FragCoord.xy);	
	float depth0 =  texelFetch(uDepthTex, coord, 0).x;
	float depth1 =  texelFetch(uDepthTex, coord, 1).x;
	float depth2 =  texelFetch(uDepthTex, coord, 2).x;
	float depth3 =  texelFetch(uDepthTex, coord, 3).x;
	float depth = 0.25 * (depth0 + depth1 + depth2 + depth3);	
	outColor = vec4(vec3(depth), 1.0);
}
)";

DepthDownsample::DepthDownsample()
{
	GLShader vert_shader(GL_VERTEX_SHADER, g_vertex.c_str());
	GLShader frag_shader(GL_FRAGMENT_SHADER, g_frag.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(vert_shader, frag_shader));
}


void DepthDownsample::render(unsigned tex_depth_4x)
{
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	glUseProgram(m_prog->m_id);	
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex_depth_4x);
	glUniform1i(0, 0);

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glUseProgram(0);
}
