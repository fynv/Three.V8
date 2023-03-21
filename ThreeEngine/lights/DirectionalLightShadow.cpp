#include <GL/glew.h>
#include <gtc/matrix_transform.hpp>
#include "DirectionalLight.h"
#include "DirectionalLightShadow.h"

DirectionalLightShadow::DirectionalLightShadow(DirectionalLight* light, int map_width, int map_height)
	: m_light(light)
	, m_map_width(map_width)
	, m_map_height(map_height)
	, constant_shadow(sizeof(ConstDirectionalShadow), GL_UNIFORM_BUFFER)
{

	glGenFramebuffers(1, &m_lightFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_lightFBO);

	glGenTextures(1, &m_lightTex);
	glBindTexture(GL_TEXTURE_2D, m_lightTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, map_width, map_height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_lightTex, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

DirectionalLightShadow::~DirectionalLightShadow()
{
	glDeleteTextures(1, &m_lightTex);
	glDeleteFramebuffers(1, &m_lightFBO);
}

void DirectionalLightShadow::setProjection(float left, float right, float bottom, float top, float zNear, float zFar)
{
	m_left = left;
	m_right = right;
	m_bottom = bottom;
	m_top = top;
	m_near = zNear;
	m_far = zFar;
	m_light_proj_matrix = glm::ortho(left, right, bottom, top, zNear, zFar);
}

void DirectionalLightShadow::makeConst(ConstDirectionalShadow& const_shadow)
{
	glm::mat4 view_matrix = glm::inverse(m_light->matrixWorld);
	const_shadow.VPSBMat = m_lightVPSBMatrix;
	const_shadow.ProjMat = m_light_proj_matrix;
	const_shadow.ViewMat = view_matrix;
	const_shadow.LeftRight = { m_left, m_right };
	const_shadow.BottomTop = { m_bottom, m_top };
	const_shadow.NearFar = { m_near, m_far };
	const_shadow.LightRadius = m_light_radius;
	const_shadow.Bias = m_force_cull? -1.0f: m_bias;
}

void DirectionalLightShadow::updateMatrices()
{
	glm::mat4 view_matrix = glm::inverse(m_light->matrixWorld);
	glm::mat4 lightScale = glm::identity<glm::mat4>();
	lightScale = glm::scale(lightScale, glm::vec3(0.5f, 0.5f, 0.5f));
	glm::mat4 lightBias = glm::identity<glm::mat4>();
	lightBias = glm::translate(lightBias, glm::vec3(0.5f, 0.5f, 0.5f));
	m_lightVPSBMatrix = lightBias * lightScale * m_light_proj_matrix * view_matrix;

	ConstDirectionalShadow constShadow;
	makeConst(constShadow);
	constant_shadow.upload(&constShadow);
}
