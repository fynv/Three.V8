#include <glm.hpp>
#include <memory.h>
#include <cmath>
#include <emscripten.h>
#include "half.hpp"

extern "C"
{
	EMSCRIPTEN_KEEPALIVE void* alloc(unsigned size);
	EMSCRIPTEN_KEEPALIVE void dealloc(void* ptr);
	EMSCRIPTEN_KEEPALIVE void zero(void* ptr, unsigned size);
    EMSCRIPTEN_KEEPALIVE void CreateSH(void* p_coeffs, const void* p_fp16);
}

void* alloc(unsigned size)
{
	return malloc(size);
}

void dealloc(void* ptr)
{
	free(ptr);
}

void zero(void* ptr, unsigned size)
{
	memset(ptr, 0, size);
}

const double PI = 3.14159265359;

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

void CreateSH(void* p_coeffs, const void* p_fp16)
{
    glm::vec4* shCoefficients = (glm::vec4*)p_coeffs;
    const half_float::half* fp16 = (const half_float::half*)p_fp16;

    float pixelSize = 2.0f / 128.0f;
	float totalWeight = 0.0f;
    memset(shCoefficients, 0, sizeof(glm::vec4) * 9);

    for (int i = 0; i < 6; i++)
    {
        const half_float::half* face = fp16 + 128*128*4*i;
        for (int j = 0; j < 128*128; j++)
        {
            float col = -1.0f + ((float)(j % 128) + 0.5f) * pixelSize;
			float row = -1.0f + ((float)(j / 128) + 0.5f) * pixelSize;
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
            glm::vec3 dir = glm::normalize(coord);
            float lengthSq = glm::dot(coord, coord);
            float weight = 4.0f / (sqrtf(lengthSq) * lengthSq);
            const half_float::half* pixel = face + j * 4;
            glm::vec3 color = { (float)pixel[0], (float)pixel[1], (float)pixel[2] };
            glm::bvec3 nan = glm::isnan(color);
			glm::bvec3 inf = glm::isinf(color);
            if (nan.x || nan.y || nan.z || inf.x || inf.y || inf.z)
            {
                color = {0.0f, 0.0f, 0.0f};
                weight = 0.0;
            }
            totalWeight += weight;
            float shBasis[9];
			SHEval3(dir.x, dir.y, dir.z, shBasis);
            for (int l = 0; l < 9; l++)
            {
                shCoefficients[l] += glm::vec4(shBasis[l] * weight * color, 0.0f);
            }
        }
    }
    float norm = (4.0f * PI) / totalWeight;
	float order[9] = { 0.0f, 1.0f, 1.0f, 1.0f, 2.0f, 2.0f, 2.0f,2.0f, 2.0f};
    for (int k = 0; k < 9; k++)
	{
		float x = order[k] / 3.0f;
		float filtering = expf(-x * x); // gauss
		shCoefficients[k] *= norm * filtering;
	}


}
