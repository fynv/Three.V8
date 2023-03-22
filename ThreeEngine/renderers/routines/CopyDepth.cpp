#include <vector>
#include <string>
#include <GL/glew.h>
#include "CopyDepth.h"

static std::string g_vertex =
R"(#version 430
void main()
{
	vec2 grid = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	vec2 vpos = grid * vec2(2.0, 2.0) + vec2(-1.0, -1.0);
	gl_Position = vec4(vpos, 0.0, 1.0);
}
)";

static std::string g_frag =
R"(#version 430

layout (location = 0) uniform sampler2D uDepthTex;
void main()
{
	ivec2 coord = ivec2(gl_FragCoord.xy);	
	gl_FragDepth = texelFetch(uDepthTex, coord, 0).x;
}
)";


CopyDepth::CopyDepth() 
{
	GLShader vert_shader(GL_VERTEX_SHADER, g_vertex.c_str());
	GLShader frag_shader(GL_FRAGMENT_SHADER, g_frag.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(vert_shader, frag_shader));
}


void CopyDepth::render(unsigned tex_depth_in)
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_TRUE);
	glDisable(GL_CULL_FACE);

	glUseProgram(m_prog->m_id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_depth_in);
	glUniform1i(0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

}