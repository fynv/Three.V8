#include <GL/glew.h>
#include <queue>
#include <gtc/random.hpp>
#include <half.hpp>
#include "LODProbeGrid.h"
#include "renderers/GLRenderer.h"

#include "scenes/Scene.h"
#include "models/SimpleModel.h"
#include "models/GLTFModel.h"
#include "core/BoundingVolumeHierarchy.h"

#include "EnvironmentMapCreator.h"

#include "renderers/BVHRenderer.h"
#include "renderers/BVHRenderTarget.h"
#include "ProbeRayList.h"

#define RADIANCE_DIFF 0

const double PI = 3.14159265359;

inline double rand01()
{
	return (double)rand() / ((double)RAND_MAX + 1.0);
}

inline double randRad()
{
	return rand01() * 2.0 * PI;
}

inline glm::mat4 rand_rotation()
{
	glm::vec3 axis = glm::sphericalRand(1.0f);
	float angle = randRad();
	glm::quat quat = glm::angleAxis(angle, axis);
	return glm::toMat4(quat);
}


struct LODProbeGridConst
{
	glm::vec4 coverageMin;
	glm::vec4 coverageMax;
	glm::ivec4 baseDivisions;	
	int subDivisionLevel;
	float normalBias;
	int numProbes;
	int visRes;
	int packSize;
	int packRes;
	int irrRes;
	int irrPackRes;
	float diffuseThresh;
	float diffuseHigh;
	float diffuseLow;
	float specularThresh;
	float specularHigh;
	float specularLow;
};


LODProbeGrid::LODProbeGrid() : m_constant(sizeof(LODProbeGridConst), GL_UNIFORM_BUFFER)
{		
	
}

LODProbeGrid::~LODProbeGrid()
{

}

void LODProbeGrid::_presample_irradiance()
{
	int num_probes = getNumberOfProbes();
	int pack_size = int(ceilf(sqrtf(float(num_probes))));
	irr_pack_res = pack_size * (irr_res + 2);

	std::vector<glm::vec3> irradiance_img(irr_pack_res * irr_pack_res);

	for (int index = 0; index < num_probes; index++)
	{
		std::vector<glm::vec3> tex_data(irr_res * irr_res);
		EnvironmentMapCreator::PresampleSH(m_probe_data.data() + index * 10 + 1, tex_data.data(), irr_res);
		for (int y = 0; y < irr_res; y++)
		{
			for (int x = 0; x < irr_res; x++)
			{
				glm::vec3 irr = tex_data[x + y* irr_res];
				int out_x = (index % pack_size) * (irr_res + 2) + x + 1;
				int out_y = (index / pack_size) * (irr_res + 2) + y + 1;
				irradiance_img[out_x + out_y * irr_pack_res] = irr;
			}
		}

		{
			glm::vec3 irr = tex_data[(irr_res - 1) + (irr_res - 1) * irr_res];			
			int out_x = (index % pack_size) * (irr_res + 2);
			int out_y = (index / pack_size) * (irr_res + 2);
			irradiance_img[out_x + out_y * irr_pack_res] = irr;
		}
		{
			glm::vec3 irr = tex_data[(irr_res - 1) * irr_res];
			int out_x = (index % pack_size) * (irr_res + 2) + irr_res + 1;
			int out_y = (index / pack_size) * (irr_res + 2);
			irradiance_img[out_x + out_y * irr_pack_res] = irr;
		}
		
		{
			glm::vec3 irr = tex_data[irr_res - 1];			
			int out_x = (index % pack_size) * (irr_res + 2);
			int out_y = (index / pack_size) * (irr_res + 2) + irr_res + 1;
			irradiance_img[out_x + out_y * irr_pack_res] = irr;
		}

		{
			glm::vec3 irr = tex_data[0];			
			int out_x = (index % pack_size) * (irr_res + 2) + irr_res + 1;
			int out_y = (index / pack_size) * (irr_res + 2) + irr_res + 1;
			irradiance_img[out_x + out_y * irr_pack_res] = irr;
		}

		for (int x = 0; x < irr_res; x++)
		{
			{
				glm::vec3 irr = tex_data[irr_res - 1 - x];
				int out_x = (index % pack_size) * (irr_res + 2) + x + 1;
				int out_y = (index / pack_size) * (irr_res + 2);
				irradiance_img[out_x + out_y * irr_pack_res] = irr;				
			}
			{
				glm::vec3 irr = tex_data[(irr_res - 1 - x) + (irr_res - 1) * irr_res];			
				int out_x = (index % pack_size) * (irr_res + 2) + x + 1;
				int out_y = (index / pack_size) * (irr_res + 2) + irr_res + 1;
				irradiance_img[out_x + out_y * irr_pack_res] = irr;
			}
		}
		for (int y = 0; y < irr_res; y++)
		{
			{
				glm::vec3 irr = tex_data[(irr_res - 1 - y) * irr_res];
				int out_x = (index % pack_size) * (irr_res + 2);
				int out_y = (index / pack_size) * (irr_res + 2) + y + 1;
				irradiance_img[out_x + out_y * irr_pack_res] = irr;
			}

			{
				glm::vec3 irr = tex_data[(irr_res - 1) + (irr_res - 1 - y) * irr_res];
				int out_x = (index % pack_size) * (irr_res + 2) + irr_res + 1;
				int out_y = (index / pack_size) * (irr_res + 2) + y + 1;
				irradiance_img[out_x + out_y * irr_pack_res] = irr;				
			}
		}
	}

	m_tex_irradiance = std::unique_ptr<GLTexture2D>(new GLTexture2D);
	glBindTexture(GL_TEXTURE_2D, m_tex_irradiance->tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);	
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R11F_G11F_B10F, irr_pack_res, irr_pack_res);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0,0, irr_pack_res, irr_pack_res, GL_RGB, GL_FLOAT, irradiance_img.data());	
	glBindTexture(GL_TEXTURE_2D, 0);

	/*std::vector<glm::u8vec3> dump(irr_pack_res * irr_pack_res);
	for (int i = 0; i < irr_pack_res * irr_pack_res; i++)
	{
		dump[i] = glm::u8vec3(glm::clamp(irradiance_img[i], 0.0f, 1.0f) * 255.0f);
	}

	FILE* fp = fopen("dmp_irr.raw", "wb");
	fwrite(dump.data(), 3, dump.size(), fp);
	fclose(fp);*/

}

