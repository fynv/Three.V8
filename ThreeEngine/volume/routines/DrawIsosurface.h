#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class Camera;
class VolumeIsosurfaceModel;
class DrawIsosurface
{
public:
	DrawIsosurface();

	void render(const Camera* camera, const VolumeIsosurfaceModel* model);

private:
	std::unique_ptr<GLProgram> m_prog;
};

