#include <GL/glew.h>
#include <gtc/random.hpp>
#include "ProbeGrid.h"

#include "scenes/Scene.h"
#include "models/SimpleModel.h"
#include "models/GLTFModel.h"
#include "core/BoundingVolumeHierarchy.h"

#include "EnvironmentMapCreator.h"

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



struct ProbeGridConst
{
	glm::vec4 coverageMin;
	glm::vec4 coverageMax;
	glm::ivec4 divisions;
	float ypower;
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

ProbeGrid::ProbeGrid() : m_constant(sizeof(ProbeGridConst), GL_UNIFORM_BUFFER)
{	
	
}


ProbeGrid::~ProbeGrid()
{

}


void ProbeGrid::_presample_irradiance()
{
	int num_probes = (int)(divisions.x * divisions.y * divisions.z);
	int pack_size = int(ceilf(sqrtf(float(num_probes))));
	irr_pack_res = pack_size * (irr_res + 2);

	std::vector<glm::vec3> irradiance_img(irr_pack_res * irr_pack_res);

	for (int index = 0; index < num_probes; index++)
	{
		std::vector<glm::vec3> tex_data(irr_res * irr_res);
		EnvironmentMapCreator::PresampleSH(m_probe_data.data() + index * 9, tex_data.data(), irr_res);
		for (int y = 0; y < irr_res; y++)
		{
			for (int x = 0; x < irr_res; x++)
			{
				glm::vec3 irr = tex_data[x + y * irr_res];
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
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, irr_pack_res, irr_pack_res, GL_RGB, GL_FLOAT, irradiance_img.data());
	glBindTexture(GL_TEXTURE_2D, 0);
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
	_presample_irradiance();
	{	
		pack_size = int(ceilf(sqrtf(float(num))));
		pack_res = pack_size * (vis_res + 2);
		m_visibility_data.resize(pack_res * pack_res * 2, 0);

		m_tex_visibility = std::unique_ptr<GLTexture2D>(new GLTexture2D);
		glBindTexture(GL_TEXTURE_2D, m_tex_visibility->tex_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG16, pack_res, pack_res);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pack_res, pack_res, GL_RG, GL_UNSIGNED_SHORT, m_visibility_data.data());
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	m_ref_buf = nullptr;
}

void ProbeGrid::presample_probe(int index)
{	
	std::vector<glm::vec3> irradiance_img((irr_res + 2) * (irr_res + 2));

	std::vector<glm::vec3> tex_data(irr_res * irr_res);
	EnvironmentMapCreator::PresampleSH(m_probe_data.data() + index * 9, tex_data.data(), irr_res);
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

void ProbeGrid::updateConstant()
{
	ProbeGridConst c;
	c.coverageMin = glm::vec4(coverage_min, 0.0f);
	c.coverageMax = glm::vec4(coverage_max, 0.0f);
	c.divisions = glm::ivec4(divisions, 0);
	c.ypower = ypower;
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
	int num_rays = 256;

	glm::mat4 rand_rot = rand_rotation();

	struct Sample
	{
		int ray_id;
		float weight;
	};

	std::vector<std::vector<Sample>> samples(vis_res * vis_res);

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

	std::vector<float> f_mean_dis(vis_res * vis_res * num_probes, max_visibility);
	std::vector<float> f_mean_var(vis_res * vis_res * num_probes, max_visibility);

	
	for (int gz = 0; gz < divisions.z; gz++)
	{
		for (int gy = 0; gy < divisions.y; gy++)
		{
			glm::vec3 spacing1 = spacing;
			if (gy > 0)
			{
				float y0 = powf(((float)(gy - 1) + 0.5f) / (float)divisions.y, ypower);
				float y1 = powf(((float)gy + 0.5f) / (float)divisions.y, ypower);
				spacing1.y = (y1 - y0) * size_grid.y;
			}
			glm::vec3 spacing2 = spacing;
			if (gy < divisions.y - 1)
			{				
				float y0 = powf(((float)gy + 0.5f) / (float)divisions.y, ypower);
				float y1 = powf(((float)(gy + 1) + 0.5f) / (float)divisions.y, ypower);
				spacing2.y = (y1 - y0) * size_grid.y;
			}
			for (int gx = 0; gx < divisions.x; gx++)
			{
				int index = gx + (gy + (gz * divisions.y)) * divisions.x;
				glm::ivec3 idx(gx, gy, gz);				
				glm::vec3 pos_normalized = (glm::vec3(idx) + 0.5f) / glm::vec3(divisions);
				pos_normalized.y = powf(pos_normalized.y, ypower);
				glm::vec3 pos = coverage_min + pos_normalized * size_grid;							

				bvh::Ray<float> bvh_ray = {
					bvh::Vector3<float>(pos.x, pos.y, pos.z),
					bvh::Vector3<float>(0.0f, 0.0f, 0.0f),
					0.0f,
					max_visibility
				};

				std::vector<float> distance(num_rays);

				for (int ray_id = 0; ray_id < num_rays; ray_id++)
				{
					glm::vec3 sf = sphericalFibonacci(ray_id, num_rays);
					glm::vec3 dir = glm::vec3(rand_rot * glm::vec4(sf, 0.0f));

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
					distance[ray_id] = dis;
				}

				// filtering				
				std::vector<float> filtered_distance(vis_res * vis_res);
				std::vector<float> filtered_sqr_dis(vis_res * vis_res);
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
							sum_dis += weight * dis;
							sum_sqr_dis += weight * dis * dis;
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
						
						float mean_dis = filtered_distance[x + y * vis_res];
						float mean_sqr_dis = filtered_sqr_dis[x + y * vis_res];
						float mean_var = sqrtf(mean_sqr_dis - mean_dis * mean_dis);

						f_mean_dis[x + y * vis_res + index * vis_res * vis_res] = glm::clamp(mean_dis / max_dis, 0.0f, 1.0f);
						f_mean_var[x + y * vis_res + index * vis_res * vis_res] = glm::clamp(mean_var / max_dis, 0.0f, 1.0f);

					}
				}
			}
		}
	}

	// printf("%d\n", pack_res);
	{
		m_visibility_data.resize(pack_res* pack_res * 2, 0);
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
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pack_res, pack_res, GL_RG, GL_UNSIGNED_SHORT, m_visibility_data.data());
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