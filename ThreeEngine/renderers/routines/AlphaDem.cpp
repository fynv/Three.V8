#include <string>
#include <GL/glew.h>
#include "AlphaDem.h"


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
layout (location = 0) out vec4 outColor;
layout (location = 0) uniform sampler2D uTex;
void main()
{
	vec4 col = texture(uTex, vUV);
	if(col.w>0.0)
	{
		col.xyz/=col.w;
	}	
	outColor = vec4(col.xyz, col.w);
}
)";


AlphaDem::AlphaDem()
{
	GLShader vert_shader(GL_VERTEX_SHADER, g_vertex.c_str());
	GLShader frag_shader(GL_FRAGMENT_SHADER, g_frag.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(vert_shader, frag_shader));
}

void AlphaDem::render(unsigned tex_id, int x, int y, int width, int height)
{
	glViewport(x, y, width, height);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glUseProgram(m_prog->m_id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glUniform1i(0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

}