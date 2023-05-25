#include <glm.hpp>
#include <emscripten.h>

const double PI = 3.14159265359;

extern "C"
{
	EMSCRIPTEN_KEEPALIVE void GeoDelete(void* ptr);
	EMSCRIPTEN_KEEPALIVE int GeoGetNumPos(void* ptr);
	EMSCRIPTEN_KEEPALIVE int GeoGetNumFace(void* ptr);
	EMSCRIPTEN_KEEPALIVE void* GeoGetPosition(void* ptr);
	EMSCRIPTEN_KEEPALIVE void* GeoGetNormal(void* ptr);
	EMSCRIPTEN_KEEPALIVE void* GeoGetUV(void* ptr);
	EMSCRIPTEN_KEEPALIVE void* GeoGetFaces(void* ptr);
	EMSCRIPTEN_KEEPALIVE void* GeoGetMinPos(void* ptr);
	EMSCRIPTEN_KEEPALIVE void* GeoGetMaxPos(void* ptr);
	
	EMSCRIPTEN_KEEPALIVE void* CreateBox(float width, float height, float depth);
	EMSCRIPTEN_KEEPALIVE void* CreateSphere(float radius, int widthSegments, int heightSegments);
	EMSCRIPTEN_KEEPALIVE void* CreatePlane(float width, float height);
}

#include <cstdio>
#include <vector>

struct Geometry
{
	std::vector<glm::vec4> pos;
	std::vector<glm::vec4> norm;
	std::vector<glm::vec2> uv;
	std::vector<glm::ivec3> faces;
	glm::vec3 min_pos;
	glm::vec3 max_pos;
};

void GeoDelete(void* ptr)
{
	Geometry* geo = (Geometry*)ptr;
	delete geo;
}

int GeoGetNumPos(void* ptr)
{
	Geometry* geo = (Geometry*)ptr;
	return (int)geo->pos.size();
}

int GeoGetNumFace(void* ptr)
{
	Geometry* geo = (Geometry*)ptr;
	return (int)geo->faces.size();
}

void* GeoGetPosition(void* ptr)
{
	Geometry* geo = (Geometry*)ptr;
	return geo->pos.data();
}

void* GeoGetNormal(void* ptr)
{
	Geometry* geo = (Geometry*)ptr;
	return geo->norm.data();
}

void* GeoGetUV(void* ptr)
{
	Geometry* geo = (Geometry*)ptr;
	return geo->uv.data();
}

void* GeoGetFaces(void* ptr)
{
	Geometry* geo = (Geometry*)ptr;
	return geo->faces.data();
}

void* GeoGetMinPos(void* ptr)
{
	Geometry* geo = (Geometry*)ptr;
	return &geo->min_pos;
}

void* GeoGetMaxPos(void* ptr)
{
	Geometry* geo = (Geometry*)ptr;
	return &geo->max_pos;
}

