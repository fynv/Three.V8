#include <GL/glew.h>
#include "MeshStandardMaterial.h"

struct ConstMaterial
{
	glm::vec4 color;
	glm::vec4 emissive;
	glm::vec4 specularGlossiness;
	glm::vec2 normalScale;
	float metallicFactor;
	float roughnessFactor;	
	float alphaCutoff;
	int doubleSided;	
};

MeshStandardMaterial::MeshStandardMaterial() : constant_material(sizeof(ConstMaterial), GL_UNIFORM_BUFFER)
{

}

void MeshStandardMaterial::update_uniform()
{
	ConstMaterial constMaterial;
	constMaterial.color = color;
	constMaterial.emissive = glm::vec4(emissive, 1.0f);
	constMaterial.specularGlossiness = glm::vec4(specular, glossinessFactor);
	constMaterial.normalScale = normalScale;
	constMaterial.metallicFactor = metallicFactor;
	constMaterial.roughnessFactor = roughnessFactor;
	constMaterial.alphaCutoff = alphaCutoff;
	constMaterial.doubleSided = doubleSided ? 1 : 0;	

	constant_material.upload(&constMaterial);
}
