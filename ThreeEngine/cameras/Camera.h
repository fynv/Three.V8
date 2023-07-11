#pragma once

#include "core/Object3D.h"
#include "renderers/GLUtils.h"

struct Scissor
{
	glm::vec2 min_proj = glm::vec2(-1.0f, -1.0f);
	glm::vec2 max_proj = glm::vec2(1.0f, 1.0f);
	glm::ivec2 origin = glm::ivec2(-1,-1);
	glm::ivec2 size = glm::ivec2(-1, -1);
};


class Reflector;
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

	Reflector* reflector = nullptr;

	GLBuffer m_constant;
	GLBuffer m_constant_scissor;
	void updateConstant();

	Scissor m_scissor;
};
