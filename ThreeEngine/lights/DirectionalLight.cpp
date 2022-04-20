#include "DirectionalLight.h"
#include "DirectionalLightShadow.h"

DirectionalLight::DirectionalLight()
{

}

DirectionalLight::~DirectionalLight()
{

}

void DirectionalLight::setShadow(bool enable, int map_width, int map_height)
{
	if (enable)
	{
		shadow = std::unique_ptr<DirectionalLightShadow>(new DirectionalLightShadow(this, map_width, map_height));
	}
	else
	{
		shadow = nullptr;
	}
}

void DirectionalLight::setShadowProjection(float left, float right, float bottom, float top, float zNear, float zFar)
{
	if (shadow != nullptr)
	{
		shadow->setProjection(left, right, bottom, top, zNear, zFar);
	}
}

glm::vec3 DirectionalLight::direction()
{
	glm::vec3 pos_target = { 0.0f, 0.0f, 0.0f };
	if (target != nullptr)
	{
		pos_target = target->matrixWorld[3];
	}
	glm::vec3 position = matrixWorld[3];
	return glm::normalize(position - pos_target);
}

void DirectionalLight::makeConst(ConstDirectionalLight& const_light)
{
	const_light.color = glm::vec4(color * intensity, 1.0f);
	const_light.direction = glm::vec4(direction(), 0.0f);
	if (shadow != nullptr)
	{
		const_light.has_shadow = 1;
		const_light.shadowVPSBMatrix = shadow->m_lightVPSBMatrix;
	}
	else
	{
		const_light.has_shadow = 0;
	}
}
