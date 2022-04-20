#pragma once

#include <memory>
#include "lights/Light.h"

struct ConstDirectionalLight
{
	glm::mat4 shadowVPSBMatrix;
	glm::vec4 color;
	glm::vec4 direction;
	int has_shadow;
	int padding[3];
};

class DirectionalLightShadow;
class DirectionalLight : public Light
{
public:
	DirectionalLight();
	~DirectionalLight();
	Object3D* target = nullptr;

	std::unique_ptr<DirectionalLightShadow> shadow;
	void setShadow(bool enable, int map_width, int map_height);
	void setShadowProjection(float left, float right, float bottom, float top, float zNear, float zFar);

	glm::vec3 direction();
	void makeConst(ConstDirectionalLight& const_light);

};