#if 0
void LODProbeGrid::_create_index_tex()
{
	glm::ivec3 vol_div = base_divisions * (1 << sub_division_level);
	std::vector<glm::u8vec4> indices(vol_div.x * vol_div.y * vol_div.z);

	struct Node
	{
		int idx;
		int level;
		glm::ivec3 ipos;
	};

	std::queue<Node> queue;
	
	for (int z = 0; z < base_divisions.z; z++)
	{
		for (int y = 0; y < base_divisions.y; y++)
		{
			for (int x = 0; x < base_divisions.x; x++)
			{
				int i = x + (y + z * base_divisions.y) * base_divisions.x;
				Node node;
				node.idx = m_sub_index[i];
				node.level = 0;
				node.ipos = { x,y,z };
				queue.push(node);
			}
		}
	}

	int num_probes = getNumberOfProbes();
	int base_offset = base_divisions.x * base_divisions.y * base_divisions.z;

	while (queue.size() > 0)
	{
		Node node = queue.front();
		queue.pop();

		int idx = node.idx;
		glm::ivec3 ipos = node.ipos;
		int level = node.level;
		if (idx < num_probes)
		{
			int range = 1 << (sub_division_level - level);
			glm::ivec3 offset = ipos * range;
			for (int z = 0; z < range; z++)
			{
				for (int y = 0; y < range; y++)
				{
					for (int x = 0; x < range; x++)
					{
						glm::ivec3 ipos_high = offset + glm::ivec3(x, y, z);
						int i = ipos_high.x + (ipos_high.y + ipos_high.z * vol_div.y) * vol_div.x;
						indices[i].x = uint8_t(idx & 0xFF);
						indices[i].y = uint8_t((idx >> 8) & 0xFF);
						indices[i].z = uint8_t(idx >> 16);
						indices[i].w = uint8_t(level);
					}
				}
			}
		}
		else
		{
			idx -= num_probes;
			int offset = base_offset + idx * 8;
			for (int z = 0; z < 2; z++)
			{
				for (int y = 0; y < 2; y++)
				{
					for (int x = 0; x < 2; x++)
					{
						int i = x + y * 2 + z * 4;
						glm::ivec3 delta = { x,y,z };
						glm::ivec3 ipos_sub = ipos * 2 + delta;
						int level_sub = level + 1;
						glm::ivec3 sub_div = base_divisions * (1 << level_sub);
						Node sub;
						sub.idx = m_sub_index[offset + i];
						sub.level = level_sub;
						sub.ipos = ipos_sub;
						queue.push(sub);
					}
				}
			}
		}
	}

	m_tex_index = std::unique_ptr<GLTexture3D>(new GLTexture3D);

	glBindTexture(GL_TEXTURE_3D, m_tex_index->tex_id);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);		
	glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8UI, vol_div.x, vol_div.y, vol_div.z);
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, vol_div.x, vol_div.y, vol_div.z, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, indices.data());	
	glBindTexture(GL_TEXTURE_3D, 0);

}
#endif

void LODProbeGrid::updateBuffers()
{
	m_sub_index_buf = std::unique_ptr<GLBuffer>(new GLBuffer(sizeof(int) * m_sub_index.size(), GL_SHADER_STORAGE_BUFFER));
	m_sub_index_buf->upload(m_sub_index.data());

	int num_probes = getNumberOfProbes();
	for (int i = 0; i < 10; i++)
	{
		std::vector<glm::vec4> data(num_probes);
		for (size_t j = 0; j < num_probes; j++)
		{
			data[j] = m_probe_data[j * 10 + i];
		}
		m_probe_bufs[i] = std::unique_ptr<GLBuffer>(new GLBuffer(num_probes * sizeof(glm::vec4), GL_SHADER_STORAGE_BUFFER));
		m_probe_bufs[i]->upload(data.data());
	}

	_presample_irradiance();	
	
	pack_size = int(ceilf(sqrtf(float(num_probes))));
	pack_res = pack_size * (vis_res + 2);
	m_visibility_data.resize(pack_res * pack_res * 2, 0);

	m_tex_visibility = std::unique_ptr<GLTexture2D>(new GLTexture2D);
	glBindTexture(GL_TEXTURE_2D, m_tex_visibility->tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG16F, pack_res, pack_res);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0,0, pack_res, pack_res, GL_RG, GL_FLOAT, m_visibility_data.data());
	glBindTexture(GL_TEXTURE_2D, 0);
}


void LODProbeGrid::presample_probe(int index)
{
	std::vector<glm::vec3> irradiance_img((irr_res + 2) * (irr_res + 2));

	std::vector<glm::vec3> tex_data(irr_res * irr_res);
	EnvironmentMapCreator::PresampleSH(m_probe_data.data() + index * 10 + 1, tex_data.data(), irr_res);
	for (int y = 0; y < irr_res; y++)
	{
		for (int x = 0; x < irr_res; x++)
		{
			glm::vec3 irr = tex_data[x + y * irr_res];
			int out_x = x + 1;
			int out_y = y + 1;
			irradiance_img[out_x + out_y * (irr_res + 2)] = irr;
		}
	}

	{
		glm::vec3 irr = tex_data[(irr_res - 1) + (irr_res - 1) * irr_res];
		int out_x = 0;
		int out_y = 0;
		irradiance_img[out_x + out_y * (irr_res + 2)] = irr;
	}
	{
		glm::vec3 irr = tex_data[(irr_res - 1) * irr_res];
		int out_x = irr_res + 1;
		int out_y = 0;
		irradiance_img[out_x + out_y * (irr_res + 2)] = irr;
	}

	{
		glm::vec3 irr = tex_data[irr_res - 1];
		int out_x = 0;
		int out_y = irr_res + 1;
		irradiance_img[out_x + out_y * (irr_res + 2)] = irr;
	}

	{
		glm::vec3 irr = tex_data[0];
		int out_x = irr_res + 1;
		int out_y = irr_res + 1;
		irradiance_img[out_x + out_y * (irr_res + 2)] = irr;
	}

	for (int x = 0; x < irr_res; x++)
	{
		{
			glm::vec3 irr = tex_data[irr_res - 1 - x];
			int out_x = x + 1;
			int out_y = 0;
			irradiance_img[out_x + out_y * (irr_res + 2)] = irr;
		}
		{
			glm::vec3 irr = tex_data[(irr_res - 1 - x) + (irr_res - 1) * irr_res];
			int out_x = x + 1;
			int out_y = irr_res + 1;
			irradiance_img[out_x + out_y * (irr_res + 2)] = irr;
		}
	}
	for (int y = 0; y < irr_res; y++)
	{
		{
			glm::vec3 irr = tex_data[(irr_res - 1 - y) * irr_res];
			int out_x = 0;
			int out_y = y + 1;
			irradiance_img[out_x + out_y * (irr_res + 2)] = irr;
		}

		{
			glm::vec3 irr = tex_data[(irr_res - 1) + (irr_res - 1 - y) * irr_res];
			int out_x = irr_res + 1;
			int out_y = y + 1;
			irradiance_img[out_x + out_y * (irr_res + 2)] = irr;
		}
	}

	int offset_x = (index % pack_size) * (irr_res + 2);
	int offset_y = (index / pack_size) * (irr_res + 2);

	glBindTexture(GL_TEXTURE_2D, m_tex_irradiance->tex_id);
	glTexSubImage2D(GL_TEXTURE_2D, 0, offset_x, offset_y, irr_res + 2, irr_res + 2, GL_RGB, GL_FLOAT, irradiance_img.data());
	glBindTexture(GL_TEXTURE_2D, 0);
}

