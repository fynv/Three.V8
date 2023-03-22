#include <GL/glew.h>
#include "DirectionalLight.h"
#include "DirectionalLightShadow.h"

DirectionalLight::DirectionalLight()
	: m_constant(sizeof(ConstDirectionalLight), GL_UNIFORM_BUFFER)
{

}

DirectionalLight::~DirectionalLight()
{

}

void DirectionalLight::setShadow(bool enable, int map_width, int map_height)
{
	if (enable)
	{
		shadow = std::unique_ptr<DirectionalLightShadow>(new DirectionalLightShadow(this));
		shadow->update_shadowmap(map_width, map_height);
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

void DirectionalLight::SetShadowRadius(float radius)
{
	if (shadow != nullptr)
	{
		shadow->m_light_radius = radius;
	}
}

glm::vec3 DirectionalLight::direction()
{
	this->updateWorldMatrix(true, false);
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
	const_light.origin = glm::vec4(position, 1.0f);
	const_light.direction = glm::vec4(direction(), 0.0f);
	const_light.has_shadow = shadow != nullptr ? 1 : 0;
	const_light.diffuseThresh = diffuse_thresh;
	const_light.diffuseHigh = diffuse_high;
	const_light.diffuseLow = diffuse_low;
	const_light.specularThresh = specular_thresh;
	const_light.specularHigh = specular_high;
	const_light.specularLow = specular_low;
}

void DirectionalLight::updateConstant()
{
	ConstDirectionalLight c;
	makeConst(c);
	m_constant.upload(&c);
}

void DirectionalLight::lookAtTarget()
{
	glm::vec3 abs_dir = glm::abs(direction());
	if (abs_dir.y < abs_dir.x)
	{
		if (abs_dir.z < abs_dir.y)
		{
			this->up = { 0.0f, 0.0f, 1.0f };
		}
		else
		{
			this->up = { 0.0f, 1.0f, 0.0f };
		}
	}
	else if (abs_dir.z < abs_dir.x)
	{
		this->up = { 0.0f, 0.0f, 1.0f };
	}
	else
	{
		this->up = { 1.0f, 0.0f, 0.0f };
	}

	glm::vec3 pos_target = { 0.0f, 0.0f, 0.0f };
	if (target != nullptr)
	{
		pos_target = target->matrixWorld[3];
	}
	lookAt(pos_target);
}

