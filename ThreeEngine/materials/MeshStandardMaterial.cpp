#include <GL/glew.h>
#include "MeshStandardMaterial.h"

struct ConstMaterial
{
	glm::vec4 color;
};

MeshStandardMaterial::MeshStandardMaterial() : constant_material(sizeof(ConstMaterial), GL_UNIFORM_BUFFER)
{

}

void MeshStandardMaterial::update_uniform()
{
	ConstMaterial constMaterial;
	constMaterial.color = glm::vec4(color, 1.0f);
	constant_material.upload(&constMaterial);
}