int LODProbeGrid::getNumberOfProbes() const
{
	return m_probe_data.size() / 10;
}

void LODProbeGrid::updateConstant()
{
	LODProbeGridConst c;
	c.coverageMin = glm::vec4(coverage_min, 0.0f);
	c.coverageMax = glm::vec4(coverage_max, 0.0f);
	c.baseDivisions = glm::ivec4(base_divisions, 0);
	c.subDivisionLevel = sub_division_level;
	c.normalBias = normal_bias;
	c.numProbes = getNumberOfProbes();
	c.visRes = vis_res;
	c.packSize = pack_size;
	c.packRes = pack_res;
	c.irrRes = irr_res;
	c.irrPackRes = irr_pack_res;
	c.diffuseThresh = diffuse_thresh;
	c.diffuseHigh = diffuse_high;
	c.diffuseLow = diffuse_low;
	c.specularThresh = specular_thresh;
	c.specularHigh = specular_high;
	c.specularLow = specular_low;
	m_constant.upload(&c);
}

void LODProbeGrid::initialize(GLRenderer& renderer, Scene& scene)
{
	m_sub_index.clear();
	m_probe_data.clear();
	m_visibility_data.clear();

	typedef std::vector<uint8_t> Volume;
	std::vector<Volume> volumes(sub_division_level + 1);

	if (sub_division_level > 0)
	{		
		unsigned tex_id;
		glGenTextures(1, &tex_id);

		glm::ivec3 vol_div = base_divisions * (1 << sub_division_level);

		{
			Volume& vol = volumes[sub_division_level];

			glBindTexture(GL_TEXTURE_3D, tex_id);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexStorage3D(GL_TEXTURE_3D, 1, GL_R8UI, vol_div.x, vol_div.y, vol_div.z);
			glBindTexture(GL_TEXTURE_3D, 0);

			uint8_t zero = 0;
			glClearTexImage(tex_id, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, &zero);
			renderer.sceneToVolume(scene, tex_id, coverage_min, coverage_max, vol_div);
			vol.resize(vol_div.x * vol_div.y * vol_div.z);

			glBindTexture(GL_TEXTURE_3D, tex_id);
			glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, vol.data());
			glBindTexture(GL_TEXTURE_3D, 0);

			glDeleteTextures(1, &tex_id);
		}

		{
			for (int level = 0; level < sub_division_level; level++)
			{
				glm::ivec3 l_div = vol_div / (1 << (sub_division_level - level));
				volumes[level].resize(l_div.x * l_div.y * l_div.z);

			}
			Volume& vol = volumes[sub_division_level];
			for (int z = 0; z < vol_div.z; z++)
			{
				for (int y = 0; y < vol_div.y; y++)
				{
					for (int x = 0; x < vol_div.x; x++)
					{
						int idx = x + (y + z * vol_div.y) * vol_div.x;
						uint8_t dir_mask = vol[idx];
						if (dir_mask & 1)
						{
							unsigned level_mask = (1 << sub_division_level) - 1;
							for (int level = 0; level < sub_division_level; level++)
							{
								if (x & level_mask)
								{
									glm::ivec3 l_div = vol_div / (1 << (sub_division_level - level));
									int lx = x >> (sub_division_level - level);
									int ly = y >> (sub_division_level - level);
									int lz = z >> (sub_division_level - level);
									int idx_l = lx + (ly + lz * l_div.y) * l_div.x;
									volumes[level][idx_l] = 1;
								}
								else
								{
									break;
								}
								level_mask >>= 1;
							}
						}
						if (dir_mask & 2)
						{
							unsigned level_mask = (1 << sub_division_level) - 1;
							for (int level = 0; level < sub_division_level; level++)
							{
								if (y & level_mask)
								{
									glm::ivec3 l_div = vol_div / (1 << (sub_division_level - level));
									int lx = x >> (sub_division_level - level);
									int ly = y >> (sub_division_level - level);
									int lz = z >> (sub_division_level - level);
									int idx_l = lx + (ly + lz * l_div.y) * l_div.x;
									volumes[level][idx_l] = 1;
								}
								else
								{
									break;
								}
								level_mask >>= 1;
							}

						}
						if (dir_mask & 4)
						{
							unsigned level_mask = (1 << sub_division_level) - 1;
							for (int level = 0; level < sub_division_level; level++)
							{
								if (z & level_mask)
								{
									glm::ivec3 l_div = vol_div / (1 << (sub_division_level - level));
									int lx = x >> (sub_division_level - level);
									int ly = y >> (sub_division_level - level);
									int lz = z >> (sub_division_level - level);
									int idx_l = lx + (ly + lz * l_div.y) * l_div.x;
									volumes[level][idx_l] = 1;
								}
								else
								{
									break;
								}
								level_mask >>= 1;
							}

						}

					}
				}
			}
		}

#if RADIANCE_DIFF
		{
			typedef std::vector<glm::vec4> RadianceVolume;
			std::vector<RadianceVolume> RadVols(sub_division_level + 1);

			{
				RadianceVolume& vol = RadVols[sub_division_level];
				vol.resize(vol_div.x * vol_div.y * vol_div.z);

				int num_directions = 256;
				int max_probes = (1 << 17) / num_directions;
				if (max_probes < 1) max_probes = 1;

				int total_num_probes = vol_div.x * vol_div.y * vol_div.z;

				BVHRenderer bvh_renderer;
				BVHRenderTarget bvh_target;

				int start_idx = 0;
				while (start_idx < total_num_probes)
				{
					int num_probes = total_num_probes - start_idx;
					if (num_probes > max_probes) num_probes = max_probes;

					bvh_target.update(num_directions, num_probes);

					ProbeRayList prl(coverage_min, coverage_max, vol_div, start_idx, start_idx + num_probes, num_directions);
					bvh_renderer.render_probe(scene, prl, bvh_target);

					std::vector<glm::vec4> rgba(num_probes * num_directions);

					glBindTexture(GL_TEXTURE_2D, bvh_target.m_tex_video->tex_id);
					glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, rgba.data());
					glBindTexture(GL_TEXTURE_2D, 0);

					for (int i = 0; i < num_probes; i++)
					{
						const glm::vec4* p_line = rgba.data() + i * num_directions;
						glm::vec3 sum_rad(0.0f);
						for (int j = 0; j < num_directions; j++)
						{
							sum_rad += glm::vec3(p_line[j]);
						}
						glm::vec3 ave_rad = sum_rad / (float)num_directions;
						vol[i + start_idx] = glm::vec4(ave_rad, glm::dot(ave_rad, ave_rad));
					}

					start_idx += num_probes;
				}
			}

			float threshold = 0.006f;
			for (int level = sub_division_level - 1; level >=0; level--)
			{
				glm::ivec3 l_div = vol_div / (1 << (sub_division_level - level));
				RadVols[level].resize(l_div.x * l_div.y * l_div.z);
				RadianceVolume& rvol = RadVols[level];
				RadianceVolume& rvol1 = RadVols[level + 1];
				Volume& vol = volumes[level];

				for (int z = 0; z < l_div.z; z++)
				{
					for (int y = 0; y < l_div.y; y++)
					{
						for (int x = 0; x < l_div.x; x++)
						{
							glm::vec4 sum(0.0);

							for (int dz = 0; dz < 2; dz++)
							{
								for (int dy = 0; dy < 2; dy++)
								{
									for (int dx = 0; dx < 2; dx++)
									{
										glm::ivec3 idx = glm::ivec3(x, y, z) * 2 + glm::ivec3(dx, dy, dz);
										sum += rvol1[idx.x + (idx.y + idx.z * l_div.y * 2) * l_div.x * 2];
									}
								}
							}
							glm::vec4 ave = sum / 8.0f;
							glm::vec3 ave_rad = glm::vec3(ave);
							float ave_rad2 = glm::dot(ave_rad, ave_rad);

							glm::ivec3 idx = glm::ivec3(x, y, z);
							rvol[idx.x + (idx.y + idx.z * l_div.y) * l_div.x] = glm::vec4(ave_rad, ave_rad2);

							float var = ave.w - ave_rad2;
							if (var > threshold)
							{
								vol[idx.x + (idx.y + idx.z * l_div.y) * l_div.x] = 1;
							}
						}
					}
				}
			}

			for (int level = sub_division_level - 2; level >= 0; level--)
			{
				glm::ivec3 l_div = vol_div / (1 << (sub_division_level - level));
				Volume& vol = volumes[level];
				Volume& vol1 = volumes[level + 1];

				for (int z = 0; z < l_div.z; z++)
				{
					for (int y = 0; y < l_div.y; y++)
					{
						for (int x = 0; x < l_div.x; x++)
						{
							glm::ivec3 idx = glm::ivec3(x, y, z);
							if (vol[idx.x + (idx.y + idx.z * l_div.y) * l_div.x] > 0) continue;							
							bool set_one = false;
							for (int dz = 0; dz < 2; dz++)
							{
								for (int dy = 0; dy < 2; dy++)
								{
									for (int dx = 0; dx < 2; dx++)
									{
										glm::ivec3 idx = glm::ivec3(x, y, z) * 2 + glm::ivec3(dx, dy, dz);
										if (vol1[idx.x + (idx.y + idx.z * l_div.y * 2) * l_div.x * 2] > 0)
										{
											set_one = true;
											break;
										}
									}
									if (set_one) break;
								}
								if (set_one) break;
							}
							if (set_one)
							{
								vol[idx.x + (idx.y + idx.z * l_div.y) * l_div.x] = 1;
							}
						}
					}
				}

			}
		}