void* CreateBox(float width, float height, float depth)
{
	Geometry* geo = new Geometry;
	
	float half_w = width * 0.5f;
	float half_h = height * 0.5f;
	float half_d = depth * 0.5f;
	
	// x positive
	{
		int v_start = (int)geo->pos.size();
		geo->pos.push_back({ half_w, half_h, half_d, 1.0f });
		geo->pos.push_back({ half_w, half_h, -half_d, 1.0f });
		geo->pos.push_back({ half_w, -half_h, half_d, 1.0f });
		geo->pos.push_back({ half_w, -half_h, -half_d, 1.0f });

		geo->norm.push_back({ 1.0f, 0.0f, 0.0f, 0.0f });
		geo->norm.push_back({ 1.0f, 0.0f, 0.0f, 0.0f });
		geo->norm.push_back({ 1.0f, 0.0f, 0.0f, 0.0f });
		geo->norm.push_back({ 1.0f, 0.0f, 0.0f, 0.0f });

		geo->uv.push_back({ 0.0f, 0.0f });
		geo->uv.push_back({ 1.0f, 0.0f });
		geo->uv.push_back({ 0.0f, 1.0f });
		geo->uv.push_back({ 1.0f, 1.0f });

		geo->faces.push_back({ v_start + 2, v_start + 1, v_start });
		geo->faces.push_back({ v_start + 1, v_start + 2, v_start + 3 });
	}

	// x negative
	{
		int v_start = (int)geo->pos.size();
		geo->pos.push_back({ -half_w, half_h, -half_d, 1.0f });
		geo->pos.push_back({ -half_w, half_h, half_d, 1.0f });
		geo->pos.push_back({ -half_w, -half_h, -half_d, 1.0f });
		geo->pos.push_back({ -half_w, -half_h, half_d, 1.0f });

		geo->norm.push_back({ -1.0f, 0.0f, 0.0f, 0.0f });
		geo->norm.push_back({ -1.0f, 0.0f, 0.0f, 0.0f });
		geo->norm.push_back({ -1.0f, 0.0f, 0.0f, 0.0f });
		geo->norm.push_back({ -1.0f, 0.0f, 0.0f, 0.0f });

		geo->uv.push_back({ 0.0f, 0.0f });
		geo->uv.push_back({ 1.0f, 0.0f });
		geo->uv.push_back({ 0.0f, 1.0f });
		geo->uv.push_back({ 1.0f, 1.0f });

		geo->faces.push_back({ v_start + 2, v_start + 1, v_start });
		geo->faces.push_back({ v_start + 1, v_start + 2, v_start + 3 });
	}

	// y positive
	{
		int v_start = (int)geo->pos.size();
		geo->pos.push_back({ -half_w, half_h, -half_d, 1.0f });
		geo->pos.push_back({ half_w, half_h, -half_d, 1.0f });
		geo->pos.push_back({ -half_w, half_h, half_d, 1.0f });
		geo->pos.push_back({ half_w, half_h, half_d, 1.0f });

		geo->norm.push_back({ 0.0f, 1.0f, 0.0f, 0.0f });
		geo->norm.push_back({ 0.0f, 1.0f, 0.0f, 0.0f });
		geo->norm.push_back({ 0.0f, 1.0f, 0.0f, 0.0f });
		geo->norm.push_back({ 0.0f, 1.0f, 0.0f, 0.0f });

		geo->uv.push_back({ 0.0f, 0.0f });
		geo->uv.push_back({ 1.0f, 0.0f });
		geo->uv.push_back({ 0.0f, 1.0f });
		geo->uv.push_back({ 1.0f, 1.0f });

		geo->faces.push_back({ v_start + 2, v_start + 1, v_start });
		geo->faces.push_back({ v_start + 1, v_start + 2, v_start + 3 });
	}

	// y negative
	{
		int v_start = (int)geo->pos.size();
		geo->pos.push_back({ -half_w, -half_h, half_d, 1.0f });
		geo->pos.push_back({ half_w, -half_h, half_d, 1.0f });
		geo->pos.push_back({ -half_w, -half_h, -half_d, 1.0f });
		geo->pos.push_back({ half_w, -half_h, -half_d, 1.0f });

		geo->norm.push_back({ 0.0f, -1.0f, 0.0f, 0.0f });
		geo->norm.push_back({ 0.0f, -1.0f, 0.0f, 0.0f });
		geo->norm.push_back({ 0.0f, -1.0f, 0.0f, 0.0f });
		geo->norm.push_back({ 0.0f, -1.0f, 0.0f, 0.0f });

		geo->uv.push_back({ 0.0f, 0.0f });
		geo->uv.push_back({ 1.0f, 0.0f });
		geo->uv.push_back({ 0.0f, 1.0f });
		geo->uv.push_back({ 1.0f, 1.0f });

		geo->faces.push_back({ v_start + 2, v_start + 1, v_start });
		geo->faces.push_back({ v_start + 1, v_start + 2, v_start + 3 });
	}

	// z positive
	{
		int v_start = (int)geo->pos.size();
		geo->pos.push_back({ -half_w, half_h, half_d, 1.0f });
		geo->pos.push_back({ half_w, half_h, half_d, 1.0f });
		geo->pos.push_back({ -half_w, -half_h, half_d, 1.0f });
		geo->pos.push_back({ half_w, -half_h, half_d, 1.0f });

		geo->norm.push_back({ 0.0f, 0.0f, 1.0f, 0.0f });
		geo->norm.push_back({ 0.0f, 0.0f, 1.0f, 0.0f });
		geo->norm.push_back({ 0.0f, 0.0f, 1.0f, 0.0f });
		geo->norm.push_back({ 0.0f, 0.0f, 1.0f, 0.0f });

		geo->uv.push_back({ 0.0f, 0.0f });
		geo->uv.push_back({ 1.0f, 0.0f });
		geo->uv.push_back({ 0.0f, 1.0f });
		geo->uv.push_back({ 1.0f, 1.0f });

		geo->faces.push_back({ v_start + 2, v_start + 1, v_start });
		geo->faces.push_back({ v_start + 1, v_start + 2, v_start + 3 });
	}

	// z negative
	{
		int v_start = (int)geo->pos.size();
		geo->pos.push_back({ half_w, half_h, -half_d, 1.0f });
		geo->pos.push_back({ -half_w, half_h, -half_d, 1.0f });
		geo->pos.push_back({ half_w, -half_h, -half_d, 1.0f });
		geo->pos.push_back({ -half_w, -half_h, -half_d, 1.0f });

		geo->norm.push_back({ 0.0f, 0.0f, -1.0f, 0.0f });
		geo->norm.push_back({ 0.0f, 0.0f, -1.0f, 0.0f });
		geo->norm.push_back({ 0.0f, 0.0f, -1.0f, 0.0f });
		geo->norm.push_back({ 0.0f, 0.0f, -1.0f, 0.0f });

		geo->uv.push_back({ 0.0f, 0.0f });
		geo->uv.push_back({ 1.0f, 0.0f });
		geo->uv.push_back({ 0.0f, 1.0f });
		geo->uv.push_back({ 1.0f, 1.0f });

		geo->faces.push_back({ v_start + 2, v_start + 1, v_start });
		geo->faces.push_back({ v_start + 1, v_start + 2, v_start + 3 });
	}
	geo->min_pos = { -half_w, -half_h, -half_d };
	geo->max_pos = { half_w, half_h, half_d };
	
	return geo;
}

