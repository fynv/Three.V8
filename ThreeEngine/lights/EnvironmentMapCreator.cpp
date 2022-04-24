#include <GL/glew.h>
#include <glm.hpp>
#include <cstdio>
#include <cmath>
#include <vector>
#include "EnvironmentMapCreator.h"

const double PI = 3.14159265359;

#if 1

#include <xmmintrin.h>
#include <mmintrin.h>

void SHEval3(const float* pX, const float* pY, const float* pZ, float* pSH)
{
	__m128 fX, fY, fZ;
	__m128 fC0, fC1, fS0, fS1, fTmpA, fTmpB, fTmpC;
	fX = _mm_load_ps(pX);
	fY = _mm_load_ps(pY);
	fZ = _mm_load_ps(pZ);

	__m128 fZ2 = _mm_mul_ps(fZ, fZ);

	_mm_store_ps(pSH + 0 * 4, _mm_set_ps1(0.2820947917738781f));
	_mm_store_ps(pSH + 2 * 4, _mm_mul_ps(_mm_set_ps1(0.4886025119029199f), fZ));
	_mm_store_ps(pSH + 6 * 4, _mm_add_ps(_mm_mul_ps(_mm_set_ps1(0.9461746957575601f), fZ2), _mm_set_ps1(-0.3153915652525201f)));
	fC0 = fX;
	fS0 = fY;

	fTmpA = _mm_set_ps1(-0.48860251190292f);
	_mm_store_ps(pSH + 3 * 4, _mm_mul_ps(fTmpA, fC0));
	_mm_store_ps(pSH + 1 * 4, _mm_mul_ps(fTmpA, fS0));
	fTmpB = _mm_mul_ps(_mm_set_ps1(-1.092548430592079f), fZ);
	_mm_store_ps(pSH + 7 * 4, _mm_mul_ps(fTmpB, fC0));
	_mm_store_ps(pSH + 5 * 4, _mm_mul_ps(fTmpB, fS0));
	fC1 = _mm_sub_ps(_mm_mul_ps(fX, fC0), _mm_mul_ps(fY, fS0));
	fS1 = _mm_add_ps(_mm_mul_ps(fX, fS0), _mm_mul_ps(fY, fC0));

	fTmpC = _mm_set_ps1(0.5462742152960395f);
	_mm_store_ps(pSH + 8 * 4, _mm_mul_ps(fTmpC, fC1));
	_mm_store_ps(pSH + 4 * 4, _mm_mul_ps(fTmpC, fS1));
}

#else

inline void SHEval3(const float fX, const float fY, const float fZ, float* pSH)
{
	float fC0, fC1, fS0, fS1, fTmpA, fTmpB, fTmpC;
	float fZ2 = fZ * fZ;

	pSH[0] = 0.2820947917738781f;
	pSH[2] = 0.4886025119029199f * fZ;
	pSH[6] = 0.9461746957575601f * fZ2 + -0.3153915652525201f;
	fC0 = fX;
	fS0 = fY;

	fTmpA = -0.48860251190292f;
	pSH[3] = fTmpA * fC0;
	pSH[1] = fTmpA * fS0;
	fTmpB = -1.092548430592079f * fZ;
	pSH[7] = fTmpB * fC0;
	pSH[5] = fTmpB * fS0;
	fC1 = fX * fC0 - fY * fS0;
	fS1 = fX * fS0 + fY * fC0;

	fTmpC = 0.5462742152960395f;
	pSH[8] = fTmpC * fC1;
	pSH[4] = fTmpC * fS1;
}

#endif

EnvironmentMapCreator::EnvironmentMapCreator()
{
	glGenFramebuffers(2, m_down_bufs);

	glGenTextures(1, &m_tex_128);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex_128);
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA8, 128, 128);

}

EnvironmentMapCreator::~EnvironmentMapCreator()
{
	glDeleteTextures(1, &m_tex_128);
	glDeleteFramebuffers(2, m_down_bufs);
}