#endif
	}

	struct ToDivide
	{
		int level;
		glm::ivec3 ipos;
	};

	std::queue<ToDivide> queue;

	Volume& vol0 = volumes[0];
	for (int z = 0; z < base_divisions.z; z++)
	{
		for (int y = 0; y < base_divisions.y; y++)
		{
			for (int x = 0; x < base_divisions.x; x++)
			{
				ToDivide td;
				td.level = 0;
				td.ipos = { x,y,z };
				queue.push(td);
			}
		}
	}

	std::vector<int> probe_idx;
	std::vector<int> sub_idx;

	int count_probe = 0;
	int count_sub = 0;

	glm::vec3 size_grid = coverage_max - coverage_min;
	while (queue.size() > 0)
	{
		ToDivide td = queue.front();
		queue.pop();

		int level = td.level;
		glm::ivec3 ipos = td.ipos;

		Volume& vol = volumes[level];
		glm::ivec3 divs = base_divisions * (1 << level);
		int idx = ipos.x + (ipos.y + ipos.z * divs.y) * divs.x;

		if (level < sub_division_level && vol[idx] > 0)
		{
			for (int z = 0; z < 2; z++)
			{
				for (int y = 0; y < 2; y++)
				{
					for (int x = 0; x < 2; x++)
					{
						glm::ivec3 delta = { x,y,z };
						glm::ivec3 ipos_sub = ipos * 2 + delta;
						int level_sub = level + 1;						
						ToDivide td;						
						td.level = level_sub;
						td.ipos = ipos_sub;
						queue.push(td);
					}
				}
			}

			probe_idx.push_back(-1);
			sub_idx.push_back(count_sub);
			count_sub++;
		}
		else
		{
			glm::vec3 norm_pos = (glm::vec3(ipos) + 0.5f) / glm::vec3(divs);
			glm::vec3 pos = coverage_min + size_grid * norm_pos;
			m_probe_data.push_back(glm::vec4(pos, float(level)));
			for (int i = 0; i < 9; i++)
			{
				m_probe_data.push_back(glm::vec4(0.0f));
			}

			probe_idx.push_back(count_probe);
			sub_idx.push_back(-1);
			count_probe++;
		}
	}

	m_sub_index.resize(probe_idx.size());
	for (size_t i = 0; i < probe_idx.size(); i++)
	{
		int idx = probe_idx[i];
		if (idx < 0)
		{
			idx = sub_idx[i] + count_probe;
		}
		m_sub_index[i] = idx;
	}

	updateBuffers();
}

inline glm::vec2 signNotZero(const glm::vec2& v)
{
	return glm::vec2((v.x >= 0.0f) ? 1.0f : -1.0f, (v.y >= 0.0f) ? 1.0f : -1.0f);
}

