#pragma once

class Material
{
public:
	Material() {}
	virtual ~Material() {}
};

enum class MaterialType
{
	None,
	MeshStandardMaterial
};
