#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <string>
#include "ReflectionDistanceCreator.h"
#include "renderers/GLRenderTarget.h"

static std::string g_comp =
R"(#version 430
layout (location = 0) uniform sampler2D uDepthTex;
layout (location = 1) uniform mat4 uInvProjMat;

layout (binding=0, r32f) uniform highp writeonly image2D uDisImg;

layout(local_size_x = 8, local_size_y = 8) in;

void main()
{
	ivec2 size = textureSize(uDepthTex, 0);
	ivec2 id = ivec3(gl_GlobalInvocationID).xy;
	if (id.x>= size.x || id.y >=size.y) return;
	vec2 UV = (vec2(id)+0.5)/vec2(size);
	float depth = texelFetch(uDepthTex, id, 0).x;
	vec3 pos_clip = vec3(UV, depth)*2.0-1.0;
	vec4 view_pos = uInvProjMat* vec4(pos_clip, 1.0);
	view_pos/=view_pos.w;
	float dis = length(view_pos.xyz);
	imageStore(uDisImg, id, vec4(dis));
}

)";

ReflectionDistanceCreator::ReflectionDistanceCreator()
{
	GLShader comp(GL_COMPUTE_SHADER, g_comp.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(comp));
}

ReflectionDistanceCreator::~ReflectionDistanceCreator()
{

}

const double PI = 3.14159265359;

void ReflectionDistanceCreator::Create(const CubeRenderTarget* target, ReflectionMap* reflection, float z_near, float z_far)
{	
	glm::mat4 projectionMatrix = glm::perspective((float)PI * 0.5f, 1.0f, z_near, z_far);
	glm::mat4 projectionMatrixInverse = glm::inverse(projectionMatrix);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glUseProgram(m_prog->m_id);
	for (int i = 0; i < 6; i++)
	{
		GLRenderTarget* face = target->m_faces[i].get();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, face->m_tex_depth->tex_id);
		glUniform1i(0, 0);
		glUniformMatrix4fv(1, 1, GL_FALSE, (float*)&projectionMatrixInverse);		
		glBindImageTexture(0, reflection->tex_id_dis, 0, GL_FALSE, i, GL_WRITE_ONLY, GL_R32F);		

		glm::ivec2 blocks = { (target->m_width + 7) / 8, (target->m_height + 7) / 8 };
		glDispatchCompute(blocks.x, blocks.y, 1);		
	}
	glUseProgram(0);	
}

