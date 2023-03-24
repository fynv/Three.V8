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
	int padding[2];
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


inline glm::vec3 sphericalFibonacci(float i, float n)
{
	const float PHI = sqrt(5.0f) * 0.5f + 0.5f;
	float m = i * (PHI - 1.0f);
	float frac_m = m - floor(m);
	float phi = 2.0f * PI * frac_m;
	float cosTheta = 1.0f - (2.0f * i + 1.0f) * (1.0f / n);
	float sinTheta = sqrtf(glm::clamp(1.0f - cosTheta * cosTheta, 0.0f, 1.0f));
	return glm::vec3(cosf(phi) * sinTheta, sinf(phi) * sinTheta, cosTheta);
}


inline void SHEval3(const float fX, const float fY, const float fZ, float* pSH)
{
	float fC0, fC1, fS0, fS1, fTmpA, fTmpB, fTmpC;
	float fZ2 = fZ * fZ;

	pSH[0] = 0.2820947917738781f;
	pSH[2] = 0.4886025119029199f * fZ;
	pSH[6] = 0.9461746957575601f * fZ2 + -0.3153915652525201f;
	fC0 = fX;
	fS0 = fY;

	fTmpA = 0.48860251190292f;
	pSH[3] = fTmpA * fC0;
	pSH[1] = fTmpA * fS0;
	fTmpB = 1.092548430592079f * fZ;
	pSH[7] = fTmpB * fC0;
	pSH[5] = fTmpB * fS0;
	fC1 = fX * fC0 - fY * fS0;
	fS1 = fX * fS0 + fY * fC0;

	fTmpC = 0.5462742152960395f;
	pSH[8] = fTmpC * fC1;
	pSH[4] = fTmpC * fS1;
}

void ProbeRayList::_calc_shirr_weights()
{
	float area = (4.0f * PI) / float(num_directions);

	float order[9] = { 0.0f, 1.0f, 1.0f, 1.0f, 2.0f, 2.0f, 2.0f,2.0f, 2.0f };
	float filtering[9];
	for (int k = 0; k < 9; k++)
	{
		float x = order[k] / 3.0f;
		filtering[k] = expf(-x * x);
	}

	std::vector<float> weights(num_directions * 9);
	for (int ray_id = 0; ray_id < num_directions; ray_id++)
	{
		glm::vec3 sf = sphericalFibonacci(ray_id, num_directions);
		glm::vec3 dir = glm::vec3(rotation * glm::vec4(sf, 0.0f));

		float shBasis[9];
		SHEval3(dir.x, dir.y, dir.z, shBasis);
		for (int k = 0; k < 9; k++)
		{
			weights[k * num_directions + ray_id] = shBasis[k] * filtering[k] * area;
		}
	}

	for (int i = 0; i < 9; i++)
	{
		TexSHIrrWeight[i] = std::unique_ptr<GLBuffer>(new GLBuffer(sizeof(float) * num_directions, GL_SHADER_STORAGE_BUFFER));
		TexSHIrrWeight[i]->upload(weights.data() + i * num_directions);

	}
}

ProbeRayList::ProbeRayList(const glm::vec3& coverage_min, const glm::vec3& coverage_max, const glm::ivec3& divisions, int begin, int end, int num_directions)
	: m_constant(sizeof(ListConst), GL_UNIFORM_BUFFER)
	, rotation(rand_rotation())
	, num_probes(end - begin)
	, num_directions(num_directions)
{
	glm::vec3 size_grid = coverage_max - coverage_min;

	updateConstant();
	positions.resize(num_probes);
	buf_positions = std::unique_ptr<GLBuffer>(new GLBuffer(sizeof(glm::vec4) * num_probes, GL_SHADER_STORAGE_BUFFER));

	for (int i = begin; i < end; i++)
	{
		int j = i - begin;
		int x = i;
		int y = x / divisions.x;
		int z = y / divisions.y;
		y = y % divisions.y;
		x = x % divisions.x;
		glm::ivec3 idx(x, y, z);
		glm::vec3 pos_normalized = (glm::vec3(idx) + 0.5f) / glm::vec3(divisions);		
		glm::vec3 pos = coverage_min + pos_normalized * size_grid;
		positions[j] = glm::vec4(pos, 1.0f);
	}
	buf_positions->upload(positions.data());
	_calc_shirr_weights();
}

ProbeRayList::ProbeRayList(const ProbeGrid& probe_grid, int begin, int end, int num_directions)
	: m_constant(sizeof(ListConst), GL_UNIFORM_BUFFER)
	, rotation(rand_rotation())
	, num_probes(end - begin)
	, num_directions(num_directions)
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
	_calc_shirr_weights();
}


ProbeRayList::ProbeRayList(const LODProbeGrid& probe_grid, int begin, int end, int num_directions)
	: m_constant(sizeof(ListConst), GL_UNIFORM_BUFFER)
	, rotation(rand_rotation())
	, num_probes(end - begin)
	, num_directions(num_directions)
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
	_calc_shirr_weights();
}

void ProbeRayList::updateConstant()
{
	ListConst c;
	c.rotation = rotation;
	c.numProbes = num_probes;
	c.numDirections = num_directions;
	m_constant.upload(&c);
}

