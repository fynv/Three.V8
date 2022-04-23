#pragma once

#include <memory>
#include "renderers/GLUtils.h"
#include "utils/Image.h"
#include "lights/EnvironmentMap.h"
#include "backgrounds/Background.h"

class EnvironmentMapCreator
{
public:
	EnvironmentMapCreator();
	~EnvironmentMapCreator();
	void Create(int width, int height, const GLCubemap* cubemap, EnvironmentMap* envMap);
	void Create(const CubeImage* image, EnvironmentMap* envMap);
	void Create(const CubeBackground* background, EnvironmentMap* envMap);


private:
	std::unique_ptr<GLShader> m_comp_downsample;
	std::unique_ptr<GLProgram> m_prog_downsample;

	unsigned m_down_bufs[2]; // for initial down-sampling
	unsigned m_tex_128; 
};