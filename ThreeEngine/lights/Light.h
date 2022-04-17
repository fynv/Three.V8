#pragma once

#include "core/Object3D.h"

class Light : public Object3D
{
public:
	Light();
	~Light();

	glm::vec3 color = { 1.0f, 1.0f, 1.0f };
	float intensity = 1.0f;

	virtual void lookAt(const glm::vec3& target) override;
};

