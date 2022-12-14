#pragma once

#include "core/Object3D.h"
#include "volume/VolumeData.h"

class VolumeIsosurfaceModel : public Object3D
{
public:
	VolumeIsosurfaceModel(VolumeData* data);
	~VolumeIsosurfaceModel();

	VolumeData* m_data;
	float m_isovalue = 0.0f;

	GLDynBuffer m_constant;
	void updateConstant();
};