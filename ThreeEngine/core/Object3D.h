#pragma once

#include <string>
#include <vector>
#include <glm.hpp>
#include <gtx/quaternion.hpp>

class Object3D
{
public:
	Object3D();
	virtual ~Object3D();

	std::string name;
	Object3D* parent = nullptr;
	std::vector<Object3D*> children;

	glm::vec3 up = { 0.0f, 1.0f, 0.0f };
	glm::vec3 position;
	glm::vec3 rotation;
	glm::quat quaternion;
	glm::vec3 scale;

	void set_rotation(const glm::vec3& rotation);
	void set_quaternion(const glm::quat& quaternion);

	glm::mat4 matrix;
	glm::mat4 matrixWorld;
	bool matrixWorldNeedsUpdate = false;

	void updateMatrix();
	virtual void updateMatrixWorld(bool force);
	virtual void updateWorldMatrix(bool updateParents, bool updateChildren);
	
	Object3D& applyMatrix4(const glm::mat4& matrix);
	Object3D& applyQuaternion(const glm::quat& q);
	void setRotationFromAxisAngle(const glm::vec3 axis, float angle);
	void setRotationFromMatrix(const glm::mat4& m);
	
	Object3D& rotateOnAxis(const glm::vec3& axis, float angle);
	Object3D& rotateOnWorldAxis(const glm::vec3& axis, float angle);
	Object3D& rotateX(float angle);
	Object3D& rotateY(float angle);
	Object3D& rotateZ(float angle);

	Object3D& translateOnAxis(const glm::vec3& axis, float distance);
	Object3D& translateX(float distance);
	Object3D& translateY(float distance);
	Object3D& translateZ(float distance);

	glm::vec3 localToWorld(const glm::vec3& vector) const;
	glm::vec3 worldToLocal(const glm::vec3& vector) const;

	virtual void lookAt(const glm::vec3& target);

	Object3D& add(Object3D* object);
	Object3D& remove(Object3D* object);
	Object3D& removeFromParent();

	Object3D& clear();

	Object3D* getObjectByName(const char* name);
	
	glm::vec3 getWorldPosition();
	glm::quat getWorldQuaternion();
	glm::vec3 getWorldScale();
	virtual glm::vec3 getWorldDirection();

	template <typename TCallBack>
		void traverse(TCallBack callback)
	{
		callback(this);
		for (size_t i = 0; i < children.size(); i++)
		{
			children[i]->traverse(callback);
		}
	}
};
