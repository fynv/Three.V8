#include <cmath>
#include "PerspectiveCamera.h"

const double PI = 3.14159265359;
const double DEG2RAD = PI / 180.0;

PerspectiveCamera::PerspectiveCamera(float fov, float aspect, float z_near, float z_far): fov(fov), aspect(aspect), z_near(z_near), z_far(z_far)
{
	updateProjectionMatrix();
}

PerspectiveCamera::~PerspectiveCamera()
{

}

void PerspectiveCamera::updateProjectionMatrix()
{
	projectionMatrix = glm::perspective(fov * (float)DEG2RAD, aspect, z_near, z_far);
	projectionMatrixInverse = glm::inverse(projectionMatrix);
}