#pragma once

#include "core/Object3D.h"

class Light : public Object3D
{
public:
	Light();
	~Light();

	glm::vec3 color = { 1.0f, 1.0f, 1.0f };
	float intensity = 1.0f;

	// tone_shading
	float diffuse_thresh = 0.2f;
	float diffuse_high = 0.8f;
	float diffuse_low = 0.2f;
	float specular_thresh = 0.2f;
	float specular_high = 0.8f;
	float specular_low = 0.2f;

	virtual void lookAt(const glm::vec3& target) override;
};

