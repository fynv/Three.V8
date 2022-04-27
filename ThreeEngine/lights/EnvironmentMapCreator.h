#pragma once

// Josiah Manson, Peter-Pike Sloan. Fast Filtering of Reflection Probes.
// https://research.activision.com/publications/archives/fast-filtering-of-reflection-probes

// Peter-Pike Sloan. Efficient Spherical Harmonic Evaluation.
// http://jcgt.org/published/0002/02/06/

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
	void Create(const GLCubemap* cubemap, EnvironmentMap* envMap);
	void Create(const CubeImage* image, EnvironmentMap* envMap);
	void Create(const CubeBackground* background, EnvironmentMap* envMap);


private:
	std::unique_ptr<GLShader> m_comp_downsample;
	std::unique_ptr<GLProgram> m_prog_downsample;

	std::unique_ptr<GLShader> m_comp_filter;
	std::unique_ptr<GLProgram> m_prog_filter;

	unsigned m_tex_src; // downsampled mipmaps	
	GLBuffer m_buf_coeffs;
};