#include "GeometryCreator.h"
#include "ModelComponents.h"
#include <GL/glew.h>
#include <cmath>


const double PI = 3.14159265359;

void GeometryCreator::create(Primitive* primitive, const std::vector<glm::vec4>& pos, const std::vector<glm::vec4>& norm, const std::vector<glm::vec2>& uv, const std::vector<glm::ivec3>& faces)
{
	primitive->geometry.resize(1);
	GeometrySet& geo = primitive->geometry[0];
	primitive->num_pos = (int)pos.size();
	primitive->num_face = (int)faces.size();
	primitive->type_indices = 4;

	geo.pos_buf = (std::unique_ptr<GLBuffer>)(new GLBuffer(sizeof(glm::vec4) * primitive->num_pos, GL_ARRAY_BUFFER));
	geo.pos_buf->upload(pos.data());
	
	primitive->cpu_pos = std::unique_ptr<std::vector<glm::vec4>>(new std::vector<glm::vec4>(primitive->num_pos));
	memcpy(primitive->cpu_pos->data(), pos.data(), geo.pos_buf->m_size);

	geo.normal_buf = (std::unique_ptr<GLBuffer>)(new GLBuffer(sizeof(glm::vec4) * primitive->num_pos, GL_ARRAY_BUFFER));
	geo.normal_buf->upload(norm.data());

	primitive->uv_buf = (std::unique_ptr<GLBuffer>)(new GLBuffer(sizeof(glm::vec2) * primitive->num_pos, GL_ARRAY_BUFFER));
	primitive->uv_buf->upload(uv.data());

	primitive->index_buf = (std::unique_ptr<GLBuffer>)(new GLBuffer(sizeof(glm::ivec3) * primitive->num_face, GL_ELEMENT_ARRAY_BUFFER));
	primitive->index_buf->upload(faces.data());

	primitive->cpu_indices = std::unique_ptr<std::vector<uint8_t>>(new std::vector<uint8_t>(primitive->index_buf->m_size));
	memcpy(primitive->cpu_indices->data(), faces.data(), primitive->index_buf->m_size);

}

void GeometryCreator::CreateBox(Primitive* primitive, float width, float height, float depth)
{
	float half_w = width * 0.5f;
	float half_h = height * 0.5f;
	float half_d = depth * 0.5f;

	std::vector<glm::vec4> pos;
	std::vector<glm::vec4> norm;
	std::vector<glm::vec2> uv;
	std::vector<glm::ivec3> faces;

	// x positive
	{
		int v_start = (int)pos.size();
		pos.push_back({ half_w, half_h, half_d, 1.0f });
		pos.push_back({ half_w, half_h, -half_d, 1.0f });
		pos.push_back({ half_w, -half_h, half_d, 1.0f });
		pos.push_back({ half_w, -half_h, -half_d, 1.0f });

		norm.push_back({ 1.0f, 0.0f, 0.0f, 0.0f });
		norm.push_back({ 1.0f, 0.0f, 0.0f, 0.0f });
		norm.push_back({ 1.0f, 0.0f, 0.0f, 0.0f });
		norm.push_back({ 1.0f, 0.0f, 0.0f, 0.0f });

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
		pos.push_back({ -half_w, half_h, -half_d, 1.0f });
		pos.push_back({ -half_w, half_h, half_d, 1.0f });
		pos.push_back({ -half_w, -half_h, -half_d, 1.0f });
		pos.push_back({ -half_w, -half_h, half_d, 1.0f });

		norm.push_back({ -1.0f, 0.0f, 0.0f, 0.0f });
		norm.push_back({ -1.0f, 0.0f, 0.0f, 0.0f });
		norm.push_back({ -1.0f, 0.0f, 0.0f, 0.0f });
		norm.push_back({ -1.0f, 0.0f, 0.0f, 0.0f });

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
		pos.push_back({ -half_w, half_h, -half_d, 1.0f });
		pos.push_back({ half_w, half_h, -half_d, 1.0f });
		pos.push_back({ -half_w, half_h, half_d, 1.0f });
		pos.push_back({ half_w, half_h, half_d, 1.0f });

		norm.push_back({ 0.0f, 1.0f, 0.0f, 0.0f });
		norm.push_back({ 0.0f, 1.0f, 0.0f, 0.0f });
		norm.push_back({ 0.0f, 1.0f, 0.0f, 0.0f });
		norm.push_back({ 0.0f, 1.0f, 0.0f, 0.0f });

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
		pos.push_back({ -half_w, -half_h, half_d, 1.0f });
		pos.push_back({ half_w, -half_h, half_d, 1.0f });
		pos.push_back({ -half_w, -half_h, -half_d, 1.0f });
		pos.push_back({ half_w, -half_h, -half_d, 1.0f });

		norm.push_back({ 0.0f, -1.0f, 0.0f, 0.0f });
		norm.push_back({ 0.0f, -1.0f, 0.0f, 0.0f });
		norm.push_back({ 0.0f, -1.0f, 0.0f, 0.0f });
		norm.push_back({ 0.0f, -1.0f, 0.0f, 0.0f });

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
		pos.push_back({ -half_w, half_h, half_d, 1.0f });
		pos.push_back({ half_w, half_h, half_d, 1.0f });
		pos.push_back({ -half_w, -half_h, half_d, 1.0f });
		pos.push_back({ half_w, -half_h, half_d, 1.0f });

		norm.push_back({ 0.0f, 0.0f, 1.0f, 0.0f });
		norm.push_back({ 0.0f, 0.0f, 1.0f, 0.0f });
		norm.push_back({ 0.0f, 0.0f, 1.0f, 0.0f });
		norm.push_back({ 0.0f, 0.0f, 1.0f, 0.0f });

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
		pos.push_back({ half_w, half_h, -half_d, 1.0f });
		pos.push_back({ -half_w, half_h, -half_d, 1.0f });
		pos.push_back({ half_w, -half_h, -half_d, 1.0f });
		pos.push_back({ -half_w, -half_h, -half_d, 1.0f });

		norm.push_back({ 0.0f, 0.0f, -1.0f, 0.0f });
		norm.push_back({ 0.0f, 0.0f, -1.0f, 0.0f });
		norm.push_back({ 0.0f, 0.0f, -1.0f, 0.0f });
		norm.push_back({ 0.0f, 0.0f, -1.0f, 0.0f });

		uv.push_back({ 0.0f, 0.0f });
		uv.push_back({ 1.0f, 0.0f });
		uv.push_back({ 0.0f, 1.0f });
		uv.push_back({ 1.0f, 1.0f });

		faces.push_back({ v_start + 2, v_start + 1, v_start });
		faces.push_back({ v_start + 1, v_start + 2, v_start + 3 });
	}

	create(primitive, pos, norm, uv, faces);

	primitive->min_pos = { -half_w, -half_h, -half_d };
	primitive->max_pos = { half_w, half_h, half_d };
}

