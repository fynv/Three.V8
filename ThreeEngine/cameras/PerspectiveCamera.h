#pragma once

#include "cameras/Camera.h"

class PerspectiveCamera : public Camera
{
public:
	PerspectiveCamera(float fov, float aspect, float z_near, float z_far);
	~PerspectiveCamera();

	float fov, aspect, z_near, z_far;

	void updateProjectionMatrix();
	
};
