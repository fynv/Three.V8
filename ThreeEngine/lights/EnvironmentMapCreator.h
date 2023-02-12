#pragma once

// Josiah Manson, Peter-Pike Sloan. Fast Filtering of Reflection Probes.
// https://research.activision.com/publications/archives/fast-filtering-of-reflection-probes

// Peter-Pike Sloan. Efficient Spherical Harmonic Evaluation.
// http://jcgt.org/published/0002/02/06/

#include <memory>
#include <glm.hpp>
#include <gtx/quaternion.hpp>
#include "renderers/GLUtils.h"
#include "utils/Image.h"
#include "lights/EnvironmentMap.h"
#include "backgrounds/Background.h"
#include "renderers/CubeRenderTarget.h"

class EnvironmentMapCreator
{
public:
	EnvironmentMapCreator();
	~EnvironmentMapCreator();

	static void CreateSH(glm::vec4 shCoefficients[9], unsigned tex_id, int tex_dim = 128, const glm::quat& rotation = glm::identity<glm::quat>());
	void CreateReflection(ReflectionMap& reflection, const GLCubemap* cubemap);

	void Create(const GLCubemap* cubemap, EnvironmentMap* envMap, bool irradiance_only);
	void Create(const CubeImage* image, EnvironmentMap* envMap, bool irradiance_only);
	void Create(const CubeBackground* background, EnvironmentMap* envMap, bool irradiance_only);
	void Create(const CubeRenderTarget* target, EnvironmentMap* envMap, bool irradiance_only);

private:	
	std::unique_ptr<GLProgram> m_prog_downsample;	
	std::unique_ptr<GLProgram> m_prog_filter;

	unsigned m_tex_src; // downsampled mipmaps	
	GLBuffer m_buf_coeffs;
};