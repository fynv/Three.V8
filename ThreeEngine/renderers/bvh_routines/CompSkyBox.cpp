#include <string>
#include <glm.hpp>
#include <GL/glew.h>
#include "CompSkyBox.h"
#include "renderers/BVHRenderTarget.h"

static std::string g_compute =
R"(#version 430
layout (location = 0) uniform samplerCube uCubeSky;

layout (binding=0, rgba16f) uniform image2D uOut;

layout(local_size_x = 8, local_size_y = 8) in;

layout (std140, binding = 0) uniform Camera
{
	mat4 uProjMat;
	mat4 uViewMat;	
	mat4 uInvProjMat;
	mat4 uInvViewMat;	
	vec3 uEyePos;
};

void main()
{
	ivec2 size = imageSize(uOut);
	ivec2 id = ivec3(gl_GlobalInvocationID).xy;	
	if (id.x>= size.x || id.y >=size.y) return;

	ivec2 screen = ivec2(id.x, id.y);
	vec4 clip = vec4((vec2(screen) + 0.5)/vec2(size)*2.0-1.0, 0.0, 1.0);
	vec4 view = uInvProjMat * clip; view /= view.w;
	vec3 world = vec3(uInvViewMat*view);
	vec3 dir = normalize(world - uEyePos);

	vec4 outColor = texture(uCubeSky, dir);
	imageStore(uOut, id, outColor);
}

)";

CompSkyBox::CompSkyBox()
{
	GLShader comp_shader(GL_COMPUTE_SHADER, g_compute.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
}

void CompSkyBox::render(const GLDynBuffer* constant_camera, const GLCubemap* cubemap, const BVHRenderTarget* target)
{
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	int width = target->m_width;
	int height = target->m_height;

	glUseProgram(m_prog->m_id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->tex_id);
	glUniform1i(0, 0);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, constant_camera->m_id);

	glBindImageTexture(0, target->m_tex_video->tex_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);

	glm::ivec2 blocks = { (width + 7) / 8, (height + 7) / 8 };
	glDispatchCompute(blocks.x, blocks.y, 1);

	glUseProgram(0);
}

