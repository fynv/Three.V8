#pragma once

#include "core/Object3D.h"

class Background;
class Scene : public Object3D
{
public:
	Background* background = nullptr;
};

