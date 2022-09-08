#pragma once

#include "core/Object3D.h"
#include "lights/Lights.h"

class Background;
class EnvironmentMap;
class Fog;
class Scene : public Object3D
{
public:
	Background* background = nullptr;
	IndirectLight* indirectLight = nullptr;

	bool has_opaque = false;
	bool has_alpha = false;
	Lights lights;

	Fog* fog = nullptr;
};

