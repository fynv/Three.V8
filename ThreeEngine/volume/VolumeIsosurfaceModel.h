#pragma once

#include <memory>
#include "core/Object3D.h"
#include "volume/VolumeData.h"
#include "volume/GridPartition.h"

class VolumeIsosurfaceModel : public Object3D
{
public:
	VolumeIsosurfaceModel(VolumeData* data);
	~VolumeIsosurfaceModel();

	struct Material
	{
		glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float metallicFactor = 0.0f;
		float roughnessFactor = 1.0f;
	};

	VolumeData* m_data;
	std::unique_ptr<GridPartition> m_partition;
	float m_isovalue = 0.4f;

	Material m_material;

	GLBuffer m_constant;
	void updateConstant();

	void set_color(const glm::vec3& color);
	void set_metalness(float metalness);
	void set_roughness(float roughness);
};