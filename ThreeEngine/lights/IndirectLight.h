#pragma once

#include <memory>
#include <glm.hpp>

class CubeRenderTarget;
class PerspectiveCamera;
class GLSpaceProbeTarget;
class EnvironmentMap;
class IndirectLight
{
public:
	IndirectLight();
	virtual ~IndirectLight();

	// tone_shading
	float diffuse_thresh = 0.2f;
	float diffuse_high = 0.8f;
	float diffuse_low = 0.2f;
	float specular_thresh = 0.2f;
	float specular_high = 0.8f;
	float specular_low = 0.2f;


	bool dynamic_map = false;
	glm::vec3 probe_position = glm::vec3(0.0f);
	std::unique_ptr<GLSpaceProbeTarget> probe_target;
	std::unique_ptr<PerspectiveCamera> probe_camera;
	std::unique_ptr<CubeRenderTarget> cube_target;
	std::unique_ptr<EnvironmentMap> env_map;

	void set_dynamic_map(bool on);
};