void GeometryCreator::CreateSphere(Primitive* primitive, float radius, int widthSegments, int heightSegments)
{
	int count_x = widthSegments + 1;
	int count_y = heightSegments + 1;
	int vert_count = count_x * count_y;
	int face_count = widthSegments * heightSegments * 2;

	std::vector<glm::vec4> pos(vert_count);
	std::vector<glm::vec4> norm(vert_count);
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
			pos[idx] = glm::vec4(dir* radius, 1.0f);
			norm[idx] = glm::vec4(dir, 0.0f);
			uv[idx] = { u,v };
		}
	}

	for (int j = 0; j < heightSegments; j++)
	{
		for (int i = 0; i < widthSegments; i++)
		{
			int a = i + j * count_x;
			int b = (i + 1) + j * count_x;
			int c = i + (j + 1) * count_x;
			int d = (i + 1) + (j + 1) * count_x;

			int idx = (i + j * widthSegments) * 2;
			faces[idx] = { c, b, a };
			faces[idx + 1] = { b, c, d };
		}
	}

	create(primitive, pos, norm, uv, faces);

	primitive->min_pos = { -radius, -radius, -radius };
	primitive->max_pos = { radius, radius, radius };
}

void GeometryCreator::CreatePlane(Primitive* primitive, float width, float height)
{
	float half_w = width * 0.5f;
	float half_h = height * 0.5f;

	std::vector<glm::vec4> pos;
	std::vector<glm::vec4> norm;
	std::vector<glm::vec2> uv;
	std::vector<glm::ivec3> faces;

	{
		int v_start = (int)pos.size();
		pos.push_back({ -half_w, half_h, 0.0f, 1.0f });
		pos.push_back({ half_w, half_h, 0.0f, 1.0f });
		pos.push_back({ -half_w, -half_h, 0.0f, 1.0f });
		pos.push_back({ half_w, -half_h, 0.0f, 1.0f });

		norm.push_back({ 0.0f, 0.0f, 1.0f, 0.0f });
		norm.push_back({ 0.0f, 0.0f, 1.0f, 0.0f });
		norm.push_back({ 0.0f, 0.0f, 1.0f, 0.0f });
		norm.push_back({ 0.0f, 0.0f, 1.0f, 0.0f });

		uv.push_back({ 0.0f, 0.0f });
		uv.push_back({ 1.0f, 0.0f });
		uv.push_back({ 0.0f, 1.0f });
		uv.push_back({ 1.0f, 1.0f });

		faces.push_back({ v_start + 2, v_start + 1, v_start });
		faces.push_back({ v_start + 1, v_start + 2, v_start + 3 });
	}
	create(primitive, pos, norm, uv, faces);

	primitive->min_pos = { -half_w, -half_h, 0.0f };
	primitive->max_pos = { half_w, half_h, 0.0f };
}
