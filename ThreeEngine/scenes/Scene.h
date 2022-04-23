#pragma once

#include "core/Object3D.h"
#include "lights/Lights.h"

class Background;
class EnvironmentMap;
class Scene : public Object3D
{
public:
	Background* background = nullptr;
	EnvironmentMap* environmentMap = nullptr;

	bool has_opaque = false;
	bool has_alpha = false;
	Lights lights;
};