void* CreateSphere(float radius, int widthSegments, int heightSegments)
{
	Geometry* geo = new Geometry;

	int count_x = widthSegments + 1;
	int count_y = heightSegments + 1;
	int vert_count = count_x * count_y;
	int face_count = widthSegments * heightSegments * 2;
	
	geo->pos.resize(vert_count);
	geo->norm.resize(vert_count);
	geo->uv.resize(vert_count);
	geo->faces.resize(face_count);

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
			geo->pos[idx] = glm::vec4(dir * radius, 1.0f);
			geo->norm[idx] = glm::vec4(dir, 0.0f);
			geo->uv[idx] = { u,v };
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
			geo->faces[idx] = { c, b, a };
			geo->faces[idx + 1] = { b, c, d };
		}
	}

	geo->min_pos = { -radius, -radius, -radius };
	geo->max_pos = { radius, radius, radius };

	return geo;
}

void* CreatePlane(float width, float height)
{
	Geometry* geo = new Geometry;

	float half_w = width * 0.5f;
	float half_h = height * 0.5f;


	{
		int v_start = (int)geo->pos.size();
		geo->pos.push_back({ -half_w, half_h, 0.0f, 1.0f });
		geo->pos.push_back({ half_w, half_h, 0.0f, 1.0f });
		geo->pos.push_back({ -half_w, -half_h, 0.0f, 1.0f });
		geo->pos.push_back({ half_w, -half_h, 0.0f, 1.0f });

		geo->norm.push_back({ 0.0f, 0.0f, 1.0f, 0.0f });
		geo->norm.push_back({ 0.0f, 0.0f, 1.0f, 0.0f });
		geo->norm.push_back({ 0.0f, 0.0f, 1.0f, 0.0f });
		geo->norm.push_back({ 0.0f, 0.0f, 1.0f, 0.0f });

		geo->uv.push_back({ 0.0f, 0.0f });
		geo->uv.push_back({ 1.0f, 0.0f });
		geo->uv.push_back({ 0.0f, 1.0f });
		geo->uv.push_back({ 1.0f, 1.0f });

		geo->faces.push_back({ v_start + 2, v_start + 1, v_start });
		geo->faces.push_back({ v_start + 1, v_start + 2, v_start + 3 });
	}	

	geo->min_pos = { -half_w, -half_h, 0.0f };
	geo->max_pos = { half_w, half_h, 0.0f };

	return geo;

}
