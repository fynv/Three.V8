#include <GL/glew.h>
#include "MeshStandardMaterial.h"

struct ConstMaterial
{
	glm::vec4 color;
	glm::vec2 normalScale;
	float metallicFactor;
	float roughnessFactor;
	float alphaCutoff;
};

MeshStandardMaterial::MeshStandardMaterial() : constant_material(sizeof(ConstMaterial), GL_UNIFORM_BUFFER)
{

}

void MeshStandardMaterial::update_uniform()
{
	ConstMaterial constMaterial;
	constMaterial.color = glm::vec4(color, 1.0f);
	constMaterial.normalScale = normalScale;
	constMaterial.metallicFactor = metallicFactor;
	constMaterial.roughnessFactor = roughnessFactor;
	constMaterial.alphaCutoff = alphaCutoff;
	constant_material.upload(&constMaterial);
}
