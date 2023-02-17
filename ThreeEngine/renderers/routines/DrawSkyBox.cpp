#include <string>
#include <GL/glew.h>
#include "DrawSkyBox.h"


static std::string g_vertex =
R"(#version 430
layout (std140, binding = 0) uniform Camera
{
    mat4 uProjMat;
    mat4 uViewMat;	
    mat4 uInvProjMat;
    mat4 uInvViewMat;	
    vec3 uEyePos;
};

layout (location = 0) out vec3 vCubeMapCoord;
void main()
{
    vec2 grid = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    vec2 pos_proj = grid * vec2(2.0, 2.0) + vec2(-1.0, -1.0);
    gl_Position = vec4(pos_proj, 1.0, 1.0);
    vec4 pos_view = uInvProjMat * gl_Position;
    pos_view = pos_view/pos_view.w;
    vCubeMapCoord = (mat3(uInvViewMat) * pos_view.xyz);
};
)";

static std::string g_frag =
R"(#version 430
layout (location = 0) in vec3 vCubeMapCoord;
layout (location = 0) uniform samplerCube uCubeSky;
layout (location = 0) out vec4 outColor;
void main()
{
    outColor = texture(uCubeSky, vCubeMapCoord);
}
)";


DrawSkyBox::DrawSkyBox()
{
    GLShader vert_shader(GL_VERTEX_SHADER, g_vertex.c_str());
    GLShader frag_shader(GL_FRAGMENT_SHADER, g_frag.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(vert_shader, frag_shader));
}

void DrawSkyBox::render(const GLDynBuffer* constant_camera, const GLCubemap* cubemap)
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);

    glUseProgram(m_prog->m_id);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, constant_camera->m_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->tex_id);
    glUniform1i(0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);   
    glUseProgram(0);
}

