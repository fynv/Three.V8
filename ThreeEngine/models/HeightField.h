#pragma once

#include <memory>
#include <vector>
#include <glm.hpp>
#include "renderers/GLUtils.h"

class HeightField
{
public:
	HeightField(const glm::vec3& pos_min, const glm::vec3& pos_max, int width, int height);
	~HeightField();

	glm::vec3 m_pos_min;
	glm::vec3 m_pos_max;
	int m_width;
	int m_height;

	GLTexture2D m_tex_depth;
	unsigned m_fbo = -1;

	GLBuffer m_constant;

	std::vector<float> m_cpu_depth;
	void toCPU();

	// bilinear
	float GetHeight(float x, float z);

	void saveFile(const char* filename);

};
