#pragma once

#include <glm.hpp>
#include <memory>
#include <string>

#include "renderers/GLUtils.h"
#include "renderers/GLRenderTarget.h"

class Primitive;
class SceneToVolume
{
public:
	SceneToVolume();
	~SceneToVolume();

	struct RenderParams
	{
		unsigned tex_id_volume;
		glm::vec3 coverage_min;
		glm::vec3 coverage_max;
		glm::ivec3 divisions;
		const GLBuffer* constant_model;
		const Primitive* primitive;
	};

	void render(const RenderParams& params);

private:
	std::unique_ptr<GLProgram> m_prog_x;
	std::unique_ptr<GLProgram> m_prog_y;
	std::unique_ptr<GLProgram> m_prog_z;

	GLRenderTarget target_x, target_y, target_z;

};


