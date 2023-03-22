#include <GL/glew.h>
#include <gtc/matrix_transform.hpp>
#include "DirectionalLight.h"
#include "DirectionalLightShadow.h"

DirectionalLightShadow::DirectionalLightShadow(DirectionalLight* light)
	: m_light(light)	
	, constant_shadow(sizeof(ConstDirectionalShadow), GL_UNIFORM_BUFFER)
{

	
}

DirectionalLightShadow::~DirectionalLightShadow()
{
	if (m_lightTex != (unsigned)(-1))
	{
		glDeleteTextures(1, &m_lightTex);
	}
	if (m_lightFBO != (unsigned)(-1))
	{
		glDeleteFramebuffers(1, &m_lightFBO);
	}
	if (m_lightTex_building != (unsigned)(-1))
	{
		glDeleteTextures(1, &m_lightTex_building);
	}
	if (m_lightFBO_building != (unsigned)(-1))
	{
		glDeleteFramebuffers(1, &m_lightFBO_building);
	}
}

bool DirectionalLightShadow::update_shadowmap(int width, int height)
{
	if (width != m_map_width || height != m_map_height)
	{
		if (m_lightTex != (unsigned)(-1))
		{
			glDeleteTextures(1, &m_lightTex);
		}
		if (m_lightFBO != (unsigned)(-1))
		{
			glDeleteFramebuffers(1, &m_lightFBO);
		}

		glGenFramebuffers(1, &m_lightFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_lightFBO);

		glGenTextures(1, &m_lightTex);
		glBindTexture(GL_TEXTURE_2D, m_lightTex);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_lightTex, 0);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		m_map_width = width;
		m_map_height = height;
		return true;
	}
	return false;

}

bool DirectionalLightShadow::update_building_map(int width, int height)
{
	if (width != m_building_map_width || height != m_building_map_height)
	{
		if (m_lightTex_building != (unsigned)(-1))
		{
			glDeleteTextures(1, &m_lightTex_building);
		}
		if (m_lightFBO_building != (unsigned)(-1))
		{
			glDeleteFramebuffers(1, &m_lightFBO_building);
		}

		glGenFramebuffers(1, &m_lightFBO_building);
		glBindFramebuffer(GL_FRAMEBUFFER, m_lightFBO_building);

		glGenTextures(1, &m_lightTex_building);
		glBindTexture(GL_TEXTURE_2D, m_lightTex_building);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_lightTex_building, 0);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		m_building_map_width = width;
		m_building_map_height = height;
		return true;
	}
	return false;
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
