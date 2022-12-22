#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class DirectionalLightShadow;
class VolumeIsosurfaceModel;
class IsosurfaceDirectionalShadow
{
public:
	IsosurfaceDirectionalShadow();

	struct RenderParams
	{
		const VolumeIsosurfaceModel* model;
		const DirectionalLightShadow* shadow;
	};

	void render(const RenderParams& params);


private:
	std::unique_ptr<GLProgram> m_prog;

};

