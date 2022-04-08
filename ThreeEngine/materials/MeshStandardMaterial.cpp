#include <GL/glew.h>
#include "MeshStandardMaterial.h"

struct ConstMaterial
{
	glm::vec4 color;
	float metallicFactor;
	float roughnessFactor;
};

MeshStandardMaterial::MeshStandardMaterial() : constant_material(sizeof(ConstMaterial), GL_UNIFORM_BUFFER)
{

}

void MeshStandardMaterial::update_uniform()
{
	ConstMaterial constMaterial;
	constMaterial.color = glm::vec4(color, 1.0f);
	constMaterial.metallicFactor = metallicFactor;
	constMaterial.roughnessFactor = roughnessFactor;
	constant_material.upload(&constMaterial);
}
