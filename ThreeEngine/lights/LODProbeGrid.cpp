#include <GL/glew.h>
#include <queue>
#include "LODProbeGrid.h"
#include "renderers/GLRenderer.h"

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


void LODProbeGrid::initialize(GLRenderer& renderer, Scene& scene)
{
	m_sub_index.resize(base_divisions.x * base_divisions.y * base_divisions.z, -1);
	m_probe_data.resize(base_divisions.x * base_divisions.y * base_divisions.z * 10, glm::vec4(0.0f));

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
		std::vector<Volume> volumes(sub_division_level);

		unsigned tex_id;
		glGenTextures(1, &tex_id);		

		glm::ivec3 vol_div = base_divisions * (1 << (sub_division_level - 1));

		{
			Volume& vol = volumes[sub_division_level - 1];

			glBindTexture(GL_TEXTURE_3D, tex_id);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexStorage3D(GL_TEXTURE_3D, 1, GL_R8, vol_div.x, vol_div.y, vol_div.z);
			glBindTexture(GL_TEXTURE_3D, 0);

			uint8_t zero = 0;
			glClearTexImage(tex_id, 0, GL_RED, GL_UNSIGNED_BYTE, &zero);

			renderer.sceneToVolume(scene, tex_id, coverage_min, coverage_max, vol_div);
			vol.resize(vol_div.x * vol_div.y * vol_div.z);

			glBindTexture(GL_TEXTURE_3D, tex_id);
			glGetTexImage(GL_TEXTURE_3D, 0, GL_RED, GL_UNSIGNED_BYTE, vol.data());
			glBindTexture(GL_TEXTURE_3D, 0);

			glDeleteTextures(1, &tex_id);
		}

		for (int i = sub_division_level - 2; i >= 0; i--)
		{
			Volume& vol1 = volumes[i + 1];
			Volume& vol0 = volumes[i];
			vol_div /= 2;
			vol0.resize(vol_div.x * vol_div.y * vol_div.z, 0);
			
			for (int z = 0; z < vol_div.z * 2; z++)
			{
				for (int y = 0; y < vol_div.y * 2; y++)
				{
					for (int x = 0; x < vol_div.x * 2; x++)
					{
						int idx_in = x + (y + z * vol_div.y * 2) * vol_div.x * 2;
						int idx_out = x / 2 + (y / 2 + z / 2 * vol_div.y) * vol_div.x;
						if (vol1[idx_in] > 0) vol0[idx_out] = 1;
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
	updateBuffers();
}