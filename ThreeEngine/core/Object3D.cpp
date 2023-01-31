#include "Object3D.h"
#include <gtx/euler_angles.hpp>
#include <gtx/matrix_decompose.hpp>
#include <algorithm>

int Object3D::s_last_id = 0;

Object3D::Object3D()
	: position({ 0.0f, 0.0f, 0.0f })
	, rotation({ 0.0f, 0.0f, 0.0f })
	, quaternion(glm::identity<glm::quat>())
	, scale({ 1.0f, 1.0f, 1.0f })
	, matrix(glm::identity<glm::mat4>())
	, matrixWorld(glm::identity<glm::mat4>())
{
	id = ++s_last_id;
}

Object3D::~Object3D()
{

}

void Object3D::set_rotation(const glm::vec3& rotation)
{
	this->rotation = rotation;
	this->quaternion = glm::eulerAngleXYZ(rotation.x, rotation.y, rotation.z);
}

void Object3D::set_quaternion(const glm::quat& quaternion)
{
	this->quaternion = quaternion;
	glm::extractEulerAngleXYZ(glm::mat4(quaternion), rotation.x, rotation.y, rotation.z);
}

void Object3D::updateMatrix()
{
	this->matrix = glm::identity<glm::mat4>();
	this->matrix = glm::translate(this->matrix, this->position);
	this->matrix *= glm::toMat4(this->quaternion);
	this->matrix = glm::scale(this->matrix, this->scale);
	this->matrixWorldNeedsUpdate = true;
}

void Object3D::updateMatrixWorld(bool force)
{
	this->updateMatrix();
	if (this->matrixWorldNeedsUpdate || force)
	{
		if (this->parent == nullptr)
		{
			this->matrixWorld = this->matrix;
		}
		else
		{
			this->matrixWorld = parent->matrixWorld * this->matrix;
		}
		this->matrixWorldNeedsUpdate = false;
		force = true;
	}

	for (size_t i = 0; i < children.size(); i++)
	{
		children[i]->updateMatrixWorld(force);
	}
}


void Object3D::updateWorldMatrix(bool updateParents, bool updateChildren)
{
	if (updateParents  && parent != nullptr) 
	{
		parent->updateWorldMatrix(true, false);
	}

	this->updateMatrix();

	if (parent == nullptr)
	{
		this->matrixWorld = this->matrix;
	}
	else
	{
		this->matrixWorld = parent->matrixWorld * this->matrix;
	}

	if (updateChildren)
	{
		for (size_t i = 0; i < children.size(); i++)
		{
			children[i]->updateWorldMatrix(false, true);
		}
	}
}


Object3D& Object3D::applyMatrix4(const glm::mat4& matrix)
{
	this->updateMatrix();
	this->matrix = matrix * this->matrix;
	glm::quat rot;
	glm::vec3 skew;
	glm::vec4 persp;
	glm::decompose(this->matrix, this->scale, rot, this->position, skew, persp);
	this->set_quaternion(rot);
	return *this;
}

Object3D& Object3D::applyQuaternion(const glm::quat& q)
{
	this->set_quaternion(q * quaternion);
	return *this;
}

void Object3D::setRotationFromAxisAngle(const glm::vec3 axis, float angle)
{
	this->set_quaternion(glm::angleAxis(angle, axis));
}

void Object3D::setRotationFromMatrix(const glm::mat4& m)
{
	this->set_quaternion(m);
}

Object3D& Object3D::rotateOnAxis(const glm::vec3& axis, float angle)
{
	glm::quat _q1 = glm::angleAxis(angle, axis);
	this->set_quaternion(quaternion*_q1);
	return *this;
}

Object3D& Object3D::rotateOnWorldAxis(const glm::vec3& axis, float angle)
{
	glm::quat _q1 = glm::angleAxis(angle, axis);
	this->set_quaternion(_q1*quaternion);
	return *this;
}

Object3D& Object3D::rotateX(float angle)
{
	return this->rotateOnAxis({ 1.0f, 0.0f, 0.0f }, angle);
}


Object3D& Object3D::rotateY(float angle)
{
	return this->rotateOnAxis({ 0.0f, 1.0f, 0.0f }, angle);
}

Object3D& Object3D::rotateZ(float angle)
{
	return this->rotateOnAxis({ 0.0f, 0.0f, 1.0f }, angle);
}

Object3D& Object3D::translateOnAxis(const glm::vec3& axis, float distance)
{	
	position += quaternion * axis * distance;
	return *this;
}

Object3D& Object3D::translateX(float distance)
{
	return translateOnAxis({ 1.0f, 0.0f, 0.0f }, distance);
}


Object3D& Object3D::translateY(float distance)
{
	return translateOnAxis({ 0.0f, 1.0f, 0.0f }, distance);
}

Object3D& Object3D::translateZ(float distance)
{
	return translateOnAxis({ 0.0f, 0.0f, 1.0f }, distance);
}

glm::vec3 Object3D::localToWorld(const glm::vec3& vector) const
{
	return matrixWorld * glm::vec4(vector, 1.0f);
}

glm::vec3 Object3D::worldToLocal(const glm::vec3& vector) const
{
	return glm::inverse(matrixWorld) * glm::vec4(vector, 1.0f);
}

void Object3D::lookAt(const glm::vec3& target)
{
	this->updateWorldMatrix(true, false);
	glm::vec3 position = this->matrixWorld[3];
	glm::mat4 m1 = glm::inverse(glm::lookAt(target, position, this->up));
	this->set_quaternion(m1);
	if (this->parent != nullptr)
	{
		glm::quat q = parent->matrixWorld;
		this->set_quaternion(glm::inverse(q) * this->quaternion);
	}
}

Object3D& Object3D::add(Object3D* object)
{
	if (object->parent != nullptr)
	{
		object->parent->remove(object);
	}
	object->parent = this;
	this->children.push_back(object);
	return *this;
}

Object3D& Object3D::remove(Object3D* object)
{
	auto iter = std::find(children.begin(), children.end(), object);
	if (iter != children.end())
	{
		object->parent = nullptr;
		children.erase(iter);
	}
	return *this;
}

Object3D& Object3D::removeFromParent()
{
	if (parent != nullptr)
	{
		parent->remove(this);
	}
	return *this;
}

Object3D& Object3D::clear()
{
	for (size_t i = 0; i < children.size(); i++)
	{
		Object3D* object = children[i];
		object->parent = nullptr;
	}
	children.clear();
	return *this;
}

Object3D* Object3D::getObjectByName(const char* name)
{
	if (this->name == name) return this;

	for (size_t i = 0; i < children.size(); i++)
	{
		Object3D* obj = children[i]->getObjectByName(name);
		if (obj != nullptr) return obj;
	}
	return nullptr;
}

glm::vec3 Object3D::getWorldPosition()
{
	this->updateWorldMatrix(true, false);
	return matrixWorld[3];
}


glm::quat Object3D::getWorldQuaternion()
{
	this->updateWorldMatrix(true, false);
	return matrixWorld;
}

glm::vec3 Object3D::getWorldScale()
{
	this->updateWorldMatrix(true, false);
	glm::vec3 scale;
	glm::quat rot;
	glm::vec3 trans;
	glm::vec3 skew;
	glm::vec4 persp;
	glm::decompose(this->matrixWorld, scale, rot, trans, skew, persp);
	return scale;
}

glm::vec3 Object3D::getWorldDirection()
{
	this->updateWorldMatrix(true, false);
	glm::vec3 z = matrixWorld[2];
	return glm::normalize(z);
}

