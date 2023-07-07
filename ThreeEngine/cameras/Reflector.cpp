#include <GL/glew.h>
#include "Reflector.h"

Reflector::Reflector()
	: m_constant(sizeof(glm::mat4), GL_UNIFORM_BUFFER)
	, m_target(false, true)
	, m_tex_depth_1x(new GLTexture2D)
	, m_camera(50.0f, 1.0f, 0.1f, 2000.0f, this)	
{
	glGenFramebuffers(1, &m_fbo_depth_1x);

}

Reflector::~Reflector()
{
	glDeleteFramebuffers(1, &m_fbo_depth_1x);

}

void Reflector::updateConstant()
{
	glm::mat4 mat_inv = glm::inverse(this->matrixWorld);
	m_constant.upload(&mat_inv);
}

void Reflector::updateTarget(int width, int height)
{
	m_target.update_framebuffers(width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_depth_1x);
	glBindTexture(GL_TEXTURE_2D, m_tex_depth_1x->tex_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex_depth_1x->tex_id, 0);
}

void Reflector::depthDownsample()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_depth_1x);
	m_depth_downsampler.render(m_target.m_tex_depth->tex_id);
}

inline void toViewAABB(const glm::mat4& MV, const glm::vec3& min_pos, const glm::vec3& max_pos, glm::vec3& min_pos_out, glm::vec3& max_pos_out)
{
	glm::vec4 view_pos[8];
	view_pos[0] = MV * glm::vec4(min_pos.x, min_pos.y, min_pos.z, 1.0f);
	view_pos[1] = MV * glm::vec4(max_pos.x, min_pos.y, min_pos.z, 1.0f);
	view_pos[2] = MV * glm::vec4(min_pos.x, max_pos.y, min_pos.z, 1.0f);
	view_pos[3] = MV * glm::vec4(max_pos.x, max_pos.y, min_pos.z, 1.0f);
	view_pos[4] = MV * glm::vec4(min_pos.x, min_pos.y, max_pos.z, 1.0f);
	view_pos[5] = MV * glm::vec4(max_pos.x, min_pos.y, max_pos.z, 1.0f);
	view_pos[6] = MV * glm::vec4(min_pos.x, max_pos.y, max_pos.z, 1.0f);
	view_pos[7] = MV * glm::vec4(max_pos.x, max_pos.y, max_pos.z, 1.0f);

	min_pos_out = { FLT_MAX, FLT_MAX, FLT_MAX };
	max_pos_out = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

	for (int k = 0; k < 8; k++)
	{
		glm::vec4 pos = view_pos[k];
		if (pos.x < min_pos_out.x) min_pos_out.x = pos.x;
		if (pos.x > max_pos_out.x) max_pos_out.x = pos.x;
		if (pos.y < min_pos_out.y) min_pos_out.y = pos.y;
		if (pos.y > max_pos_out.y) max_pos_out.y = pos.y;
		if (pos.z < min_pos_out.z) min_pos_out.z = pos.z;
		if (pos.z > max_pos_out.z) max_pos_out.z = pos.z;
	}
}

void Reflector::calc_scissor()
{
	Scissor& scissor = m_camera.m_scissor;
	scissor.origin = { 0,0 };
	scissor.size = { 0, 0 };

	glm::vec3 min_pos = { -width * 0.5f, -height * 0.5f, 0.0f };
	glm::vec3 max_pos = { width * 0.5f, height * 0.5f, 0.0f };

	glm::mat4 MV = m_camera.matrixWorldInverse * matrixWorld;
	glm::vec3 min_pos_view, max_pos_view;
	toViewAABB(MV, min_pos, max_pos, min_pos_view, max_pos_view);

	glm::mat4 invP = m_camera.projectionMatrixInverse;
	glm::vec4 view_far = invP * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	view_far /= view_far.w;
	glm::vec4 view_near = invP * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
	view_near /= view_near.w;

	if (min_pos_view.z < view_far.z)
	{
		min_pos_view.z = view_far.z;
	}

	if (max_pos_view.z > view_near.z)
	{
		max_pos_view.z = view_near.z;
	}

	if (min_pos_view.z > max_pos_view.z) return;

	glm::mat4 P = m_camera.projectionMatrix;

	glm::vec4 min_pos_proj = P * glm::vec4(min_pos_view.x, min_pos_view.y, min_pos_view.z, 1.0f);
	min_pos_proj /= min_pos_proj.w;

	glm::vec4 max_pos_proj = P * glm::vec4(max_pos_view.x, max_pos_view.y, min_pos_view.z, 1.0f);
	max_pos_proj /= max_pos_proj.w;

	glm::vec4 min_pos_proj2 = P * glm::vec4(min_pos_view.x, min_pos_view.y, max_pos_view.z, 1.0f);
	min_pos_proj2 /= min_pos_proj2.w;

	glm::vec4 max_pos_proj2 = P * glm::vec4(max_pos_view.x, max_pos_view.y, max_pos_view.z, 1.0f);
	max_pos_proj2 /= max_pos_proj2.w;

	scissor.min_proj = glm::min(min_pos_proj, min_pos_proj2);
	scissor.max_proj = glm::max(max_pos_proj, max_pos_proj2);

	if (scissor.min_proj.x < -1.0f) scissor.min_proj.x = -1.0f;
	if (scissor.min_proj.y < -1.0f) scissor.min_proj.y = -1.0f;
	if (scissor.max_proj.x > 1.0f) scissor.max_proj.x = 1.0f;
	if (scissor.max_proj.y > 1.0f) scissor.max_proj.y = 1.0f;

	if (scissor.min_proj.x > scissor.max_proj.x || scissor.min_proj.y > scissor.max_proj.y) return;

	glm::vec2 min_screen = (glm::vec2(scissor.min_proj) + 1.0f) * 0.5f * glm::vec2(m_target.m_width, m_target.m_height);
	glm::vec2 max_screen = (glm::vec2(scissor.max_proj) + 1.0f) * 0.5f * glm::vec2(m_target.m_width, m_target.m_height);

	scissor.origin.x = (int)floorf(min_screen.x);
	scissor.origin.y = (int)floorf(min_screen.y);

	scissor.size.x = (int)ceilf(max_screen.x) - scissor.origin.x;
	scissor.size.y = (int)ceilf(max_screen.y) - scissor.origin.y;
}