void EnvironmentMapCreator::Create(int width, int height, const GLCubemap * cubemap, EnvironmentMap * envMap)
{
	// w_h => 128_128
	for (int i = 0; i < 6; i++)
	{	
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_down_bufs[0]);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap->tex_id, 0);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_down_bufs[1]);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_tex_128, 0);

		glBlitFramebuffer(0, 0, width, height, 0, 0, 128, 128, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}
	
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex_128);

	std::vector<uint8_t> faces[6];
	for (int i = 0; i < 6; i++)
	{
		faces[i].resize(128 * 128 * 4);
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, GL_UNSIGNED_BYTE, faces[i].data());		
	}

	float pixelSize = 2.0f / 128.0f;
	float totalWeight = 0.0f;
	memset(envMap->shCoefficients, 0, sizeof(glm::vec4) * 9);

	for (int i = 0; i < 6; i++)
	{
		std::vector<uint8_t>& face = faces[i];

		for (int j = 0; j < 128 * 128; j+=4)
		{
			float dir_x[4];
			float dir_y[4];
			float dir_z[4];
			float weight[4];
			glm::vec3 color[4];

			for (int k = 0; k < 4; k++)
			{
				int pixelIndex = j + k;

				float col = -1.0f + ((float)(pixelIndex % 128) + 0.5f) * pixelSize;
				float row = -1.0f + ((float)(pixelIndex / 128) + 0.5f) * pixelSize;
				glm::vec3 coord;
				switch (i)
				{
				case 0:
					coord = { 1.0f, -row, -col };
					break;
				case 1:
					coord = { -1.0f, -row, col };
					break;
				case 2:
					coord = { col, 1, row };
					break;
				case 3:
					coord = { col, -1, -row };
					break;
				case 4:
					coord = { col, -row, 1 };
					break;
				case 5:
					coord = { -col, -row, -1 };
					break;
				}
				float lengthSq = glm::dot(coord, coord);
				weight[k] = 4.0f / (sqrtf(lengthSq) * lengthSq);
				totalWeight += weight[k];
				glm::vec3 dir = glm::normalize(coord);
				dir_x[k] = dir.x;
				dir_y[k] = dir.y;
				dir_z[k] = dir.z;

				const uint8_t* pixel = &face[(size_t)pixelIndex * 4];
				color[k] = { (float)pixel[0] / 255.0f, (float)pixel[1] / 255.0f, (float)pixel[2] / 255.0f };
			}
			
#if 1
			float shBasis[36];
			SHEval3(dir_x, dir_y, dir_z, shBasis);

			for (int k = 0; k < 36; k++)
			{
				int idx_col = k % 4;
				int idx_coeff = k / 4;
				envMap->shCoefficients[idx_coeff] += glm::vec4(shBasis[k] * weight[idx_col] * color[idx_col], 0.0f);
			}
#else
			for (int k = 0; k < 4; k++)
			{
				float shBasis[9];
				SHEval3(dir_x[k], dir_y[k], dir_z[k], shBasis);

				for (int l = 0; l < 9; l++)
				{
					envMap->shCoefficients[l] += glm::vec4(shBasis[l] * weight[k] * color[k], 0.0f);
				}
			}
#endif
		}
	}

	float norm = (4.0f * PI) / totalWeight;

	for (int k = 0; k < 9; k++)
	{
		envMap->shCoefficients[k] *= norm;
	}

	envMap->updateConstant();
}

void EnvironmentMapCreator::Create(const CubeImage* image, EnvironmentMap* envMap)
{
	GLCubemap cubemap;
	cubemap.load_memory_bgr(image->images[0].width(), image->images[0].height(),
		image->images[0].data(), image->images[1].data(), image->images[2].data(), image->images[3].data(), image->images[4].data(), image->images[5].data());
	Create(image->images[0].width(), image->images[0].height(), &cubemap, envMap);
}

void EnvironmentMapCreator::Create(const CubeBackground* background, EnvironmentMap* envMap)
{
	Create(background->width, background->height, &background->cubemap, envMap);
}
