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
		glm::vec3 spacing = (coverage_max - coverage_min) / glm::vec3(divisions);
		float max_visibility = glm::length(spacing);
		size_t size = sizeof(float) * 26 * num;
		m_visibility_buf = std::unique_ptr<GLBuffer>(new GLBuffer(size, GL_SHADER_STORAGE_BUFFER));
		m_visibility_data.resize(26 * num, max_visibility);
		m_visibility_buf->upload(m_visibility_data.data());
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
	c.diffuseThresh = diffuse_thresh;
	c.diffuseHigh = diffuse_high;
	c.diffuseLow = diffuse_low;
	c.specularThresh = specular_thresh;
	c.specularHigh = specular_high;
	c.specularLow = specular_low;
	m_constant.upload(&c);
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

	glm::vec3 size_grid = coverage_max - coverage_min;
	glm::vec3 spacing = size_grid / glm::vec3(divisions);
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

				for (int i = 0; i < 26; i++)
				{
					glm::vec3 dir = directions[i] * spacing;
					float vis = glm::length(dir);
					dir = glm::normalize(dir);
					bvh_ray.direction = bvh::Vector3<float>(dir.x, dir.y, dir.z);
					auto intersection = bvh.intersect(bvh_ray, 2);
					if (intersection.has_value())
					{
						float dis = intersection->distance();
						if (dis < vis) vis = dis;
					}
					m_visibility_data[index * 26 + i] = vis;

				}
			}
		}
	}

	m_visibility_buf->upload(m_visibility_data.data());
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