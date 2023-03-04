#include <GL/glew.h>
#include "ProbeGrid.h"

#include "scenes/Scene.h"
#include "models/SimpleModel.h"
#include "models/GLTFModel.h"
#include "core/BoundingVolumeHierarchy.h"

struct ProbeGridConst
{
	glm::vec4 coverageMin;
	glm::vec4 coverageMax;
	glm::ivec4 divisions;
	float ypower;
	int visRes;
	int packSize;
	int packRes;
	float diffuseThresh;
	float diffuseHigh;
	float diffuseLow;
	float specularThresh;
	float specularHigh;
	float specularLow;
};

ProbeGrid::ProbeGrid() : m_constant(sizeof(ProbeGridConst), GL_UNIFORM_BUFFER)
{
	m_tex_visibility = std::unique_ptr<GLTexture2D>(new GLTexture2D);
}


ProbeGrid::~ProbeGrid()
{

}

void ProbeGrid::allocate_probes()
{	
	size_t num = divisions.x * divisions.y * divisions.z;
	
	{
		size_t size = sizeof(glm::vec4) * 9 * num;
		m_probe_buf = std::unique_ptr<GLBuffer>(new GLBuffer(size, GL_SHADER_STORAGE_BUFFER));
		m_probe_data.resize(9 * num, glm::vec4(0.0f));
		m_probe_buf->upload(m_probe_data.data());
	}
	{	
		pack_size = int(ceilf(sqrtf(float(num))));
		pack_res = pack_size * (vis_res + 2);
		m_visibility_data.resize(pack_res * pack_res, 0);

		glBindTexture(GL_TEXTURE_2D, m_tex_visibility->tex_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, pack_res, pack_res, 0, GL_RED, GL_UNSIGNED_SHORT, m_visibility_data.data());
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	m_ref_buf = nullptr;
}

void ProbeGrid::updateConstant()
{
	ProbeGridConst c;
	c.coverageMin = glm::vec4(coverage_min, 0.0f);
	c.coverageMax = glm::vec4(coverage_max, 0.0f);
	c.divisions = glm::ivec4(divisions, 0);
	c.ypower = ypower;
	c.visRes = vis_res;
	c.packSize = pack_size;
	c.packRes = pack_res;
	c.diffuseThresh = diffuse_thresh;
	c.diffuseHigh = diffuse_high;
	c.diffuseLow = diffuse_low;
	c.specularThresh = specular_thresh;
	c.specularHigh = specular_high;
	c.specularLow = specular_low;
	m_constant.upload(&c);
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

void ProbeGrid::construct_visibility(Scene& scene)
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
	glm::vec3 spacing = size_grid / glm::vec3(divisions);
	float max_visibility = glm::length(spacing);

	size_t num_probes = divisions.x * divisions.y * divisions.z;
	std::vector<float> f_visibility(vis_res * vis_res * num_probes, max_visibility);
	
	for (int z = 0; z < divisions.z; z++)
	{
		for (int y = 0; y < divisions.y; y++)
		{
			glm::vec3 spacing1 = spacing;
			if (y > 0)
			{
				float y0 = powf(((float)(y - 1) + 0.5f) / (float)divisions.y, ypower);
				float y1 = powf(((float)y + 0.5f) / (float)divisions.y, ypower);
				spacing1.y = (y1 - y0) * size_grid.y;
			}
			glm::vec3 spacing2 = spacing;
			if (y < divisions.y - 1)
			{				
				float y0 = powf(((float)y + 0.5f) / (float)divisions.y, ypower);
				float y1 = powf(((float)(y + 1) + 0.5f) / (float)divisions.y, ypower);
				spacing2.y = (y1 - y0) * size_grid.y;
			}
			for (int x = 0; x < divisions.x; x++)
			{
				int index = x + (y + (z * divisions.y)) * divisions.x;
				glm::ivec3 idx(x, y, z);				
				glm::vec3 pos_normalized = (glm::vec3(idx) + 0.5f) / glm::vec3(divisions);
				pos_normalized.y = powf(pos_normalized.y, ypower);
				glm::vec3 pos = coverage_min + pos_normalized * size_grid;							

				bvh::Ray<float> bvh_ray = {
					bvh::Vector3<float>(pos.x, pos.y, pos.z),
					bvh::Vector3<float>(0.0f, 0.0f, 0.0f),
					0.0f,
					max_visibility
				};

				for (int py = 0; py < vis_res; py++)
				{
					for (int px = 0; px < vis_res; px++)
					{
						glm::vec2 v2 = glm::vec2(float(px) + 0.5f, float(py) + 0.5f) / float(vis_res) * 2.0f - 1.0f;
						glm::vec3 dir = oct_to_vec3(v2);

						if (dir.y <= 0.0f) spacing = spacing1;
						else spacing = spacing2;
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
							dir_abs *= spacing.x / dir_abs.x;
						}
						else if (major_dir == 1)
						{
							dir_abs *= spacing.y / dir_abs.y;
						}
						else if (major_dir == 2)
						{
							dir_abs *= spacing.z / dir_abs.z;
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
						f_visibility[px + py * vis_res + index * vis_res * vis_res] = glm::clamp(dis / max_dis, 0.0f, 1.0f);
					}
				}
			}
		}
	}

	// printf("%d\n", pack_res);
	{
		m_visibility_data.resize(pack_res * pack_res, 0);
		for (int index = 0; index < num_probes; index++)
		{
			for (int y = 0; y < vis_res; y++)
			{
				for (int x = 0; x < vis_res; x++)
				{
					float dis = f_visibility[(x % vis_res) + (y % vis_res) * vis_res + index * vis_res * vis_res];
					unsigned short udis = (unsigned short)(dis * 65535.0f);
					int out_x = (index % pack_size) * (vis_res + 2) + x + 1;
					int out_y = (index / pack_size) * (vis_res + 2) + y + 1;
					m_visibility_data[out_x + out_y * pack_res] = udis;
				}
			}
			{
				float dis = f_visibility[(vis_res - 1) + (vis_res - 1) * vis_res + index * vis_res * vis_res];
				unsigned short udis = (unsigned short)(dis * 65535.0f);
				int out_x = (index % pack_size) * (vis_res + 2);
				int out_y = (index / pack_size) * (vis_res + 2);
				m_visibility_data[out_x + out_y * pack_res] = udis;
			}
			{
				float dis = f_visibility[(vis_res - 1) * vis_res + index * vis_res * vis_res];
				unsigned short udis = (unsigned short)(dis * 65535.0f);
				int out_x = (index % pack_size) * (vis_res + 2) + vis_res + 1;
				int out_y = (index / pack_size) * (vis_res + 2);
				m_visibility_data[out_x + out_y * pack_res] = udis;
			}
			{
				float dis = f_visibility[(vis_res - 1) + index * vis_res * vis_res];
				unsigned short udis = (unsigned short)(dis * 65535.0f);
				int out_x = (index % pack_size) * (vis_res + 2);
				int out_y = (index / pack_size) * (vis_res + 2) + vis_res + 1;
				m_visibility_data[out_x + out_y * pack_res] = udis;
			}

			{
				float dis = f_visibility[index * vis_res * vis_res];
				unsigned short udis = (unsigned short)(dis * 65535.0f);
				int out_x = (index % pack_size) * (vis_res + 2) + vis_res + 1;
				int out_y = (index / pack_size) * (vis_res + 2) + vis_res + 1;
				m_visibility_data[out_x + out_y * pack_res] = udis;
			}

			for (int x = 0; x < vis_res; x++)
			{
				{
					float dis = f_visibility[(vis_res - 1 - x) + index * vis_res * vis_res];
					unsigned short udis = (unsigned short)(dis * 65535.0f);
					int out_x = (index % pack_size) * (vis_res + 2) + x + 1;
					int out_y = (index / pack_size) * (vis_res + 2);
					m_visibility_data[out_x + out_y * pack_res] = udis;
				}
				{
					float dis = f_visibility[(vis_res - 1 - x) + (vis_res - 1) * vis_res + index * vis_res * vis_res];
					unsigned short udis = (unsigned short)(dis * 65535.0f);
					int out_x = (index % pack_size) * (vis_res + 2) + x + 1;
					int out_y = (index / pack_size) * (vis_res + 2) + vis_res + 1;
					m_visibility_data[out_x + out_y * pack_res] = udis;
				}
			}
			for (int y = 0; y < vis_res; y++)
			{
				{
					float dis = f_visibility[(vis_res - 1 - y) * vis_res + index * vis_res * vis_res];
					unsigned short udis = (unsigned short)(dis * 65535.0f);
					int out_x = (index % pack_size) * (vis_res + 2);
					int out_y = (index / pack_size) * (vis_res + 2) + y + 1;
					m_visibility_data[out_x + out_y * pack_res] = udis;
				}

				{
					float dis = f_visibility[(vis_res - 1) + (vis_res - 1 - y) * vis_res + index * vis_res * vis_res];
					unsigned short udis = (unsigned short)(dis * 65535.0f);
					int out_x = (index % pack_size) * (vis_res + 2) + vis_res + 1;
					int out_y = (index / pack_size) * (vis_res + 2) + y + 1;
					m_visibility_data[out_x + out_y * pack_res] = udis;
				}
			}
		}

		/*FILE* fp = fopen("vis16_dmp.raw", "wb");
		fwrite(m_visibility_data.data(), 2, m_visibility_data.size(), fp);
		fclose(fp);*/
		
	}

	glBindTexture(GL_TEXTURE_2D, m_tex_visibility->tex_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, pack_res, pack_res, 0, GL_RED, GL_UNSIGNED_SHORT, m_visibility_data.data());
	glBindTexture(GL_TEXTURE_2D, 0);
}

void ProbeGrid::set_record_references(bool record)
{
	if (record == record_references) return;
	record_references = record;
	if (record && m_ref_buf == nullptr)
	{
		size_t num = divisions.x * divisions.y * divisions.z;		
		m_ref_buf = std::unique_ptr<GLBuffer>(new GLBuffer(num*sizeof(unsigned), GL_SHADER_STORAGE_BUFFER));
	}
}

void ProbeGrid::get_references(std::vector<unsigned>& references)
{
	if (m_ref_buf == nullptr) return;
	size_t num = divisions.x * divisions.y * divisions.z;
	references.resize(num);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ref_buf->m_id);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, num * sizeof(unsigned), references.data());
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	std::vector<unsigned> zeros(num, 0);
	m_ref_buf->upload(zeros.data());

}