#include <GL/glew.h>
#include "Model.h"

struct ModelConst
{
	glm::mat4 ModelMat;
	glm::mat4 NormalMat;
};


Model::Model() : m_constant(sizeof(ModelConst), GL_UNIFORM_BUFFER)
{

}

void Model::updateConstant()
{
	ModelConst c;
	c.ModelMat = matrixWorld;
	c.NormalMat = glm::transpose(glm::inverse(matrixWorld));
	m_constant.upload(&c);
}
