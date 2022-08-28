#include "Light.h"

Light::Light()
{

}

Light::~Light()
{

}


void Light::lookAt(const glm::vec3& target)
{
	this->updateWorldMatrix(true, false);
	glm::vec3 position = this->matrixWorld[3];
	glm::mat4 m1 = glm::inverse(glm::lookAt(position, target, this->up));
	this->set_quaternion(m1);
	if (this->parent != nullptr)
	{
		glm::quat q = parent->matrixWorld;
		this->set_quaternion(glm::inverse(q) * this->quaternion);
	}
}

