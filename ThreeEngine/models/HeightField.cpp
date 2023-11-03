#include <GL/glew.h>
#include "HeightField.h"

struct HeightFieldConst
{
	glm::vec4 min_pos;
	glm::vec4 max_pos;
};

HeightField::HeightField(const glm::vec3& pos_min, const glm::vec3& pos_max, int width, int height)
	: m_pos_min(pos_min)
	, m_pos_max(pos_max)
	, m_width(width)
	, m_height(height)
	, m_constant(sizeof(HeightFieldConst), GL_UNIFORM_BUFFER)
{
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	glBindTexture(GL_TEXTURE_2D, m_tex_depth.tex_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_tex_depth.tex_id, 0);

	HeightFieldConst c;
	c.min_pos = glm::vec4(m_pos_min, 1.0f);
	c.max_pos = glm::vec4(m_pos_max, 1.0f);
	m_constant.upload(&c);

	m_cpu_depth.resize(width * height);
}

HeightField::~HeightField()
{
	glDeleteFramebuffers(1, &m_fbo);
}

void HeightField::toCPU()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glReadPixels(0, 0, m_width, m_height, GL_DEPTH_COMPONENT, GL_FLOAT, m_cpu_depth.data());

	/*std::vector<unsigned char> buf(m_width * m_height);
	for (int i = 0; i < m_width * m_height; i++)
	{
		buf[i] = (unsigned char)(m_cpu_depth[i] * 255.0f + 0.5f);
	}
	FILE* fp = fopen("dmp.raw", "wb");
	fwrite(buf.data(), 1, m_width * m_height, fp);
	fclose(fp);*/

}

float HeightField::GetHeight(float x, float z)
{
	float fx = glm::clamp((x - m_pos_min.x) / (m_pos_max.x - m_pos_min.x) * (float)m_width - 0.5f, 0.0f, (float)m_width - 1.0f);
	float fz = glm::clamp((z - m_pos_min.z) / (m_pos_max.z - m_pos_min.z) * (float)m_height - 0.5f, 0.0f, (float)m_height - 1.0f);

	int ix = (int)fminf(floorf(fx), (float)m_width - 2.0f);
	int iz = (int)fminf(floorf(fz), (float)m_height - 2.0f);
	float frac_x = fx - (float)ix;
	float frac_z = fz - (float)iz;

	float h00 = m_cpu_depth[ix + iz * m_width];
	float h01 = m_cpu_depth[(ix + 1) + iz * m_width];
	float h10 = m_cpu_depth[ix + (iz+1) * m_width];
	float h11 = m_cpu_depth[(ix + 1) + (iz + 1) * m_width];

	float h = (h00 * (1.0f - frac_x) + h01 * frac_x) * (1.0f - frac_z)
		+ (h10 * (1.0f - frac_x) + h11 * frac_x) * frac_z;

	return h * (m_pos_max.y - m_pos_min.y) + m_pos_min.y;
}

#include "stb_image_write.h"

void HeightField::saveFile(const char* filename)
{
	std::vector<unsigned char> buf(m_width * m_height);
	for (int i = 0; i < m_width * m_height; i++)
	{
		buf[i] = (unsigned char)(m_cpu_depth[i] * 255.0f + 0.5f);
	}

	std::vector<unsigned char> jpg_buf;
	stbi_write_jpg_to_func([](void* context, void* data, int size)
	{
		std::vector<unsigned char>* buf = (std::vector<unsigned char>*)context;
		size_t offset = buf->size();
		buf->resize(offset + size);
		memcpy(buf->data() + offset, data, size);
	}, &jpg_buf, m_width, m_height, 1, buf.data(), 80);

	int bin_size = (int)jpg_buf.size();

	FILE* fp = fopen(filename, "wb");
	fwrite(&m_pos_min, sizeof(glm::vec3), 1, fp);
	fwrite(&m_pos_max, sizeof(glm::vec3), 1, fp);
	fwrite(&m_width, sizeof(int), 1, fp);
	fwrite(&m_height, sizeof(int), 1, fp);
	fwrite(&bin_size, sizeof(int), 1, fp);
	fwrite(jpg_buf.data(), 1, bin_size, fp);
	fclose(fp);
}
