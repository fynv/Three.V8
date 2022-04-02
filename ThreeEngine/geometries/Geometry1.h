#pragma once

#include <memory>
#include "renderers/GLUtils.h"

class Geometry1
{
public:
	int num_pos;
	int num_face;
	std::unique_ptr<GLBuffer> pos_buf;
	std::unique_ptr<GLBuffer> normal_buf;
	std::unique_ptr<GLBuffer> uv_buf;
	std::unique_ptr<GLBuffer> ind_buf;

	void CreateBox(float width, float height, float depth);
	void CreateSphere(float radius, int widthSegments, int heightSegments);

private:
	void create(const std::vector<glm::vec3>& pos, const std::vector<glm::vec3>& norm, const std::vector<glm::vec2>& uv, const std::vector<glm::ivec3>& faces);
};

