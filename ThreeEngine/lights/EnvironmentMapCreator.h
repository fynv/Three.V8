#pragma once

#include <memory>
#include "renderers/GLUtils.h"
#include "utils/Image.h"
#include "lights/EnvironmentMap.h"

class EnvironmentMapCreator
{
public:
	EnvironmentMapCreator();
	void Create(const CubeImage* image, EnvironmentMap* envMap);


private:
	std::unique_ptr<GLShader> m_comp_downsample;
	std::unique_ptr<GLProgram> m_prog_downsample;
};