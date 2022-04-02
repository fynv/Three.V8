#include <glm.hpp>
#include <GL/glew.h>
#include <vector>
#include <cmath>
#include "Geometry1.h"

const double PI = 3.14159265359;

void Geometry1::create(const std::vector<glm::vec3>& pos, const std::vector<glm::vec3>& norm, const std::vector<glm::vec2>& uv, const std::vector<glm::ivec3>& faces)
{
	num_pos = (int)pos.size();
	num_face = (int)faces.size();
	pos_buf = (std::unique_ptr<GLBuffer>)(new GLBuffer(sizeof(glm::vec3)*num_pos, GL_ARRAY_BUFFER));
	pos_buf->upload(pos.data());
	normal_buf = (std::unique_ptr<GLBuffer>)(new GLBuffer(sizeof(glm::vec3)*num_pos, GL_ARRAY_BUFFER));
	normal_buf->upload(norm.data());
	uv_buf = (std::unique_ptr<GLBuffer>)(new GLBuffer(sizeof(glm::vec2)*num_pos, GL_ARRAY_BUFFER));
	uv_buf->upload(uv.data());
	ind_buf = (std::unique_ptr<GLBuffer>)(new GLBuffer(sizeof(glm::ivec3)*num_face, GL_ELEMENT_ARRAY_BUFFER));
	ind_buf->upload(faces.data());
}

void Geometry1::CreateBox(float width, float height, float depth)
{
	float half_w = width * 0.5f;
	float half_h = height * 0.5f;
	float half_d = depth * 0.5f;

	std::vector<glm::vec3> pos;
	std::vector<glm::vec3> norm;
	std::vector<glm::vec2> uv;
	std::vector<glm::ivec3> faces;

	// x positive
	{
		int v_start = (int)pos.size();
		pos.push_back({ half_w, half_h, half_d });
		pos.push_back({ half_w, half_h, -half_d });
		pos.push_back({ half_w, -half_h, half_d });
		pos.push_back({ half_w, -half_h, -half_d });

		norm.push_back({ 1.0f, 0.0f, 0.0f });
		norm.push_back({ 1.0f, 0.0f, 0.0f });
		norm.push_back({ 1.0f, 0.0f, 0.0f });
		norm.push_back({ 1.0f, 0.0f, 0.0f });

		uv.push_back({ 0.0f, 0.0f });
		uv.push_back({ 1.0f, 0.0f });
		uv.push_back({ 0.0f, 1.0f });
		uv.push_back({ 1.0f, 1.0f });

		faces.push_back({ v_start + 2, v_start + 1, v_start });
		faces.push_back({ v_start + 1, v_start + 2, v_start + 3 });
	}

	// x negative
	{
		int v_start = (int)pos.size();
		pos.push_back({ -half_w, half_h, -half_d });
		pos.push_back({ -half_w, half_h, half_d });
		pos.push_back({ -half_w, -half_h, -half_d });
		pos.push_back({ -half_w, -half_h, half_d });

		norm.push_back({ -1.0f, 0.0f, 0.0f });
		norm.push_back({ -1.0f, 0.0f, 0.0f });
		norm.push_back({ -1.0f, 0.0f, 0.0f });
		norm.push_back({ -1.0f, 0.0f, 0.0f });

		uv.push_back({ 0.0f, 0.0f });
		uv.push_back({ 1.0f, 0.0f });
		uv.push_back({ 0.0f, 1.0f });
		uv.push_back({ 1.0f, 1.0f });

		faces.push_back({ v_start + 2, v_start + 1, v_start });
		faces.push_back({ v_start + 1, v_start + 2, v_start + 3 });
	}

	// y positive
	{
		int v_start = (int)pos.size();
		pos.push_back({ -half_w, half_h, -half_d });
		pos.push_back({ half_w, half_h, -half_d });
		pos.push_back({ -half_w, half_h, half_d });
		pos.push_back({ half_w, half_h, half_d });

		norm.push_back({ 0.0f, 1.0f, 0.0f });
		norm.push_back({ 0.0f, 1.0f, 0.0f });
		norm.push_back({ 0.0f, 1.0f, 0.0f });
		norm.push_back({ 0.0f, 1.0f, 0.0f });

		uv.push_back({ 0.0f, 0.0f });
		uv.push_back({ 1.0f, 0.0f });
		uv.push_back({ 0.0f, 1.0f });
		uv.push_back({ 1.0f, 1.0f });

		faces.push_back({ v_start + 2, v_start + 1, v_start });
		faces.push_back({ v_start + 1, v_start + 2, v_start + 3 });
	}

	// y negative
	{
		int v_start = (int)pos.size();
		pos.push_back({ -half_w, -half_h, half_d });
		pos.push_back({ half_w, -half_h, half_d });
		pos.push_back({ -half_w, -half_h, -half_d });
		pos.push_back({ half_w, -half_h, -half_d });

		norm.push_back({ 0.0f, -1.0f, 0.0f });
		norm.push_back({ 0.0f, -1.0f, 0.0f });
		norm.push_back({ 0.0f, -1.0f, 0.0f });
		norm.push_back({ 0.0f, -1, 0.0f });

		uv.push_back({ 0.0f, 0.0f });
		uv.push_back({ 1.0f, 0.0f });
		uv.push_back({ 0.0f, 1.0f });
		uv.push_back({ 1.0f, 1.0f });

		faces.push_back({ v_start + 2, v_start + 1, v_start });
		faces.push_back({ v_start + 1, v_start + 2, v_start + 3 });
	}

	// z positive
	{
		int v_start = (int)pos.size();
		pos.push_back({ -half_w, half_h, half_d });
		pos.push_back({ half_w, half_h, half_d });
		pos.push_back({ -half_w, -half_h, half_d });
		pos.push_back({ half_w, -half_h, half_d });

		norm.push_back({ 0.0f, 0.0f, 1.0f });
		norm.push_back({ 0.0f, 0.0f, 1.0f });
		norm.push_back({ 0.0f, 0.0f, 1.0f });
		norm.push_back({ 0.0f, 0.0f, 1.0f });

		uv.push_back({ 0.0f, 0.0f });
		uv.push_back({ 1.0f, 0.0f });
		uv.push_back({ 0.0f, 1.0f });
		uv.push_back({ 1.0f, 1.0f });

		faces.push_back({ v_start + 2, v_start + 1, v_start });
		faces.push_back({ v_start + 1, v_start + 2, v_start + 3 });
	}

	// z negative
	{
		int v_start = (int)pos.size();
		pos.push_back({ half_w, half_h, -half_d });
		pos.push_back({ -half_w, half_h, -half_d });
		pos.push_back({ half_w, -half_h, -half_d });
		pos.push_back({ -half_w, -half_h, -half_d });

		norm.push_back({ 0.0f, 0.0f, -1.0f });
		norm.push_back({ 0.0f, 0.0f, -1.0f });
		norm.push_back({ 0.0f, 0.0f, -1.0f });
		norm.push_back({ 0.0f, 0.0f, -1.0f });

		uv.push_back({ 0.0f, 0.0f });
		uv.push_back({ 1.0f, 0.0f });
		uv.push_back({ 0.0f, 1.0f });
		uv.push_back({ 1.0f, 1.0f });

		faces.push_back({ v_start + 2, v_start + 1, v_start });
		faces.push_back({ v_start + 1, v_start + 2, v_start + 3 });
	}

	create(pos, norm, uv, faces);
}

