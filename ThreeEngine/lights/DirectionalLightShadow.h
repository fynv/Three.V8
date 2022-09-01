#pragma once

#include <glm.hpp>
#include "renderers/GLUtils.h"

struct ConstDirectionalShadow
{
	glm::mat4 ProjMat;
	glm::mat4 ViewMat;
	glm::vec2 LeftRight;
	glm::vec2 BottomTop;
	glm::vec2 NearFar;
	float LightRadius;
	float Padding;

};

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
	float m_left, m_right, m_bottom, m_top, m_near, m_far;
	float m_light_radius = 0.0f;
	void setProjection(float left, float right, float bottom, float top, float zNear, float zFar);
	
	GLDynBuffer constant_shadow;
	glm::mat4 m_lightVPSBMatrix;

	void makeConst(ConstDirectionalShadow& const_shadow);
	void updateMatrices();

};