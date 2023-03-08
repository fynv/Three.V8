#include <GL/glew.h>
#include <queue>
#include "LODProbeGrid.h"
#include "renderers/GLRenderer.h"

#include "scenes/Scene.h"
#include "models/SimpleModel.h"
#include "models/GLTFModel.h"
#include "core/BoundingVolumeHierarchy.h"

#include "EnvironmentMapCreator.h"


inline double rand01()
{
	return (double)rand() / ((double)RAND_MAX + 1.0);
}


struct LODProbeGridConst
{
	glm::vec4 coverageMin;
	glm::vec4 coverageMax;
	glm::ivec4 baseDivisions;	
	int subDivisionLevel;
	float normalBias;
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
	m_tex_lod = std::unique_ptr<GLTexture3D>(new GLTexture3D);
	m_tex_irradiance = std::unique_ptr<GLTexture2D>(new GLTexture2D);
	m_tex_visibility = std::unique_ptr<GLTexture2D>(new GLTexture2D);
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

	glBindTexture(GL_TEXTURE_2D, m_tex_irradiance->tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, irr_pack_res, irr_pack_res, 0, GL_RGB, GL_FLOAT, irradiance_img.data());	
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

void LODProbeGrid::_create_lod_tex()
{
	typedef std::vector<uint8_t> Volume;
	glm::ivec3 vol_div = base_divisions * (1 << sub_division_level);
	Volume lod_data(vol_div.x* vol_div.y* vol_div.z, 0);

	for (int z = 0; z < vol_div.z; z++)
	{
		for (int y = 0; y < vol_div.y; y++)
		{
			for (int x = 0; x < vol_div.x; x++)
			{
				glm::ivec3 ipos = glm::ivec3(x, y, z);
				glm::ivec3 ipos_base = ipos / (1 << sub_division_level);
				int probe_idx = ipos_base.x + (ipos_base.y + ipos_base.z * base_divisions.y) * base_divisions.x;
				int idx_sub = m_sub_index[probe_idx];
				int base_offset = base_divisions.x * base_divisions.y * base_divisions.z;

				int lod = 0;
				int digit_mask = 1 << (sub_division_level - 1);
				while (lod < sub_division_level && idx_sub >= 0)
				{
					int offset = base_offset + idx_sub * 8;
					int sub = 0;
					if ((ipos.x & digit_mask) != 0) sub += 1;
					if ((ipos.y & digit_mask) != 0) sub += 2;
					if ((ipos.z & digit_mask) != 0) sub += 4;
					probe_idx = offset + sub;

					if (lod < sub_division_level - 1)
					{
						idx_sub = m_sub_index[probe_idx];
					}
					else
					{
						idx_sub = -1;
					}
					lod++;
					digit_mask >>= 1;
				}
				int idx = x + (y + z * vol_div.y) * vol_div.x;
				lod_data[idx] = lod;
			}
		}
	}
	m_tex_lod->load_memory(vol_div.x, vol_div.y, vol_div.z, lod_data.data());
}

void LODProbeGrid::updateBuffers()
{
	_create_lod_tex();

	m_sub_index_buf = std::unique_ptr<GLBuffer>(new GLBuffer(sizeof(int) * m_sub_index.size(), GL_SHADER_STORAGE_BUFFER));
	m_sub_index_buf->upload(m_sub_index.data());

	m_probe_buf = std::unique_ptr<GLBuffer>(new GLBuffer(sizeof(glm::vec4) * m_probe_data.size(), GL_SHADER_STORAGE_BUFFER));
	m_probe_buf->upload(m_probe_data.data());

	_presample_irradiance();
	
	int num_probes = getNumberOfProbes();
	pack_size = int(ceilf(sqrtf(float(num_probes))));
	pack_res = pack_size * (vis_res + 2);
	m_visibility_data.resize(pack_res * pack_res * 2, 0);

	glBindTexture(GL_TEXTURE_2D, m_tex_visibility->tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16, pack_res, pack_res, 0, GL_RG, GL_UNSIGNED_SHORT, m_visibility_data.data());
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


void LODProbeGrid::_initialize(GLRenderer& renderer, Scene& scene, int probe_budget)
{
	m_sub_index.clear();
	m_sub_index.resize(base_divisions.x * base_divisions.y * base_divisions.z, -1);
	m_probe_data.clear();
	m_probe_data.resize(base_divisions.x * base_divisions.y * base_divisions.z * 10, glm::vec4(0.0f));
	m_visibility_data.clear();

	glm::vec3 size_grid = coverage_max - coverage_min;
	for (int z = 0; z < base_divisions.z; z++)
	{
		for (int y = 0; y < base_divisions.y; y++)
		{
			for (int x = 0; x < base_divisions.x; x++)
			{
				int idx = x + (y + z * base_divisions.y) * base_divisions.x;
				glm::vec4& position = m_probe_data[idx * 10];
				glm::ivec3 ipos = { x,y,z };
				glm::vec3 norm_pos = (glm::vec3(ipos) + 0.5f) / glm::vec3(base_divisions);
				glm::vec3 pos = coverage_min + size_grid * norm_pos;
				position = glm::vec4(pos, 0.0f);
			}
		}
	}

	if (sub_division_level > 0)
	{
		typedef std::vector<uint8_t> Volume;
		std::vector<Volume> volumes(sub_division_level+1);

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
						int idx = x + (y + z  * vol_div.y) * vol_div.x;
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

		if (probe_budget > 0)
		{
			glm::ivec3 vol_div = base_divisions * (1 << (sub_division_level-1));
			int count = 0;
			Volume& vol = volumes[sub_division_level - 1];
			for (int z = 0; z < vol_div.z; z++)
			{
				for (int y = 0; y < vol_div.y; y++)
				{
					for (int x = 0; x < vol_div.x; x++)
					{
						int idx = x + (y + z * vol_div.y) * vol_div.x;
						if (vol[idx] > 0)
						{
							count++;
						}
					}
				}
			}

			int base_count = base_divisions.x * base_divisions.y * base_divisions.z;
			int root_count = 0;
			for (int level = 0; level < sub_division_level; level++)
			{
				root_count += base_count * (1 << (3 * level));
			}
			int est_probes = root_count + (base_count * (1 << (3 * sub_division_level))) * count / (vol_div.x * vol_div.y * vol_div.z);

			if (probe_budget < est_probes)
			{
				float pass_rate = float(probe_budget - root_count) / float(est_probes - root_count);
				if (pass_rate < 0.0f) pass_rate = 0.0f;
				for (int z = 0; z < vol_div.z; z++)
				{
					for (int y = 0; y < vol_div.y; y++)
					{
						for (int x = 0; x < vol_div.x; x++)
						{
							int idx = x + (y + z * vol_div.y) * vol_div.x;
							if (vol[idx] > 0)
							{
								float r = rand01();
								if (r > pass_rate)
								{
									vol[idx] = 0;
								}
							}
						}
					}
				}
			}
		}

		struct ToDivide
		{
			int idx;
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
					int idx = x + (y + z * base_divisions.y) * base_divisions.x;					
					if (vol0[idx] > 0)
					{
						ToDivide td;
						td.idx = idx;
						td.level = 0;
						td.ipos = { x,y,z };
						queue.push(td);
					}
				}
			}
		}


		int ref_idx = 0;

		while (queue.size() > 0)
		{
			ToDivide td = queue.front();
			queue.pop();
			
			m_sub_index[td.idx] = ref_idx;
			ref_idx++;

			for (int z = 0; z < 2; z++)
			{
				for (int y = 0; y < 2; y++)
				{
					for (int x = 0; x < 2; x++)
					{						
						glm::ivec3 delta = { x,y,z };
						glm::ivec3 ipos_sub = td.ipos * 2 + delta;
						int level_sub = td.level + 1;				
						glm::ivec3 sub_div = base_divisions * (1 << level_sub);
						glm::vec3 sub_norm_pos = (glm::vec3(ipos_sub) + 0.5f) / glm::vec3(sub_div);
						glm::vec3 sub_pos = coverage_min + size_grid * sub_norm_pos;
						m_probe_data.push_back(glm::vec4(sub_pos, float(level_sub)));
						for (int i = 0; i < 9; i++)
						{
							m_probe_data.push_back(glm::vec4(0.0f));
						}

						if (level_sub < sub_division_level)
						{
							int push_idx = (int)m_sub_index.size();
							m_sub_index.push_back(-1);
							Volume& vol = volumes[level_sub];
							int idx_sub = ipos_sub.x + (ipos_sub.y + ipos_sub.z * sub_div.y) * sub_div.x;
							if (vol[idx_sub] > 0)
							{
								ToDivide td;
								td.idx = push_idx;
								td.level = level_sub;
								td.ipos = ipos_sub;
								queue.push(td);
							}
						}
					}
				}
			}
		}

		/*int count_divided = 0;
		for (int i = 0; i < m_sub_index.size(); i++)
		{
			if (m_sub_index[i] >= 0) count_divided++;
		}
		printf("%d/%d\n", count_divided, getNumberOfProbes());*/
		//int num_probes = getNumberOfProbes();
		//printf("%d %d %d\n", num_probes, base_divisions.x* base_divisions.y* base_divisions.z, ref_idx*8);

	}		
	
}

