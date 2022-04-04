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

void Camera::lookAt(const glm::vec3& target)
{	
	this->updateWorldMatrix(true, false);
	glm::vec3 position = this->matrixWorld[3];	
	glm::mat4 m1 = glm::inverse(glm::lookAt(position, target, this->up));
	this->set_quaternion(m1);
	if (this->parent != nullptr)
	{
		glm::quat q = parent->matrixWorld;
		this->set_quaternion(q * this->quaternion);
	}
}

glm::vec3 Camera::getWorldDirection()
{
	this->updateWorldMatrix(true, false);
	glm::vec3 z =  -matrixWorld[2];
	return glm::normalize(z);
}