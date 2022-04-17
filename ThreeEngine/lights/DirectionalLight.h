#pragma once

#include "lights/Light.h"

struct ConstDirectionalLight
{
	glm::vec4 color;
	glm::vec4 direction;
};

class DirectionalLight : public Light
{
public:
	DirectionalLight();
	~DirectionalLight();
	Object3D* target = nullptr;

};