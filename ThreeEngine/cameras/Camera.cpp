#include "Camera.h"

Camera::Camera()
	: matrixWorldInverse(glm::identity<glm::mat4>())
	, projectionMatrix(glm::identity<glm::mat4>())
	, projectionMatrixInverse(glm::identity<glm::mat4>())
{

}

Camera::~Camera()
{

}

void Camera::updateMatrixWorld(bool force)
{
	Object3D::updateMatrixWorld(force);
	matrixWorldInverse = glm::inverse(matrixWorld);
}

void Camera::updateWorldMatrix(bool updateParents, bool updateChildren)
{
	Object3D::updateWorldMatrix(updateParents, updateChildren);
	matrixWorldInverse = glm::inverse(matrixWorld);
}

glm::vec3 Camera::getWorldDirection()
{
	this->updateWorldMatrix(true, false);
	glm::vec3 z =  -matrixWorld[2];
	return glm::normalize(z);
}