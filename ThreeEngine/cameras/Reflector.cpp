#include <GL/glew.h>
#include "Reflector.h"

Reflector::Reflector()
	: m_constant(sizeof(glm::mat4), GL_UNIFORM_BUFFER)
	, m_target(false, true)
	, m_camera(50.0f, 1.0f, 0.1f, 2000.0f, this)
{


}

Reflector::~Reflector()
{

}

void Reflector::updateConstant()
{
	glm::mat4 mat_inv = glm::inverse(this->matrixWorld);
	m_constant.upload(&mat_inv);
}


