#pragma once

#include <glm.hpp>
#include "renderers/GLUtils.h"

class DirectionalLight;
class DirectionalLightShadow
{
public:
	DirectionalLightShadow(DirectionalLight* light, int map_width, int map_height);
	~DirectionalLightShadow();

	DirectionalLight* m_light;
	int m_map_width, m_map_height;

	unsigned m_lightTex;
	unsigned m_lightFBO;

	glm::mat4 m_light_proj_matrix;
	void setProjection(float left, float right, float bottom, float top, float zNear, float zFar);
	
	GLDynBuffer constant_shadow;
	glm::mat4 m_lightVPSBMatrix;

	void updateMatrices();

};