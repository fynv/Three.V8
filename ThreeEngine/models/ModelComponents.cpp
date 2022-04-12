#include "ModelComponents.h"

struct ModelConst
{
	glm::mat4 ModelMat;
	glm::mat4 NormalMat;
};

Mesh::Mesh()
{
	model_constant = std::unique_ptr<GLDynBuffer>(new GLDynBuffer(sizeof(ModelConst)));
}

