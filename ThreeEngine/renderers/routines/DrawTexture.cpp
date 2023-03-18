#include <vector>
#include <string>
#include <GL/glew.h>
#include "DrawTexture.h"

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

static std::string g_vertex_flip =
R"(#version 430
layout (location = 0) out vec2 vUV;
void main()
{
	vec2 grid = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	vec2 vpos = grid * vec2(2.0, 2.0) + vec2(-1.0, -1.0);
	gl_Position = vec4(vpos, 1.0, 1.0);
	vUV = vec2(grid.x, 1.0 - grid.y);
}
)";


static std::string g_frag =
R"(#version 430
layout (location = 0) in vec2 vUV;
layout (location = 0) out vec4 outColor;
layout (location = 0) uniform sampler2D uTex;
layout (location = 1) uniform vec2 kb;
layout (location = 2) uniform float alpha;
void main()
{
	vec4 col = texture(uTex, vUV);
	col.xyz = col.xyz*kb.x + kb.y;
	col.w *= alpha;
	outColor = vec4(col.xyz*col.w, col.w);
}
)";


static std::string g_frag_premult =
R"(#version 430
layout (location = 0) in vec2 vUV;
layout (location = 0) out vec4 outColor;
layout (location = 0) uniform sampler2D uTex;
layout (location = 1) uniform vec2 kb;
layout (location = 2) uniform float alpha;
void main()
{
	vec4 col = texture(uTex, vUV);
	col.xyz = col.xyz*kb.x + kb.y;
	outColor = col * alpha;
}
)";


DrawTexture::DrawTexture(bool premult, bool flipY, bool auto_contrast) : m_auto_contrast(auto_contrast)
{
	std::string s_vertex = flipY ? g_vertex_flip : g_vertex;
	std::string s_frag = premult ? g_frag_premult : g_frag;
	GLShader vert_shader(GL_VERTEX_SHADER, s_vertex.c_str());
	GLShader frag_shader(GL_FRAGMENT_SHADER, s_frag.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(vert_shader, frag_shader));
}

inline float to_linear(float lum)
{
	if (lum < 0.04045f) return lum / 12.92f;
	else return powf((lum + 0.055f) / 1.055f, 2.4f);
}

void DrawTexture::render(unsigned tex_id, int x, int y, int width, int height, bool blending, float alpha)
{
	float k = 1.0f;
	float b = 0.0f;
	if (m_auto_contrast)
	{
		glBindTexture(GL_TEXTURE_2D, tex_id);
		glGenerateMipmap(GL_TEXTURE_2D);

		int w, h;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 3, GL_TEXTURE_WIDTH, &w);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 3, GL_TEXTURE_HEIGHT, &h);

		std::vector<uint8_t> rgba(w * h * 4);
		glGetTexImage(GL_TEXTURE_2D, 4, GL_RGB, GL_UNSIGNED_BYTE, rgba.data());
		glBindTexture(GL_TEXTURE_2D, 0);

		float sum = 0.0f;
		float sum2 = 0.0f;
		for (size_t i = 0; i < w * h; i++)
		{
			float r = (float)rgba[i * 4] / 255.0f;
			float g = (float)rgba[i * 4 + 1] / 255.0f;
			float b = (float)rgba[i * 4 + 2] / 255.0f;

			float lum = 0.2126f * r + 0.7152f*g + 0.0722f*b;
			lum = to_linear(lum);
			sum += lum;
			sum2 += lum * lum;
		}

		float ave = sum / (float)(w * h);
		float std = sqrtf(sum2 / (float)(w * h) - ave * ave);
		
		k = sqrtf(1.0f / 12.0f) / std;
		b = (1.0f - k)*ave;
		
	}

	glViewport(x, y, width, height);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	if (blending)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	glUseProgram(m_prog->m_id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glUniform1i(0, 0);
	glUniform2f(1, k, b);
	glUniform1f(2, alpha);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);

}