#include <GL/glew.h>
#include <string>
#include "models/ModelComponents.h"
#include "DrawWire.h"

static std::string g_vertex =
R"(#version 430
layout (location = 0) in vec3 aPos;
layout (std140, binding = 0) uniform Camera
{
	mat4 uProjMat;
	mat4 uViewMat;	
	mat4 uInvProjMat;
	mat4 uInvViewMat;	
	vec3 uEyePos;
};

layout (std140, binding = 1) uniform Model
{
	mat4 uModelMat;
	mat4 uNormalMat;
};

layout (location = 1) uniform float uRadius;

void main()
{
	vec4 wolrd_pos = uModelMat * vec4(aPos, 1.0);
	vec4 view_pos = uViewMat*wolrd_pos;
	view_pos.xyz *= 1.0 + 0.01*uRadius;
	gl_Position = uProjMat*view_pos;
}
)";

static std::string g_geo =
R"(#version 430
layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;
layout (location = 0) uniform vec2 uViewport;
layout (location = 1) uniform float uRadius;
void main()
{
	vec2 pos0_screen = gl_in[0].gl_Position.xy/ gl_in[0].gl_Position.w * uViewport *0.5;
	vec2 pos1_screen = gl_in[1].gl_Position.xy/ gl_in[1].gl_Position.w * uViewport *0.5;
	float z0 = gl_in[0].gl_Position.z/ gl_in[0].gl_Position.w;
	float z1 = gl_in[1].gl_Position.z/ gl_in[1].gl_Position.w;
	float radius = uRadius;
	vec2 norm = normalize(pos1_screen - pos0_screen);
	vec2 norm2 = vec2(-norm.y, norm.x);
	vec2 pos_out;
	pos_out = pos0_screen + (-norm + norm2)*radius;
	gl_Position = vec4(pos_out*2.0/uViewport, z0, 1.0);
	EmitVertex();
	pos_out = pos0_screen + (-norm - norm2)*radius;
	gl_Position = vec4(pos_out*2.0/uViewport, z0, 1.0);
	EmitVertex();
	pos_out = pos1_screen + (norm + norm2)*radius;
	gl_Position = vec4(pos_out*2.0/uViewport, z1, 1.0);
	EmitVertex();
	pos_out = pos1_screen + (norm - norm2)*radius;
	gl_Position = vec4(pos_out*2.0/uViewport, z1, 1.0);
	EmitVertex();
	EndPrimitive();
}
)";

static std::string g_frag =
R"(#version 430
out vec4 outColor;
void main()
{
    outColor = vec4(0.0,0.0,0.0, 1.0);
}
)";

DrawWire::DrawWire()
{
	m_vert_shader = std::unique_ptr<GLShader>(new GLShader(GL_VERTEX_SHADER, g_vertex.c_str()));
	m_geo_shader = std::unique_ptr<GLShader>(new GLShader(GL_GEOMETRY_SHADER, g_geo.c_str()));
	m_frag_shader = std::unique_ptr<GLShader>(new GLShader(GL_FRAGMENT_SHADER, g_frag.c_str()));
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(*m_vert_shader, *m_geo_shader, *m_frag_shader));
}


void DrawWire::render(const RenderParams& params)
{
	const GeometrySet& geo = params.primitive->geometry[params.primitive->geometry.size() - 1];

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);

	glUseProgram(m_prog->m_id);		
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.constant_camera->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.constant_model->m_id);

	glBindBuffer(GL_ARRAY_BUFFER, geo.pos_buf->m_id);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	GLint i_viewport[4];
	glGetIntegerv(GL_VIEWPORT, i_viewport);
	glm::vec2 viewport = { (float)i_viewport[2], (float)i_viewport[3] };
	glUniform2fv(0, 1, (float*)&viewport);
	glUniform1f(1, params.radius);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, params.primitive->wire_ind_buf->m_id);
	glDrawElements(GL_LINES, params.primitive->num_wires * 2, GL_UNSIGNED_INT, nullptr);	

	glUseProgram(0);
}