void LODProbeGrid::initialize(GLRenderer& renderer, Scene& scene, int probe_budget)
{
	if (probe_budget > 0)
	{
		int base_count = base_divisions.x * base_divisions.y * base_divisions.z;
		int est_count = base_count;
		int lod = 0;
		while (est_count < probe_budget)
		{
			lod++;
			est_count += base_count * (1 << (3 * lod));
		}
		sub_division_level = lod;
	}
	_initialize(renderer, scene, probe_budget);
	
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

	glm::vec3 size_grid = coverage_max - coverage_min;
	glm::vec3 spacing = size_grid / glm::vec3(base_divisions);
	float max_visibility = glm::length(spacing);

	int num_probes = getNumberOfProbes();

	std::vector<float> f_mean_dis(vis_res * vis_res * num_probes, max_visibility);	
	std::vector<float> f_mean_var(vis_res * vis_res * num_probes, max_visibility);

	for (int index = 0; index < num_probes; index++)
	{
		glm::vec4 pos_lod = m_probe_data[index * 10];
		bvh::Ray<float> bvh_ray = {
			bvh::Vector3<float>(pos_lod.x, pos_lod.y, pos_lod.z),
			bvh::Vector3<float>(0.0f, 0.0f, 0.0f),
			0.0f,
			max_visibility
		};

		int lod = (int)pos_lod.w;
		float scale = 1.0f / float(1 << lod);

		std::vector<float> distance(vis_res * vis_res);
		std::vector<float> sqr_dis(vis_res * vis_res);
		for (int y = 0; y < vis_res; y++)
		{
			for (int x = 0; x < vis_res; x++)
			{
				glm::vec2 v2 = glm::vec2(float(x) + 0.5f, float(y) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
				glm::vec3 dir = oct_to_vec3(v2);

				glm::vec3 dir_abs = glm::abs(dir) / spacing;

				int major_dir = 0;
				if (dir_abs.y > dir_abs.x)
				{
					if (dir_abs.z > dir_abs.y)
					{
						major_dir = 2;
					}
					else
					{
						major_dir = 1;
					}
				}
				else
				{
					if (dir_abs.z > dir_abs.x)
					{
						major_dir = 2;
					}
				}

				if (major_dir == 0)
				{
					dir_abs *= spacing.x * scale / dir_abs.x;
				}
				else if (major_dir == 1)
				{
					dir_abs *= spacing.y * scale / dir_abs.y;
				}
				else if (major_dir == 2)
				{
					dir_abs *= spacing.z * scale / dir_abs.z;
				}

				float max_dis = glm::length(dir_abs);
				float dis = max_dis;

				bvh_ray.direction = bvh::Vector3<float>(dir.x, dir.y, dir.z);
				bvh_ray.tmax = max_dis;
				auto intersection = bvh.intersect(bvh_ray);
				if (intersection.has_value())
				{					
					dis = intersection->distance();
				}		
				distance[x + y * vis_res] = dis;		
				sqr_dis[x + y * vis_res] = dis * dis;
			}			
		}

		// filtering
		float power = 50.0f;
		std::vector<float> filtered_distance(vis_res* vis_res);
		std::vector<float> filtered_sqr_dis(vis_res* vis_res);
		for (int y = 0; y < vis_res; y++)
		{
			for (int x = 0; x < vis_res; x++)
			{
				float sum_dis = 0.0f;
				float sum_sqr_dis = 0.0f;
				float sum_weight = 0.0f;

				glm::vec2 v2_0 = glm::vec2(float(x) + 0.5f, float(y) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
				glm::vec3 dir_0 = oct_to_vec3(v2_0);
				{
					sum_dis += distance[x + y * vis_res];
					sum_sqr_dis += sqr_dis[x + y * vis_res];
					sum_weight += 1.0f;
				}

				if (x > 0)
				{
					{
						glm::vec2 v2_1 = glm::vec2(float(x - 1) + 0.5f, float(y) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
						glm::vec3 dir_1 = oct_to_vec3(v2_1);
						float weight = powf(glm::dot(dir_0, dir_1), power);
						sum_dis += distance[(x - 1) + y * vis_res] * weight;
						sum_sqr_dis += sqr_dis[(x - 1) + y * vis_res] * weight;
						sum_weight += weight;
					}

					if (y > 0)
					{
						glm::vec2 v2_1 = glm::vec2(float(x - 1) + 0.5f, float(y - 1) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
						glm::vec3 dir_1 = oct_to_vec3(v2_1);
						float weight = powf(glm::dot(dir_0, dir_1), power);
						sum_dis += distance[(x - 1) + (y - 1) * vis_res] * weight;
						sum_sqr_dis += sqr_dis[(x - 1) + (y - 1) * vis_res] * weight;
						sum_weight += weight;
					}
					else
					{
						glm::vec2 v2_1 = glm::vec2(float(vis_res - x) + 0.5f, float(y) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
						glm::vec3 dir_1 = oct_to_vec3(v2_1);
						float weight = powf(glm::dot(dir_0, dir_1), power);
						sum_dis += distance[(vis_res - x) + y * vis_res] * weight;
						sum_sqr_dis += sqr_dis[(vis_res - x) + y * vis_res] * weight;
						sum_weight += weight;
					}

					if (y < vis_res - 1)
					{
						glm::vec2 v2_1 = glm::vec2(float(x - 1) + 0.5f, float(y + 1) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
						glm::vec3 dir_1 = oct_to_vec3(v2_1);
						float weight = powf(glm::dot(dir_0, dir_1), power);
						sum_dis += distance[(x - 1) + (y + 1) * vis_res] * weight;
						sum_sqr_dis += sqr_dis[(x - 1) + (y + 1) * vis_res] * weight;
						sum_weight += weight;
					}
					else
					{
						glm::vec2 v2_1 = glm::vec2(float(vis_res - x) + 0.5f, float(y) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
						glm::vec3 dir_1 = oct_to_vec3(v2_1);
						float weight = powf(glm::dot(dir_0, dir_1), power);
						sum_dis += distance[(vis_res - x) + y * vis_res] * weight;
						sum_sqr_dis += sqr_dis[(vis_res - x) + y * vis_res] * weight;
						sum_weight += weight;
					}
				}
				else
				{
					{
						glm::vec2 v2_1 = glm::vec2(float(x) + 0.5f, float(vis_res - 1 - y) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
						glm::vec3 dir_1 = oct_to_vec3(v2_1);
						float weight = powf(glm::dot(dir_0, dir_1), power);
						sum_dis += distance[x + (vis_res - 1 - y) * vis_res] * weight;
						sum_sqr_dis += sqr_dis[x + (vis_res - 1 - y) * vis_res] * weight;
						sum_weight += weight;
					}

					if (y > 0)
					{
						glm::vec2 v2_1 = glm::vec2(float(x) + 0.5f, float(vis_res - y) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
						glm::vec3 dir_1 = oct_to_vec3(v2_1);
						float weight = powf(glm::dot(dir_0, dir_1), power);
						sum_dis += distance[x + (vis_res - y) * vis_res] * weight;
						sum_sqr_dis += sqr_dis[x + (vis_res - y) * vis_res] * weight;
						sum_weight += weight;
					}
					else
					{
						glm::vec2 v2_1 = glm::vec2(float(vis_res - 1) + 0.5f, float(vis_res - 1) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
						glm::vec3 dir_1 = oct_to_vec3(v2_1);
						float weight = powf(glm::dot(dir_0, dir_1), power);
						sum_dis += distance[(vis_res - 1) + (vis_res - 1) * vis_res] * weight;
						sum_sqr_dis += sqr_dis[(vis_res - 1) + (vis_res - 1) * vis_res] * weight;
						sum_weight += weight;
					}

					if (y < vis_res - 1)
					{
						glm::vec2 v2_1 = glm::vec2(float(x) + 0.5f, float(vis_res - 1 - (y + 1)) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
						glm::vec3 dir_1 = oct_to_vec3(v2_1);
						float weight = powf(glm::dot(dir_0, dir_1), power);
						sum_dis += distance[x + (vis_res - 1 - (y + 1)) * vis_res] * weight;
						sum_sqr_dis += sqr_dis[x + (vis_res - 1 - (y + 1)) * vis_res] * weight;
						sum_weight += weight;
					}
					else
					{
						glm::vec2 v2_1 = glm::vec2(float(vis_res - 1) + 0.5f, 0.5f) / float(vis_res) * 2.0f - 1.0f;
						glm::vec3 dir_1 = oct_to_vec3(v2_1);
						float weight = powf(glm::dot(dir_0, dir_1), power);
						sum_dis += distance[vis_res - 1] * weight;
						sum_sqr_dis += sqr_dis[vis_res - 1] * weight;
						sum_weight += weight;
					}
				}

				if (x < vis_res - 1)
				{
					{
						glm::vec2 v2_1 = glm::vec2(float(x + 1) + 0.5f, float(y) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
						glm::vec3 dir_1 = oct_to_vec3(v2_1);
						float weight = powf(glm::dot(dir_0, dir_1), power);
						sum_dis += distance[(x + 1) + y * vis_res] * weight;
						sum_sqr_dis += sqr_dis[(x + 1) + y * vis_res] * weight;
						sum_weight += weight;
					}

					if (y > 0)
					{
						glm::vec2 v2_1 = glm::vec2(float(x + 1) + 0.5f, float(y - 1) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
						glm::vec3 dir_1 = oct_to_vec3(v2_1);
						float weight = powf(glm::dot(dir_0, dir_1), power);
						sum_dis += distance[(x + 1) + (y - 1) * vis_res] * weight;
						sum_sqr_dis += sqr_dis[(x + 1) + (y - 1) * vis_res] * weight;
						sum_weight += weight;
					}
					else
					{
						glm::vec2 v2_1 = glm::vec2(float(vis_res - 1 - (x + 1)) + 0.5f, float(y) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
						glm::vec3 dir_1 = oct_to_vec3(v2_1);
						float weight = powf(glm::dot(dir_0, dir_1), power);
						sum_dis += distance[(vis_res - 1 - (x + 1)) + y * vis_res] * weight;
						sum_sqr_dis += sqr_dis[(vis_res - 1 - (x + 1)) + y * vis_res] * weight;
						sum_weight += weight;
					}

					if (y < vis_res - 1)
					{
						glm::vec2 v2_1 = glm::vec2(float(x + 1) + 0.5f, float(y + 1) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
						glm::vec3 dir_1 = oct_to_vec3(v2_1);
						float weight = powf(glm::dot(dir_0, dir_1), power);
						sum_dis += distance[(x + 1) + (y + 1) * vis_res] * weight;
						sum_sqr_dis += sqr_dis[(x + 1) + (y + 1) * vis_res] * weight;
						sum_weight += weight;
					}
					else
					{
						glm::vec2 v2_1 = glm::vec2(float(vis_res - 1 - (x + 1)) + 0.5f, float(y) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
						glm::vec3 dir_1 = oct_to_vec3(v2_1);
						float weight = powf(glm::dot(dir_0, dir_1), power);
						sum_dis += distance[(vis_res - 1 - (x + 1)) + y * vis_res] * weight;
						sum_sqr_dis += sqr_dis[(vis_res - 1 - (x + 1)) + y * vis_res] * weight;
						sum_weight += weight;
					}
				}
				else
				{
					{
						glm::vec2 v2_1 = glm::vec2(float(x) + 0.5f, float(vis_res - 1 - y) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
						glm::vec3 dir_1 = oct_to_vec3(v2_1);
						float weight = powf(glm::dot(dir_0, dir_1), power);
						sum_dis += distance[x + (vis_res - 1 - y) * vis_res] * weight;
						sum_sqr_dis += sqr_dis[x + (vis_res - 1 - y) * vis_res] * weight;
						sum_weight += weight;
					}

					if (y > 0)
					{
						glm::vec2 v2_1 = glm::vec2(float(x) + 0.5f, float(vis_res - y) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
						glm::vec3 dir_1 = oct_to_vec3(v2_1);
						float weight = powf(glm::dot(dir_0, dir_1), power);
						sum_dis += distance[x + (vis_res - y) * vis_res] * weight;
						sum_sqr_dis += sqr_dis[x + (vis_res - y) * vis_res] * weight;
						sum_weight += weight;
					}
					else
					{
						glm::vec2 v2_1 = glm::vec2(0.5f, float(vis_res - 1) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
						glm::vec3 dir_1 = oct_to_vec3(v2_1);
						float weight = powf(glm::dot(dir_0, dir_1), power);
						sum_dis += distance[(vis_res - 1) * vis_res] * weight;
						sum_sqr_dis += sqr_dis[(vis_res - 1) * vis_res] * weight;
						sum_weight += weight;
					}

					if (y < vis_res - 1)
					{
						glm::vec2 v2_1 = glm::vec2(float(x) + 0.5f, float(vis_res - 1 - (y + 1)) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
						glm::vec3 dir_1 = oct_to_vec3(v2_1);
						float weight = powf(glm::dot(dir_0, dir_1), power);
						sum_dis += distance[x + (vis_res - 1 - (y + 1)) * vis_res] * weight;
						sum_sqr_dis += sqr_dis[x + (vis_res - 1 - (y + 1)) * vis_res] * weight;
						sum_weight += weight;
					}
					else
					{
						glm::vec2 v2_1 = glm::vec2(0.5f, 0.5f) / float(vis_res) * 2.0f - 1.0f;
						glm::vec3 dir_1 = oct_to_vec3(v2_1);
						float weight = powf(glm::dot(dir_0, dir_1), power);
						sum_dis += distance[0] * weight;
						sum_sqr_dis += sqr_dis[0] * weight;
						sum_weight += weight;
					}
				}

				if (y > 0)
				{
					glm::vec2 v2_1 = glm::vec2(float(x) + 0.5f, float(y - 1) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
					glm::vec3 dir_1 = oct_to_vec3(v2_1);
					float weight = powf(glm::dot(dir_0, dir_1), power);
					sum_dis += distance[x + (y - 1) * vis_res] * weight;
					sum_sqr_dis += sqr_dis[x + (y - 1) * vis_res] * weight;
					sum_weight += weight;
				}
				else
				{
					glm::vec2 v2_1 = glm::vec2(float(vis_res - 1 - x) + 0.5f, float(y) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
					glm::vec3 dir_1 = oct_to_vec3(v2_1);
					float weight = powf(glm::dot(dir_0, dir_1), power);
					sum_dis += distance[(vis_res - 1 - x) + y * vis_res] * weight;
					sum_sqr_dis += sqr_dis[(vis_res - 1 - x) + y * vis_res] * weight;
					sum_weight += weight;
				}

				if (y < vis_res - 1)
				{
					glm::vec2 v2_1 = glm::vec2(float(x) + 0.5f, float(y + 1) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
					glm::vec3 dir_1 = oct_to_vec3(v2_1);
					float weight = powf(glm::dot(dir_0, dir_1), power);
					sum_dis += distance[x + (y + 1) * vis_res] * weight;
					sum_sqr_dis += sqr_dis[x + (y + 1) * vis_res] * weight;
					sum_weight += weight;
				}
				else
				{
					glm::vec2 v2_1 = glm::vec2(float(vis_res - 1 - x) + 0.5f, float(y) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
					glm::vec3 dir_1 = oct_to_vec3(v2_1);
					float weight = powf(glm::dot(dir_0, dir_1), power);
					sum_dis += distance[(vis_res - 1 - x) + y * vis_res] * weight;
					sum_sqr_dis += sqr_dis[(vis_res - 1 - x) + y * vis_res] * weight;
					sum_weight += weight;
				}

				filtered_distance[x + y * vis_res] = sum_dis / sum_weight;
				filtered_sqr_dis[x + y * vis_res] = sum_sqr_dis / sum_weight;
			}
		}

		for (int y = 0; y < vis_res; y++)
		{
			for (int x = 0; x < vis_res; x++)
			{
				glm::vec2 v2 = glm::vec2(float(x) + 0.5f, float(y) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
				glm::vec3 dir = oct_to_vec3(v2);

				glm::vec3 dir_abs = glm::abs(dir) / spacing;

				int major_dir = 0;
				if (dir_abs.y > dir_abs.x)
				{
					if (dir_abs.z > dir_abs.y)
					{
						major_dir = 2;
					}
					else
					{
						major_dir = 1;
					}
				}
				else
				{
					if (dir_abs.z > dir_abs.x)
					{
						major_dir = 2;
					}
				}

				if (major_dir == 0)
				{
					dir_abs *= spacing.x * scale / dir_abs.x;
				}
				else if (major_dir == 1)
				{
					dir_abs *= spacing.y * scale / dir_abs.y;
				}
				else if (major_dir == 2)
				{
					dir_abs *= spacing.z * scale / dir_abs.z;
				}

				float max_dis = glm::length(dir_abs);

				float mean_dis = filtered_distance[x + y * vis_res];
				float mean_sqr_dis = filtered_sqr_dis[x + y * vis_res];
				float mean_var = sqrtf(mean_sqr_dis - mean_dis * mean_dis);

				f_mean_dis[x + y * vis_res + index * vis_res * vis_res] = glm::clamp(mean_dis / max_dis, 0.0f, 1.0f);
				f_mean_var[x + y * vis_res + index * vis_res * vis_res] = glm::clamp(mean_var / max_dis, 0.0f, 1.0f);
			}
		}
	}
	
	//printf("%d\n", pack_res);

	{
		m_visibility_data.resize(pack_res * pack_res * 2, 0);
		for (int index = 0; index < num_probes; index++)
		{
			for (int y = 0; y < vis_res; y++)
			{
				for (int x = 0; x < vis_res; x++)
				{
					float dis = f_mean_dis[x + y * vis_res + index * vis_res * vis_res];
					unsigned short udis = (unsigned short)(dis * 65535.0f);
					float mean_var = f_mean_var[x + y * vis_res + index * vis_res * vis_res];
					unsigned short uvar = (unsigned short)std::max(mean_var * 65535.0f, 1.0f);
					int out_x = (index % pack_size) * (vis_res + 2) + x + 1;
					int out_y = (index / pack_size) * (vis_res + 2) + y + 1;
					m_visibility_data[(out_x + out_y * pack_res) * 2] = udis;
					m_visibility_data[(out_x + out_y * pack_res) * 2 + 1] = uvar;

				}
			}
			{
				float dis = f_mean_dis[(vis_res - 1) + (vis_res - 1) * vis_res + index * vis_res * vis_res];
				unsigned short udis = (unsigned short)(dis * 65535.0f);
				float mean_var = f_mean_var[(vis_res - 1) + (vis_res - 1) * vis_res + index * vis_res * vis_res];
				unsigned short uvar = (unsigned short)std::max(mean_var * 65535.0f, 1.0f);
				int out_x = (index % pack_size) * (vis_res + 2);
				int out_y = (index / pack_size) * (vis_res + 2);
				m_visibility_data[(out_x + out_y * pack_res) * 2] = udis;
				m_visibility_data[(out_x + out_y * pack_res) * 2 + 1] = uvar;
			}
			{
				float dis = f_mean_dis[(vis_res - 1) * vis_res + index * vis_res * vis_res];
				unsigned short udis = (unsigned short)(dis * 65535.0f);
				float mean_var = f_mean_var[(vis_res - 1) * vis_res + index * vis_res * vis_res];
				unsigned short uvar = (unsigned short)std::max(mean_var * 65535.0f, 1.0f);
				int out_x = (index % pack_size) * (vis_res + 2) + vis_res + 1;
				int out_y = (index / pack_size) * (vis_res + 2);
				m_visibility_data[(out_x + out_y * pack_res) * 2] = udis;
				m_visibility_data[(out_x + out_y * pack_res) * 2 + 1] = uvar;
			}
			{
				float dis = f_mean_dis[(vis_res - 1) + index * vis_res * vis_res];
				unsigned short udis = (unsigned short)(dis * 65535.0f);
				float mean_var = f_mean_var[(vis_res - 1) + index * vis_res * vis_res];
				unsigned short uvar = (unsigned short)std::max(mean_var * 65535.0f, 1.0f);
				int out_x = (index % pack_size) * (vis_res + 2);
				int out_y = (index / pack_size) * (vis_res + 2) + vis_res + 1;
				m_visibility_data[(out_x + out_y * pack_res) * 2] = udis;
				m_visibility_data[(out_x + out_y * pack_res) * 2 + 1] = uvar;
			}

			{
				float dis = f_mean_dis[index * vis_res * vis_res];
				unsigned short udis = (unsigned short)(dis * 65535.0f);
				float mean_var = f_mean_var[index * vis_res * vis_res];
				unsigned short uvar = (unsigned short)std::max(mean_var * 65535.0f, 1.0f);
				int out_x = (index % pack_size) * (vis_res + 2) + vis_res + 1;
				int out_y = (index / pack_size) * (vis_res + 2) + vis_res + 1;
				m_visibility_data[(out_x + out_y * pack_res) * 2] = udis;
				m_visibility_data[(out_x + out_y * pack_res) * 2 + 1] = uvar;
			}

			for (int x = 0; x < vis_res; x++)
			{
				{
					float dis = f_mean_dis[(vis_res - 1 - x) + index * vis_res * vis_res];
					unsigned short udis = (unsigned short)(dis * 65535.0f);
					float mean_var = f_mean_var[(vis_res - 1 - x) + index * vis_res * vis_res];
					unsigned short uvar = (unsigned short)std::max(mean_var * 65535.0f, 1.0f);
					int out_x = (index % pack_size) * (vis_res + 2) + x + 1;
					int out_y = (index / pack_size) * (vis_res + 2);
					m_visibility_data[(out_x + out_y * pack_res) * 2] = udis;
					m_visibility_data[(out_x + out_y * pack_res) * 2 + 1] = uvar;
				}
				{
					float dis = f_mean_dis[(vis_res - 1 - x) + (vis_res - 1) * vis_res + index * vis_res * vis_res];
					unsigned short udis = (unsigned short)(dis * 65535.0f);
					float mean_var = f_mean_var[(vis_res - 1 - x) + (vis_res - 1) * vis_res + index * vis_res * vis_res];
					unsigned short uvar = (unsigned short)std::max(mean_var * 65535.0f, 1.0f);
					int out_x = (index % pack_size) * (vis_res + 2) + x + 1;
					int out_y = (index / pack_size) * (vis_res + 2) + vis_res + 1;
					m_visibility_data[(out_x + out_y * pack_res) * 2] = udis;
					m_visibility_data[(out_x + out_y * pack_res) * 2 + 1] = uvar;
				}
			}
			for (int y = 0; y < vis_res; y++)
			{
				{
					float dis = f_mean_dis[(vis_res - 1 - y) * vis_res + index * vis_res * vis_res];
					unsigned short udis = (unsigned short)(dis * 65535.0f);
					float mean_var = f_mean_var[(vis_res - 1 - y) * vis_res + index * vis_res * vis_res];
					unsigned short uvar = (unsigned short)std::max(mean_var * 65535.0f, 1.0f);
					int out_x = (index % pack_size) * (vis_res + 2);
					int out_y = (index / pack_size) * (vis_res + 2) + y + 1;
					m_visibility_data[(out_x + out_y * pack_res) * 2] = udis;
					m_visibility_data[(out_x + out_y * pack_res) * 2 + 1] = uvar;
				}

				{
					float dis = f_mean_dis[(vis_res - 1) + (vis_res - 1 - y) * vis_res + index * vis_res * vis_res];
					unsigned short udis = (unsigned short)(dis * 65535.0f);
					float mean_var = f_mean_var[(vis_res - 1) + (vis_res - 1 - y) * vis_res + index * vis_res * vis_res];
					unsigned short uvar = (unsigned short)std::max(mean_var * 65535.0f, 1.0f);
					int out_x = (index % pack_size) * (vis_res + 2) + vis_res + 1;
					int out_y = (index / pack_size) * (vis_res + 2) + y + 1;
					m_visibility_data[(out_x + out_y * pack_res) * 2] = udis;
					m_visibility_data[(out_x + out_y * pack_res) * 2 + 1] = uvar;
				}
			}
		}

		/*FILE* fp = fopen("vis16_dmp.raw", "wb");
		fwrite(m_visibility_data.data(), 2, m_visibility_data.size(), fp);
		fclose(fp);*/
	}

	glBindTexture(GL_TEXTURE_2D, m_tex_visibility->tex_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16, pack_res, pack_res, 0, GL_RG, GL_UNSIGNED_SHORT, m_visibility_data.data());
	glBindTexture(GL_TEXTURE_2D, 0);
}