inline glm::vec3 oct_to_vec3(const glm::vec2& e)
{
	glm::vec3 v = glm::vec3(glm::vec2(e.x, e.y), 1.0f - fabsf(e.x) - fabsf(e.y));
	if (v.z < 0.0f)
	{
		glm::vec2 tmp = (1.0f - glm::abs(glm::vec2(v.y, v.x))) * signNotZero(glm::vec2(v.x, v.y));
		v.x = tmp.x;
		v.y = tmp.y;
	}
	return glm::normalize(v);
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

void LODProbeGrid::construct_visibility(Scene& scene)
{
	std::vector<Object3D*> objects;
	auto* p_objects = &objects;

	scene.traverse([p_objects](Object3D* obj) {
		do
		{
			{
				SimpleModel* model = dynamic_cast<SimpleModel*>(obj);
				if (model)
				{
					p_objects->push_back(model);
					break;
				}
			}
			{
				GLTFModel* model = dynamic_cast<GLTFModel*>(obj);
				if (model)
				{
					p_objects->push_back(model);
					break;
				}
			}
		} while (false);
		});

	BoundingVolumeHierarchy bvh(objects);

	int num_probes = getNumberOfProbes();
	int num_rays = 256;

	glm::mat4 rand_rot = rand_rotation();

	struct Sample
	{
		int ray_id;
		float weight;
	};

	std::vector<std::vector<Sample>> samples(vis_res* vis_res);
	
	for (int y = 0; y < vis_res; y++)
	{
		for (int x = 0; x < vis_res; x++)
		{
			glm::vec2 v2 = glm::vec2(float(x) + 0.5f, float(y) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
			glm::vec3 out_dir = oct_to_vec3(v2);

			for (int ray_id = 0; ray_id < num_rays; ray_id++)
			{
				glm::vec3 sf = sphericalFibonacci(ray_id, num_rays);
				glm::vec3 in_dir = glm::vec3(rand_rot * glm::vec4(sf, 0.0f));

				float weight = powf(glm::max(0.0f, glm::dot(in_dir, out_dir)), 50.0);
				if (weight > 1e-6f)
				{
					samples[x + y * vis_res].push_back({ ray_id, weight });
				}		
			}
		}
	}

	std::vector<float> f_mean_dis(vis_res* vis_res* num_probes);
	std::vector<float> f_mean_var(vis_res* vis_res* num_probes);

	float max_half = std::numeric_limits<half_float::half>::max();

	for (int index = 0; index < num_probes; index++)
	{
		glm::vec4 pos_lod = m_probe_data[index * 10];
		bvh::Ray<float> bvh_ray = {
			bvh::Vector3<float>(pos_lod.x, pos_lod.y, pos_lod.z),
			bvh::Vector3<float>(0.0f, 0.0f, 0.0f),
			0.0f,
			max_half
		};

		int lod = (int)pos_lod.w;
		float scale = 1.0f / float(1 << lod);

		std::vector<float> distance(num_rays);
		for (int ray_id = 0; ray_id < num_rays; ray_id++)
		{
			glm::vec3 sf = sphericalFibonacci(ray_id, num_rays);
			glm::vec3 dir = glm::vec3(rand_rot * glm::vec4(sf, 0.0f));

			float dis = max_half;
			bvh_ray.direction = bvh::Vector3<float>(dir.x, dir.y, dir.z);

			auto intersection = bvh.intersect(bvh_ray);
			if (intersection.has_value())
			{
				dis = intersection->distance();
			}
			distance[ray_id] = dis;
		}

		// filtering
		for (int y = 0; y < vis_res; y++)
		{
			for (int x = 0; x < vis_res; x++)
			{
				std::vector<Sample>& samples_pix = samples[x + y * vis_res];
				int num_samples = (int)samples_pix.size();

				float sum_dis = 0.0f;
				float sum_sqr_dis = 0.0f;
				float sum_weight = 0.0f;

				for (int s = 0; s < num_samples; s++)
				{
					int ray_id = samples_pix[s].ray_id;
					float weight = samples_pix[s].weight;
					float dis = distance[ray_id];
					if (dis == max_half) continue;
					sum_dis += weight * dis;
					sum_sqr_dis += weight * dis * dis;
					sum_weight += weight;
				}

				float mean_dis, mean_var;
				if (sum_weight > 0.0f)
				{
					mean_dis = sum_dis / sum_weight;
					float mean_sqr_dis = sum_sqr_dis / sum_weight;
					mean_var = sqrtf(mean_sqr_dis - mean_dis * mean_dis);
				}
				else
				{
					mean_dis = max_half;
					mean_var = 0.0f;
				}		

				f_mean_dis[x + y * vis_res + index * vis_res * vis_res] = std::min(mean_dis, max_half);
				f_mean_var[x + y * vis_res + index * vis_res * vis_res] = std::min(mean_var, max_half);
			}
		}
	}

	{
		m_visibility_data.resize(pack_res* pack_res * 2, 0);
		for (int index = 0; index < num_probes; index++)
		{
			for (int y = 0; y < vis_res; y++)
			{
				for (int x = 0; x < vis_res; x++)
				{
					float dis = f_mean_dis[x + y * vis_res + index * vis_res * vis_res];
					float mean_var = f_mean_var[x + y * vis_res + index * vis_res * vis_res];
					int out_x = (index % pack_size) * (vis_res + 2) + x + 1;
					int out_y = (index / pack_size) * (vis_res + 2) + y + 1;
					m_visibility_data[(out_x + out_y * pack_res) * 2] = dis;
					m_visibility_data[(out_x + out_y * pack_res) * 2 + 1] = mean_var;

				}
			}
			{
				float dis = f_mean_dis[(vis_res - 1) + (vis_res - 1) * vis_res + index * vis_res * vis_res];
				float mean_var = f_mean_var[(vis_res - 1) + (vis_res - 1) * vis_res + index * vis_res * vis_res];
				int out_x = (index % pack_size) * (vis_res + 2);
				int out_y = (index / pack_size) * (vis_res + 2);
				m_visibility_data[(out_x + out_y * pack_res) * 2] = dis;
				m_visibility_data[(out_x + out_y * pack_res) * 2 + 1] = mean_var;
			}
			{
				float dis = f_mean_dis[(vis_res - 1) * vis_res + index * vis_res * vis_res];
				float mean_var = f_mean_var[(vis_res - 1) * vis_res + index * vis_res * vis_res];
				int out_x = (index % pack_size) * (vis_res + 2) + vis_res + 1;
				int out_y = (index / pack_size) * (vis_res + 2);
				m_visibility_data[(out_x + out_y * pack_res) * 2] = dis;
				m_visibility_data[(out_x + out_y * pack_res) * 2 + 1] = mean_var;
			}
			{
				float dis = f_mean_dis[(vis_res - 1) + index * vis_res * vis_res];
				float mean_var = f_mean_var[(vis_res - 1) + index * vis_res * vis_res];
				int out_x = (index % pack_size) * (vis_res + 2);
				int out_y = (index / pack_size) * (vis_res + 2) + vis_res + 1;
				m_visibility_data[(out_x + out_y * pack_res) * 2] = dis;
				m_visibility_data[(out_x + out_y * pack_res) * 2 + 1] = mean_var;
			}

			{
				float dis = f_mean_dis[index * vis_res * vis_res];
				float mean_var = f_mean_var[index * vis_res * vis_res];
				int out_x = (index % pack_size) * (vis_res + 2) + vis_res + 1;
				int out_y = (index / pack_size) * (vis_res + 2) + vis_res + 1;
				m_visibility_data[(out_x + out_y * pack_res) * 2] = dis;
				m_visibility_data[(out_x + out_y * pack_res) * 2 + 1] = mean_var;
			}

			for (int x = 0; x < vis_res; x++)
			{
				{
					float dis = f_mean_dis[(vis_res - 1 - x) + index * vis_res * vis_res];
					float mean_var = f_mean_var[(vis_res - 1 - x) + index * vis_res * vis_res];
					int out_x = (index % pack_size) * (vis_res + 2) + x + 1;
					int out_y = (index / pack_size) * (vis_res + 2);
					m_visibility_data[(out_x + out_y * pack_res) * 2] = dis;
					m_visibility_data[(out_x + out_y * pack_res) * 2 + 1] = mean_var;
				}
				{
					float dis = f_mean_dis[(vis_res - 1 - x) + (vis_res - 1) * vis_res + index * vis_res * vis_res];
					float mean_var = f_mean_var[(vis_res - 1 - x) + (vis_res - 1) * vis_res + index * vis_res * vis_res];
					int out_x = (index % pack_size) * (vis_res + 2) + x + 1;
					int out_y = (index / pack_size) * (vis_res + 2) + vis_res + 1;
					m_visibility_data[(out_x + out_y * pack_res) * 2] = dis;
					m_visibility_data[(out_x + out_y * pack_res) * 2 + 1] = mean_var;
				}
			}
			for (int y = 0; y < vis_res; y++)
			{
				{
					float dis = f_mean_dis[(vis_res - 1 - y) * vis_res + index * vis_res * vis_res];
					float mean_var = f_mean_var[(vis_res - 1 - y) * vis_res + index * vis_res * vis_res];
					int out_x = (index % pack_size) * (vis_res + 2);
					int out_y = (index / pack_size) * (vis_res + 2) + y + 1;
					m_visibility_data[(out_x + out_y * pack_res) * 2] = dis;
					m_visibility_data[(out_x + out_y * pack_res) * 2 + 1] = mean_var;
				}

				{
					float dis = f_mean_dis[(vis_res - 1) + (vis_res - 1 - y) * vis_res + index * vis_res * vis_res];
					float mean_var = f_mean_var[(vis_res - 1) + (vis_res - 1 - y) * vis_res + index * vis_res * vis_res];
					int out_x = (index % pack_size) * (vis_res + 2) + vis_res + 1;
					int out_y = (index / pack_size) * (vis_res + 2) + y + 1;
					m_visibility_data[(out_x + out_y * pack_res) * 2] = dis;
					m_visibility_data[(out_x + out_y * pack_res) * 2 + 1] = mean_var;
				}
			}
		}

		/*FILE* fp = fopen("vis16_dmp.raw", "wb");
		fwrite(m_visibility_data.data(), 2, m_visibility_data.size(), fp);
		fclose(fp);*/
	}

	glBindTexture(GL_TEXTURE_2D, m_tex_visibility->tex_id);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pack_res, pack_res, GL_RG, GL_FLOAT, m_visibility_data.data());
	glBindTexture(GL_TEXTURE_2D, 0);
}

void LODProbeGrid::ToProbeGrid(ProbeGrid* grid_out, Scene& scene)
{
	grid_out->coverage_min = coverage_min;
	grid_out->coverage_max = coverage_max;
	grid_out->divisions = base_divisions * (1 << sub_division_level);
	grid_out->normal_bias = normal_bias;

	glm::ivec3 divs_out = grid_out->divisions;
	size_t num_out = divs_out.x * divs_out.y * divs_out.z;

	grid_out->vis_res = vis_res;
	int pack_size_out = int(ceilf(sqrtf(float(num_out))));
	int pack_res_out = pack_size_out * (vis_res + 2);
	grid_out->pack_size = pack_size_out;
	grid_out->pack_res = pack_res_out;

	grid_out->m_probe_data.resize(num_out * 9);
	grid_out->m_visibility_data.resize(pack_res_out * pack_res_out * 2);

	// build mapping;
	std::vector<int> indices(num_out);
	std::vector<int> levels(num_out);
	{
		struct Node
		{
			int idx;
			int level;
			glm::ivec3 ipos;
		};

		std::queue<Node> queue;

		for (int z = 0; z < base_divisions.z; z++)
		{
			for (int y = 0; y < base_divisions.y; y++)
			{
				for (int x = 0; x < base_divisions.x; x++)
				{
					int i = x + (y + z * base_divisions.y) * base_divisions.x;
					Node node;
					node.idx = m_sub_index[i];
					node.level = 0;
					node.ipos = { x,y,z };
					queue.push(node);
				}
			}
		}

		int num_probes = getNumberOfProbes();
		int base_offset = base_divisions.x * base_divisions.y * base_divisions.z;

		while (queue.size() > 0)
		{
			Node node = queue.front();
			queue.pop();

			int idx = node.idx;
			glm::ivec3 ipos = node.ipos;
			int level = node.level;
			if (idx < num_probes)
			{
				int range = 1 << (sub_division_level - level);
				glm::ivec3 offset = ipos * range;
				for (int z = 0; z < range; z++)
				{
					for (int y = 0; y < range; y++)
					{
						for (int x = 0; x < range; x++)
						{
							glm::ivec3 ipos_high = offset + glm::ivec3(x, y, z);
							int i = ipos_high.x + (ipos_high.y + ipos_high.z * divs_out.y) * divs_out.x;
							indices[i] = idx;
							levels[i] = level;
						}
					}
				}
			}
			else
			{
				idx -= num_probes;
				int offset = base_offset + idx * 8;
				for (int z = 0; z < 2; z++)
				{
					for (int y = 0; y < 2; y++)
					{
						for (int x = 0; x < 2; x++)
						{
							int i = x + y * 2 + z * 4;
							glm::ivec3 delta = { x,y,z };
							glm::ivec3 ipos_sub = ipos * 2 + delta;
							int level_sub = level + 1;
							glm::ivec3 sub_div = base_divisions * (1 << level_sub);
							Node sub;
							sub.idx = m_sub_index[offset + i];
							sub.level = level_sub;
							sub.ipos = ipos_sub;
							queue.push(sub);
						}
					}
				}
			}
		}
	}

	std::vector<Object3D*> objects;
	auto* p_objects = &objects;

	scene.traverse([p_objects](Object3D* obj) {
		do
		{
			{
				SimpleModel* model = dynamic_cast<SimpleModel*>(obj);
				if (model)
				{
					p_objects->push_back(model);
					break;
				}
			}
			{
				GLTFModel* model = dynamic_cast<GLTFModel*>(obj);
				if (model)
				{
					p_objects->push_back(model);
					break;
				}
			}
		} while (false);
		});
	BoundingVolumeHierarchy bvh(objects);

	glm::vec3 size_grid = coverage_max - coverage_min;

	int num_rays = 256;

	glm::mat4 rand_rot = rand_rotation();

	struct Sample
	{
		int ray_id;
		float weight;
	};

	std::vector<std::vector<Sample>> samples(vis_res* vis_res);

	for (int y = 0; y < vis_res; y++)
	{
		for (int x = 0; x < vis_res; x++)
		{
			glm::vec2 v2 = glm::vec2(float(x) + 0.5f, float(y) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
			glm::vec3 out_dir = oct_to_vec3(v2);

			for (int ray_id = 0; ray_id < num_rays; ray_id++)
			{
				glm::vec3 sf = sphericalFibonacci(ray_id, num_rays);
				glm::vec3 in_dir = glm::vec3(rand_rot * glm::vec4(sf, 0.0f));

				float weight = powf(glm::max(0.0f, glm::dot(in_dir, out_dir)), 50.0);
				if (weight > 1e-6f)
				{
					samples[x + y * vis_res].push_back({ ray_id, weight });
				}
			}
		}
	}

	float max_half = std::numeric_limits<half_float::half>::max();
	for (int gz = 0; gz < divs_out.z; gz++)
	{
		for (int gy = 0; gy < divs_out.y; gy++)
		{
			for (int gx = 0; gx < divs_out.x; gx++)
			{
				int i = gx + (gy + gz * divs_out.y) * divs_out.x;
				int idx = indices[i];
				int level = levels[i];
				memcpy(grid_out->m_probe_data.data() + i * 9, m_probe_data.data() + idx * 10 + 1, sizeof(glm::vec4) * 9);

				if (level == sub_division_level)
				{
					// copy visibility
					for (int y = 0; y < vis_res + 2; y++)
					{
						for (int x = 0; x < vis_res + 2; x++)
						{
							int in_x = (idx % pack_size) * (vis_res + 2) + x;
							int in_y = (idx / pack_size) * (vis_res + 2) + y;
							int out_x = (i % pack_size_out) * (vis_res + 2) + x;
							int out_y = (i / pack_size_out) * (vis_res + 2) + y;
							const float* p_in = m_visibility_data.data() + (in_x + in_y * pack_res) * 2;
							float* p_out = grid_out->m_visibility_data.data() + (out_x + out_y * pack_res_out) * 2;
							p_out[0] = p_in[0];
							p_out[1] = p_in[1];
						}
					}
				}
				else
				{
					// reconstruct visibility
					glm::ivec3 idx(gx, gy, gz);
					glm::vec3 pos_normalized = (glm::vec3(idx) + 0.5f) / glm::vec3(divs_out);
					glm::vec3 pos = coverage_min + pos_normalized * size_grid;

					bvh::Ray<float> bvh_ray = {
						bvh::Vector3<float>(pos.x, pos.y, pos.z),
						bvh::Vector3<float>(0.0f, 0.0f, 0.0f),
						0.0f,
						max_half
					};

					std::vector<float> distance(num_rays);
					for (int ray_id = 0; ray_id < num_rays; ray_id++)
					{
						glm::vec3 sf = sphericalFibonacci(ray_id, num_rays);
						glm::vec3 dir = glm::vec3(rand_rot * glm::vec4(sf, 0.0f));					

						float dis = max_half;

						bvh_ray.direction = bvh::Vector3<float>(dir.x, dir.y, dir.z);
	
						auto intersection = bvh.intersect(bvh_ray);
						if (intersection.has_value())
						{
							dis = intersection->distance();
						}
						distance[ray_id] = dis;						
					}

					// filtering
					std::vector<float> f_mean_dis(vis_res* vis_res);
					std::vector<float> f_mean_var(vis_res* vis_res);
					for (int y = 0; y < vis_res; y++)
					{
						for (int x = 0; x < vis_res; x++)
						{
							std::vector<Sample>& samples_pix = samples[x + y * vis_res];
							int num_samples = (int)samples_pix.size();

							float sum_dis = 0.0f;
							float sum_sqr_dis = 0.0f;
							float sum_weight = 0.0f;

							for (int s = 0; s < num_samples; s++)
							{
								int ray_id = samples_pix[s].ray_id;
								float weight = samples_pix[s].weight;
								float dis = distance[ray_id];
								if (dis == max_half) continue;
								sum_dis += weight * dis;
								sum_sqr_dis += weight * dis * dis;
								sum_weight += weight;
							}	

							float mean_dis, mean_var;
							if (sum_weight > 0.0f)
							{
								mean_dis = sum_dis / sum_weight;
								float mean_sqr_dis = sum_sqr_dis / sum_weight;
								mean_var = sqrtf(mean_sqr_dis - mean_dis * mean_dis);
							}
							else
							{
								mean_dis = max_half;
								mean_var = 0.0f;
							}							

							f_mean_dis[x + y * vis_res] = std::min(mean_dis, max_half);
							f_mean_var[x + y * vis_res] = std::min(mean_var, max_half);
						}
					}					
					

					for (int y = 0; y < vis_res; y++)
					{
						for (int x = 0; x < vis_res; x++)
						{
							float dis = f_mean_dis[x + y * vis_res];
							float mean_var = f_mean_var[x + y * vis_res];
							int out_x = (i % pack_size_out) * (vis_res + 2) + x + 1;
							int out_y = (i / pack_size_out) * (vis_res + 2) + y + 1;
							grid_out->m_visibility_data[(out_x + out_y * pack_res_out) * 2] = dis;
							grid_out->m_visibility_data[(out_x + out_y * pack_res_out) * 2 + 1] = mean_var;
						}
					}

					{
						float dis = f_mean_dis[(vis_res - 1) + (vis_res - 1) * vis_res];
						float mean_var = f_mean_var[(vis_res - 1) + (vis_res - 1) * vis_res];
						int out_x = (i % pack_size_out) * (vis_res + 2);
						int out_y = (i / pack_size_out) * (vis_res + 2);
						grid_out->m_visibility_data[(out_x + out_y * pack_res_out) * 2] = dis;
						grid_out->m_visibility_data[(out_x + out_y * pack_res_out) * 2 + 1] = mean_var;
					}
					{
						float dis = f_mean_dis[(vis_res - 1) * vis_res];
						float mean_var = f_mean_var[(vis_res - 1) * vis_res];
						int out_x = (i % pack_size_out) * (vis_res + 2) + vis_res + 1;
						int out_y = (i / pack_size_out) * (vis_res + 2);
						grid_out->m_visibility_data[(out_x + out_y * pack_res_out) * 2] = dis;
						grid_out->m_visibility_data[(out_x + out_y * pack_res_out) * 2 + 1] = mean_var;
					}

					{
						float dis = f_mean_dis[(vis_res - 1)];
						float mean_var = f_mean_var[(vis_res - 1)];
						int out_x = (i % pack_size_out) * (vis_res + 2);
						int out_y = (i / pack_size_out) * (vis_res + 2) + vis_res + 1;
						grid_out->m_visibility_data[(out_x + out_y * pack_res_out) * 2] = dis;
						grid_out->m_visibility_data[(out_x + out_y * pack_res_out) * 2 + 1] = mean_var;
					}

					{
						float dis = f_mean_dis[0];
						float mean_var = f_mean_var[0];
						int out_x = (i % pack_size_out) * (vis_res + 2) + vis_res + 1;
						int out_y = (i / pack_size_out) * (vis_res + 2) + vis_res + 1;
						grid_out->m_visibility_data[(out_x + out_y * pack_res_out) * 2] = dis;
						grid_out->m_visibility_data[(out_x + out_y * pack_res_out) * 2 + 1] = mean_var;
					}

					for (int x = 0; x < vis_res; x++)
					{
						{
							float dis = f_mean_dis[(vis_res - 1 - x) ];
							float mean_var = f_mean_var[(vis_res - 1 - x)];
							int out_x = (i % pack_size_out) * (vis_res + 2) + x + 1;
							int out_y = (i / pack_size_out) * (vis_res + 2);
							grid_out->m_visibility_data[(out_x + out_y * pack_res_out) * 2] = dis;
							grid_out->m_visibility_data[(out_x + out_y * pack_res_out) * 2 + 1] = mean_var;
						}
						{
							float dis = f_mean_dis[(vis_res - 1 - x) + (vis_res - 1) * vis_res];
							float mean_var = f_mean_var[(vis_res - 1 - x) + (vis_res - 1) * vis_res];
							int out_x = (i % pack_size_out) * (vis_res + 2) + x + 1;
							int out_y = (i / pack_size_out) * (vis_res + 2) + vis_res + 1;
							grid_out->m_visibility_data[(out_x + out_y * pack_res_out) * 2] = dis;
							grid_out->m_visibility_data[(out_x + out_y * pack_res_out) * 2 + 1] = mean_var;
						}
					}

					for (int y = 0; y < vis_res; y++)
					{
						{
							float dis = f_mean_dis[(vis_res - 1 - y) * vis_res];
							float mean_var = f_mean_var[(vis_res - 1 - y) * vis_res];
							int out_x = (i % pack_size_out) * (vis_res + 2);
							int out_y = (i / pack_size_out) * (vis_res + 2) + y + 1;
							grid_out->m_visibility_data[(out_x + out_y * pack_res_out) * 2] = dis;
							grid_out->m_visibility_data[(out_x + out_y * pack_res_out) * 2 + 1] = mean_var;
						}

						{
							float dis = f_mean_dis[(vis_res - 1) + (vis_res - 1 - y) * vis_res];
							float mean_var = f_mean_var[(vis_res - 1) + (vis_res - 1 - y) * vis_res];
							int out_x = (i % pack_size_out) * (vis_res + 2) + vis_res + 1;
							int out_y = (i / pack_size_out) * (vis_res + 2) + y + 1;
							grid_out->m_visibility_data[(out_x + out_y * pack_res_out) * 2] = dis;
							grid_out->m_visibility_data[(out_x + out_y * pack_res_out) * 2 + 1] = mean_var;
						}
					}
				}
				
			}
		}
	}

	grid_out->allocate_probes();
}


void LODProbeGrid::download_probes()
{
	{		
		int num_probes = getNumberOfProbes();
		for (int i = 0; i < 10; i++)
		{
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_probe_bufs[i]->m_id);
			const glm::vec4* p_data = (const glm::vec4*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
			for (size_t j = 0; j < num_probes; j++)
			{
				m_probe_data[j * 10 + i] = p_data[j];
			}
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}		
	}
	{
		glBindTexture(GL_TEXTURE_2D, m_tex_visibility->tex_id);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, m_visibility_data.data());
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	updated = false;
}

int LODProbeGrid::get_probe_idx(const glm::ivec3& ipos) const
{
	int base_offset = base_divisions.x * base_divisions.y * base_divisions.z;

	glm::ivec3 ipos_base = ipos / (1 << sub_division_level);
	int node_idx = ipos_base.x + (ipos_base.y + ipos_base.z * base_divisions.y) * base_divisions.x;
	int probe_idx = m_sub_index[node_idx];

	int lod = 0;
	int digit_mask = 1 << (sub_division_level - 1);

	int num_probes = getNumberOfProbes();
	while (lod < sub_division_level && probe_idx >= num_probes)
	{
		int offset = base_offset + (probe_idx - num_probes) * 8;
		int sub = 0;
		if ((ipos.x & digit_mask) != 0) sub += 1;
		if ((ipos.y & digit_mask) != 0) sub += 2;
		if ((ipos.z & digit_mask) != 0) sub += 4;
		node_idx = offset + sub;
		probe_idx = m_sub_index[node_idx];

		lod++;
		digit_mask >>= 1;
	}

	return probe_idx;

}

void LODProbeGrid::get_probe(const glm::vec3& position, EnvironmentMap& envMap) const
{
	glm::ivec3 divs = base_divisions * (1 << sub_division_level);

	glm::vec3 size_grid = coverage_max - coverage_min;
	glm::vec3 pos_normalized = (position - coverage_min) / size_grid;
	glm::vec3 pos_voxel = pos_normalized * glm::vec3(divs) - 0.5f;
	pos_voxel = glm::clamp(pos_voxel, glm::vec3(0.0f), glm::vec3(divs) - 1.0f);

	glm::ivec3 i_voxel = glm::clamp(glm::ivec3(pos_voxel), glm::ivec3(0), glm::ivec3(divs) - glm::ivec3(2));
	glm::vec3 frac_voxel = pos_voxel - glm::vec3(i_voxel);

	float sum_weight = 0.0f;
	glm::vec4 sumSH[9];
	for (int i = 0; i < 9; i++)
	{
		sumSH[i] = glm::vec4(0.0f);
	}

	for (int z = 0; z < 2; z++)
	{
		for (int y = 0; y < 2; y++)
		{
			for (int x = 0; x < 2; x++)
			{
				glm::vec3 w = glm::vec3(1.0f) - glm::abs(glm::vec3(x, y, z) - frac_voxel);
				float weight = w.x * w.y * w.z;
				if (weight > 0.0f)
				{
					sum_weight += weight;

					glm::ivec3 vert = i_voxel + glm::ivec3(x, y, z);
					int idx_probe = get_probe_idx(vert);
					const glm::vec4* p_sh = m_probe_data.data() + idx_probe * 10 + 1;
					for (int i = 0; i < 9; i++)
					{
						sumSH[i] += p_sh[i] * weight;
					}
				}
			}
		}
	}

	if (sum_weight > 0.0f)
	{
		for (int i = 0; i < 9; i++)
		{
			envMap.shCoefficients[i] = sumSH[i] / sum_weight;
		}
	}
	else
	{
		memset(envMap.shCoefficients, 0, sizeof(glm::vec4) * 9);
	}

	envMap.updateConstant();
}