void Geometry1::CreateSphere(float radius, int widthSegments, int heightSegments)
{
	int count_x = widthSegments + 1;
	int count_y = heightSegments + 1;
	int vert_count = count_x * count_y;
	int face_count = widthSegments * heightSegments * 2;

	std::vector<glm::vec3> pos(vert_count);
	std::vector<glm::vec3> norm(vert_count);
	std::vector<glm::vec2> uv(vert_count);
	std::vector<glm::ivec3> faces(face_count);
	
	for (int j = 0; j < count_y; j++)
	{
		float v = (float)j / (float)heightSegments;
		float phi = (0.5f - v) * (float)PI;
		float cos_phi = cosf(phi);
		float sin_phi = sinf(phi);	
		for (int i = 0; i < count_x; i++)
		{
			float u = (float)i / (float)widthSegments;
			float theta = u * 2.0f * (float)PI;
			float cos_theta = cosf(theta);
			float sin_theta = sinf(theta);
			glm::vec3 dir = { cos_phi * sin_theta, sin_phi, cos_phi * cos_theta };

			int idx = i + j * count_x;
			pos[idx] = dir * radius;
			norm[idx] = dir;
			uv[idx] = { u,v };
		}
	}

	for (int j = 0; j < heightSegments; j++)
	{
		for (int i = 0; i < widthSegments; i++)
		{
			int a = i + j * count_x;
			int b = (i+1) + j * count_x;
			int c = i + (j+1) * count_x;
			int d = (i+1) + (j + 1) * count_x;

			int idx = (i + j * widthSegments) * 2;
			faces[idx] = { c, b, a };
			faces[idx + 1] = { b, c, d };
		}
	}

	create(pos, norm, uv, faces);
}