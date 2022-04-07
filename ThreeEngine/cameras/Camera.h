#pragma once

#include "core/Object3D.h"
#include "renderers/GLUtils.h"

class Camera : public Object3D
{
public:
	Camera();
	~Camera();
	
	glm::mat4 matrixWorldInverse;
	glm::mat4 projectionMatrix;
	glm::mat4 projectionMatrixInverse;	

	virtual void updateMatrixWorld(bool force) override;
	virtual void updateWorldMatrix(bool updateParents, bool updateChildren) override;
	virtual void lookAt(const glm::vec3& target) override;
	virtual glm::vec3 getWorldDirection() override;

	GLDynBuffer m_constant;
	void updateConstant();
};