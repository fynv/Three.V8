#include <glm.hpp>
#include <emscripten.h>

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
