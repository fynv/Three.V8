#include <GL/glew.h>
#include <gtx/quaternion.hpp>
#include <gtc/random.hpp>
#include "ProbeRayList.h"
#include "ProbeGrid.h"
#include "LODProbeGrid.h"

const double PI = 3.14159265359;

inline double rand01()
{
	return (double)rand() / ((double)RAND_MAX + 1.0);
}

inline double randRad()
{
	return rand01() * 2.0 * PI;
}


struct ListConst
{
	glm::mat4 rotation;
	int numProbes;
	int numDirections;
	float maxDistance;
	int padding;
};

inline glm::mat4 rand_rotation()
{
	glm::vec3 axis = glm::sphericalRand(1.0f);
	float angle = randRad();
	glm::quat quat = glm::angleAxis(angle, axis);
	return glm::toMat4(quat);
}

inline float get_max_distance(const ProbeGrid& probe_grid)
{
	glm::vec3 size_grid = probe_grid.coverage_max - probe_grid.coverage_min;
	glm::vec3 spacing = size_grid / glm::vec3(probe_grid.divisions);
	if (probe_grid.ypower > 1.0f)
	{
		float y0 = powf(((float)(probe_grid.divisions.y - 2) + 0.5f) / (float)probe_grid.divisions.y, probe_grid.ypower);
		float y1 = powf(((float)(probe_grid.divisions.y - 1) + 0.5f) / (float)probe_grid.divisions.y, probe_grid.ypower);
		spacing.y = (y1 - y0) * size_grid.y;
	}
	else if (probe_grid.ypower < 1.0f)
	{
		float y0 = powf(0.5f / (float)probe_grid.divisions.y, probe_grid.ypower);
		float y1 = powf(1.5f / (float)probe_grid.divisions.y, probe_grid.ypower);
		spacing.y = (y1 - y0) * size_grid.y;
	}
	return glm::length(spacing);
}

inline float get_max_distance(const LODProbeGrid& probe_grid)
{
	glm::vec3 size_grid = probe_grid.coverage_max - probe_grid.coverage_min;
	glm::vec3 spacing = size_grid / glm::vec3(probe_grid.base_divisions);
	return glm::length(spacing);
}

ProbeRayList::ProbeRayList(const ProbeGrid& probe_grid, int begin, int end, int num_directions)
	: m_constant(sizeof(ListConst), GL_UNIFORM_BUFFER)
	, rotation(rand_rotation())
	, num_probes(end - begin)
	, num_directions(num_directions)
	, max_distance(get_max_distance(probe_grid))
{
	updateConstant();
	positions.resize(num_probes);
	buf_positions = std::unique_ptr<GLBuffer>(new GLBuffer(sizeof(glm::vec4) * num_probes, GL_SHADER_STORAGE_BUFFER));
	glm::vec3 size_grid = probe_grid.coverage_max - probe_grid.coverage_min;
	for (int i = begin; i < end; i++)
	{
		int j = i - begin;
		int x = i;
		int y = x / probe_grid.divisions.x;
		int z = y / probe_grid.divisions.y;
		y = y % probe_grid.divisions.y;
		x = x % probe_grid.divisions.x;
		glm::ivec3 idx(x, y, z);
		glm::vec3 pos_normalized = (glm::vec3(idx) + 0.5f) / glm::vec3(probe_grid.divisions);
		pos_normalized.y = powf(pos_normalized.y, probe_grid.ypower);
		glm::vec3 pos = probe_grid.coverage_min + pos_normalized * size_grid;
		positions[j] = glm::vec4(pos, 1.0f);
	}
	buf_positions->upload(positions.data());
}


ProbeRayList::ProbeRayList(const LODProbeGrid& probe_grid, int begin, int end, int num_directions)
	: m_constant(sizeof(ListConst), GL_UNIFORM_BUFFER)
	, rotation(rand_rotation())
	, num_probes(end - begin)
	, num_directions(num_directions)
	, max_distance(get_max_distance(probe_grid))
{
	updateConstant();
	positions.resize(num_probes);
	buf_positions = std::unique_ptr<GLBuffer>(new GLBuffer(sizeof(glm::vec4) * num_probes, GL_SHADER_STORAGE_BUFFER));
	for (int i = begin; i < end; i++)
	{
		int j = i - begin;
		glm::vec4 pos_lod = probe_grid.m_probe_data[i * 10];
		positions[j] = glm::vec4(pos_lod.x, pos_lod.y, pos_lod.z, 1.0f);
	}
	buf_positions->upload(positions.data());
}

void ProbeRayList::updateConstant()
{
	ListConst c;
	c.rotation = rotation;
	c.numProbes = num_probes;
	c.numDirections = num_directions;
	c.maxDistance = max_distance;
	m_constant.upload(&c);
}

