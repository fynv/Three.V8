#pragma once

#include <glm.hpp>
#include "renderers/GLUtils.h"

struct ConstDirectionalShadow
{
	glm::mat4 VPSBMat;
	glm::mat4 ProjMat;
	glm::mat4 ViewMat;
	glm::vec2 LeftRight;
	glm::vec2 BottomTop;
	glm::vec2 NearFar;
	float LightRadius;
	float Bias;

};

class DirectionalLight;
class DirectionalLightShadow
{
public:
	DirectionalLightShadow(DirectionalLight* light);
	~DirectionalLightShadow();

	DirectionalLight* m_light;

	int m_map_width = -1;
	int m_map_height = -1;
	unsigned m_lightTex = (unsigned)(-1);
	unsigned m_lightFBO = (unsigned)(-1);
	bool update_shadowmap(int width, int height);

	int m_building_map_width = -1;
	int m_building_map_height = -1;
	unsigned m_lightTex_building = (unsigned)(-1);
	unsigned m_lightFBO_building = (unsigned)(-1);
	bool update_building_map(int width, int height);

	glm::mat4 m_light_proj_matrix;
	float m_left, m_right, m_bottom, m_top, m_near, m_far;
	float m_light_radius = 0.0f;
	float m_bias = 0.001f;
	bool m_force_cull = true;
	void setProjection(float left, float right, float bottom, float top, float zNear, float zFar);
	
	GLBuffer constant_shadow;
	glm::mat4 m_lightVPSBMatrix;

	void makeConst(ConstDirectionalShadow& const_shadow);
	void updateMatrices();

};