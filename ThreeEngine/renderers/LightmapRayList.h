#pragma once

#include <vector>
#include <glm.hpp>

#include "models/ModelComponents.h"
#include "renderers/LightmapRenderTarget.h"
#include "renderers/BVHRenderTarget.h"
#include "renderers/GLUtils.h"

class LightmapRayList
{
public:
	LightmapRayList(LightmapRenderTarget* src, BVHRenderTarget* dst, int begin, int end, int num_rays = 64);

	// input
	LightmapRenderTarget* source;
	int begin;
	int end;	
	int num_rays;
	int jitter;
	
	// output
	int texels_per_row;
	int num_rows;

	GLDynBuffer m_constant;
	void updateConstant();

};

