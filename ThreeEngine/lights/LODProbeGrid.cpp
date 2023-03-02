#include <GL/glew.h>
#include <queue>
#include "LODProbeGrid.h"
#include "renderers/GLRenderer.h"

#include "scenes/Scene.h"
#include "models/SimpleModel.h"
#include "models/GLTFModel.h"
#include "core/BoundingVolumeHierarchy.h"


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

void LODProbeGrid::updateBuffers()
{
	m_sub_index_buf = std::unique_ptr<GLBuffer>(new GLBuffer(sizeof(int) * m_sub_index.size(), GL_SHADER_STORAGE_BUFFER));
	m_sub_index_buf->upload(m_sub_index.data());

	m_probe_buf = std::unique_ptr<GLBuffer>(new GLBuffer(sizeof(glm::vec4) * m_probe_data.size(), GL_SHADER_STORAGE_BUFFER));
	m_probe_buf->upload(m_probe_data.data());

	int num_probes = getNumberOfProbes();
	if (m_visibility_data.size()< num_probes)
	{
		glm::vec3 spacing = (coverage_max - coverage_min) / glm::vec3(base_divisions);
		float max_visibility = glm::length(spacing);		
		m_visibility_data.resize(26 * num_probes, max_visibility);		
	}
	m_visibility_buf = std::unique_ptr<GLBuffer>(new GLBuffer(sizeof(float) * m_visibility_data.size(), GL_SHADER_STORAGE_BUFFER));
	m_visibility_buf->upload(m_visibility_data.data());
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

	glm::vec3 size_grid = coverage_max - coverage_min;
	glm::vec3 spacing = size_grid / glm::vec3(base_divisions);
	float max_visibility = glm::length(spacing);

	const glm::vec3 directions[26] = {
		glm::vec3(-1.0f,0.0f,0.0f),
		glm::vec3(1.0f,0.0f,0.0f),
		glm::vec3(0.0f,-1.0f,0.0f),
		glm::vec3(0.0f,1.0f,0.0f),
		glm::vec3(0.0f,0.0f,-1.0f),
		glm::vec3(0.0f,0.0f,1.0f),
		glm::vec3(0.0f,-1.0f,-1.0f),
		glm::vec3(0.0f,1.0f,-1.0f),
		glm::vec3(0.0f,-1.0f,1.0f),
		glm::vec3(0.0f,1.0f,1.0f),
		glm::vec3(-1.0f,0.0f,-1.0f),
		glm::vec3(-1.0f,0.0f,1.0f),
		glm::vec3(1.0f,0.0f,-1.0f),
		glm::vec3(1.0f,0.0f,1.0f),
		glm::vec3(-1.0f,-1.0f,0.0f),
		glm::vec3(1.0f,-1.0f,0.0f),
		glm::vec3(-1.0f,1.0f,0.0f),
		glm::vec3(1.0f,1.0f,0.0f),
		glm::vec3(-1.0f,-1.0f,-1.0f),
		glm::vec3(1.0f,-1.0f,-1.0f),
		glm::vec3(-1.0f,1.0f,-1.0f),
		glm::vec3(1.0f,1.0f,-1.0f),
		glm::vec3(-1.0f,-1.0f,1.0f),
		glm::vec3(1.0f,-1.0f,1.0f),
		glm::vec3(-1.0f,1.0f,1.0f),
		glm::vec3(1.0f,1.0f,1.0f)
	};

	BoundingVolumeHierarchy bvh(objects);

	int num_probes = getNumberOfProbes();
	m_visibility_data.resize(26 * num_probes, max_visibility);
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
		float scale = 1.0f/ float(1 << lod);

		bool zero = false;
		for (int i = 0; i < 26; i++)
		{
			glm::vec3 dir = directions[i] * spacing * scale;
			float vis = glm::length(dir);
			dir = glm::normalize(dir);
			bvh_ray.direction = bvh::Vector3<float>(dir.x, dir.y, dir.z);
			auto intersectionA = bvh.intersect(bvh_ray, 2);
			if (intersectionA.has_value())
			{
				float disA = intersectionA->distance();
				auto intersectionB = bvh.intersect(bvh_ray,1);
				if (intersectionB.has_value())
				{
					float disB = intersectionB->distance();
					if (disB >= disA)
					{
						zero = true;
						break;
					}
					float dis = (disA + disB) * 0.5f;
					if (dis < vis) vis = dis;
				}
				else
				{
					zero = true;
					break;
				}				
			}
			m_visibility_data[index * 26 + i] = vis;
		}
		if (zero)
		{
			for (int i = 0; i < 26; i++)
			{
				m_visibility_data[index * 26 + i] = 0.0f;
			}
		}
	}

	m_visibility_buf->upload(m_visibility_data.data());

}

