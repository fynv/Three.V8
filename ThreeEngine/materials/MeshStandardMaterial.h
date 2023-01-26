#pragma once

#include "Material.h"
#include <glm.hpp>
#include "renderers/GLUtils.h"

enum class AlphaMode
{
	Opaque,
	Mask,
	Blend
};

class MeshStandardMaterial : public Material
{
public:
	MeshStandardMaterial();
	AlphaMode alphaMode = AlphaMode::Opaque;
	float alphaCutoff = 0.5f;
	bool doubleSided = false;
	bool specular_glossiness = false;

	glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	glm::vec2 normalScale = { 1.0f, 1.0f };
	glm::vec3 emissive = { 0.0f, 0.0f, 0.0f };
	int tex_idx_map = -1;
	int tex_idx_normalMap = -1;
	int tex_idx_emissiveMap = -1;

	// specular_glossiness = false
	float metallicFactor = 0.0f;
	float roughnessFactor = 1.0f;
	int tex_idx_metalnessMap = -1; // blue channel
	int tex_idx_roughnessMap = -1; // green channel 

	// specular_glossiness = true
	glm::vec3 specular = { 1.0f, 1.0f, 1.0f };
	float glossinessFactor = 0.0f;
	int tex_idx_specularMap = -1; // r g b channels
	int tex_idx_glossinessMap = -1; // alpha channel

	GLBuffer constant_material;
	void update_uniform();

	// 1: direct - diffuse 2: direct - specular 4: indirect - diffuse 8: indirect - specular 16: wire_only
	int tone_shading = 0;
	float wire_width = 1.5f;
	glm::vec3 wire_color = { 0.0f, 0.0f, 0.0f };


};