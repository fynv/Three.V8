#include <GL/glew.h>
#include <glm.hpp>
#include <cstdio>
#include <cmath>
#include <vector>
#include <string>
#include "EnvironmentMapCreator.h"
#include "FilterCoeffs.hpp"

const double PI = 3.14159265359;

#if 1

#include <xmmintrin.h>
#include <mmintrin.h>

inline void SHEval3(const float* pX, const float* pY, const float* pZ, float* pSH)
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

	fTmpA = _mm_set_ps1(0.48860251190292f);
	_mm_store_ps(pSH + 3 * 4, _mm_mul_ps(fTmpA, fC0));
	_mm_store_ps(pSH + 1 * 4, _mm_mul_ps(fTmpA, fS0));
	fTmpB = _mm_mul_ps(_mm_set_ps1(1.092548430592079f), fZ);
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

#endif


/*glm::vec4 shGetIrradianceAt(const glm::vec3& normal, const glm::vec4 shCoefficients[9]) {

	// normal is assumed to have unit length

	float x = normal.x, y = normal.y, z = normal.z;

	// band 0
	glm::vec4 result = shCoefficients[0] * 0.886227f;

	// band 1
	result += shCoefficients[1] * 2.0f * 0.511664f * y;
	result += shCoefficients[2] * 2.0f * 0.511664f * z;
	result += shCoefficients[3] * 2.0f * 0.511664f * x;

	// band 2
	result += shCoefficients[4] * 2.0f * 0.429043f * x * y;
	result += shCoefficients[5] * 2.0f * 0.429043f * y * z;
	result += shCoefficients[6] * (0.743125f * z * z - 0.247708f);
	result += shCoefficients[7] * 2.0f * 0.429043f * x * z;
	result += shCoefficients[8] * 0.429043f * (x * x - y * y);

	return result;
}*/

static std::string g_compute_downsample =
R"(#version 430
layout (location = 0) uniform samplerCube tex_hi_res;
layout (binding=0, rgba8) uniform imageCube tex_lo_res;

void get_dir_0( out vec3 dir, in float u, in float v )
{
    dir[0] = 1.0;
    dir[1] = v;
    dir[2] = -u;
}
void get_dir_1( out vec3 dir, in float u, in float v )
{
    dir[0] = -1.0;
    dir[1] = v;
    dir[2] = u;
}
void get_dir_2( out vec3 dir, in float u, in float v )
{
    dir[0] = u;
    dir[1] = 1.0;
    dir[2] = -v;
}
void get_dir_3( out vec3 dir, in float u, in float v )
{
    dir[0] = u;
    dir[1] = -1.0;
    dir[2] = v;
}
void get_dir_4( out vec3 dir, in float u, in float v )
{
    dir[0] = u;
    dir[1] = v;
    dir[2] = 1.0;
}
void get_dir_5( out vec3 dir, in float u, in float v )
{
    dir[0] = -u;
    dir[1] = v;
    dir[2] = -1.0;
}

float calcWeight( float u, float v )
{
    float val = u*u + v*v + 1.0;
    return val*sqrt( val );
}

layout (location = 1) uniform float lod;

layout(local_size_x = 8, local_size_y = 8) in;

void main()
{
	ivec3 id = ivec3(gl_GlobalInvocationID);
	int res_lo = imageSize(tex_lo_res).x;
	if (id.x < res_lo && id.y < res_lo)
	{
		float inv_res_lo = 1.0 / float(res_lo);
		float u0 = ( float(id.x) * 2.0 + 1.0 - 0.75 ) * inv_res_lo - 1.0;
		float u1 = ( float(id.x) * 2.0 + 1.0 + 0.75 ) * inv_res_lo - 1.0;

		float v0 = ( float(id.y) * 2.0 + 1.0 - 0.75 ) * -inv_res_lo + 1.0;
		float v1 = ( float(id.y) * 2.0 + 1.0 + 0.75 ) * -inv_res_lo + 1.0;

		vec4 weights;
		weights.x = calcWeight( u0, v0 );
		weights.y = calcWeight( u1, v0 );
		weights.z = calcWeight( u0, v1 );
		weights.w = calcWeight( u1, v1 );

		float wsum = 0.5 / ( weights.x + weights.y + weights.z + weights.w );
		weights = weights*wsum + 0.125;

		vec3 dir;
		vec4 color;

		switch ( id.z )
		{
		case 0:
			get_dir_0( dir, u0, v0 );
			color = textureLod(tex_hi_res, dir, lod) * weights.x;

			get_dir_0( dir, u1, v0 );
			color += textureLod(tex_hi_res, dir, lod) * weights.y;			

			get_dir_0( dir, u0, v1 );
			color += textureLod(tex_hi_res, dir, lod) * weights.z;

			get_dir_0( dir, u1, v1 );
			color += textureLod(tex_hi_res, dir, lod) * weights.w;
			break;

		case 1:
			get_dir_1( dir, u0, v0 );
			color = textureLod(tex_hi_res, dir, lod) * weights.x;

			get_dir_1( dir, u1, v0 );
			color += textureLod(tex_hi_res, dir, lod) * weights.y;			

			get_dir_1( dir, u0, v1 );
			color += textureLod(tex_hi_res, dir, lod) * weights.z;

			get_dir_1( dir, u1, v1 );
			color += textureLod(tex_hi_res, dir, lod) * weights.w;
			break;

		case 2:
			get_dir_2( dir, u0, v0 );
			color = textureLod(tex_hi_res, dir, lod) * weights.x;

			get_dir_2( dir, u1, v0 );
			color += textureLod(tex_hi_res, dir, lod) * weights.y;			

			get_dir_2( dir, u0, v1 );
			color += textureLod(tex_hi_res, dir, lod) * weights.z;

			get_dir_2( dir, u1, v1 );
			color += textureLod(tex_hi_res, dir, lod) * weights.w;
			break;

		case 3:
			get_dir_3( dir, u0, v0 );
			color = textureLod(tex_hi_res, dir, lod) * weights.x;

			get_dir_3( dir, u1, v0 );
			color += textureLod(tex_hi_res, dir, lod) * weights.y;			

			get_dir_3( dir, u0, v1 );
			color += textureLod(tex_hi_res, dir, lod) * weights.z;

			get_dir_3( dir, u1, v1 );
			color += textureLod(tex_hi_res, dir, lod) * weights.w;
			break;

		case 4:
			get_dir_4( dir, u0, v0 );
			color = textureLod(tex_hi_res, dir, lod) * weights.x;

			get_dir_4( dir, u1, v0 );
			color += textureLod(tex_hi_res, dir, lod) * weights.y;			

			get_dir_4( dir, u0, v1 );
			color += textureLod(tex_hi_res, dir, lod) * weights.z;

			get_dir_4( dir, u1, v1 );
			color += textureLod(tex_hi_res, dir, lod) * weights.w;
			break;

		case 5:
			get_dir_5( dir, u0, v0 );
			color = textureLod(tex_hi_res, dir, lod) * weights.x;

			get_dir_5( dir, u1, v0 );
			color += textureLod(tex_hi_res, dir, lod) * weights.y;			

			get_dir_5( dir, u0, v1 );
			color += textureLod(tex_hi_res, dir, lod) * weights.z;

			get_dir_5( dir, u1, v1 );
			color += textureLod(tex_hi_res, dir, lod) * weights.w;
			break;
		}

		imageStore(tex_lo_res, id, color);
	}
}
)";

static std::string g_compute_filter =
R"(#version 430
layout (location = 0) uniform samplerCube tex_in;
layout (binding=0, rgba8) uniform imageCube tex_out0;
layout (binding=1, rgba8) uniform imageCube tex_out1;
layout (binding=2, rgba8) uniform imageCube tex_out2;
layout (binding=3, rgba8) uniform imageCube tex_out3;
layout (binding=4, rgba8) uniform imageCube tex_out4;
layout (binding=5, rgba8) uniform imageCube tex_out5;
layout (binding=6, rgba8) uniform imageCube tex_out6;


layout (std140, binding = 0) uniform Coeffs
{
	vec4 coeffs[7][5][3][24];
};

#define NUM_TAPS 32
#define BASE_RESOLUTION 128

void get_dir( out vec3 dir, in vec2 uv, in int face )
{
    switch ( face )
    {
    case 0:
        dir[0] = 1.0;
        dir[1] = uv.y;
        dir[2] = -uv.x;
        break;
    case 1:
        dir[0] = -1.0;
        dir[1] = uv.y;
        dir[2] = uv.x;
        break;
    case 2:
        dir[0] = uv.x;
        dir[1] = 1.0;
        dir[2] = -uv.y;
        break;
    case 3:
        dir[0] = uv.x;
        dir[1] = -1.0;
        dir[2] = uv.y;
        break;
    case 4:
        dir[0] = uv.x;
        dir[1] = uv.y;
        dir[2] = 1.0;
        break;
    case 5:
        dir[0] = -uv.x;
        dir[1] = uv.y;
        dir[2] = -1.0;
        break;
    }
}

layout(local_size_x = 64) in;

void main()
{
	ivec3 id = ivec3(gl_GlobalInvocationID);
	int level = 0;
	if ( id.x < ( 128 * 128 ) )
	{
		level = 0;
	}
	else if ( id.x < ( 128 * 128 + 64 * 64 ) )
	{
		level = 1;
		id.x -= ( 128 * 128 );
	}
	else if ( id.x < ( 128 * 128 + 64 * 64 + 32 * 32 ) )
	{
		level = 2;
		id.x -= ( 128 * 128 + 64 * 64 );
	}
	else if ( id.x < ( 128 * 128 + 64 * 64 + 32 * 32 + 16 * 16 ) )
	{
		level = 3;
		id.x -= ( 128 * 128 + 64 * 64 + 32 * 32 );
	}
	else if ( id.x < ( 128 * 128 + 64 * 64 + 32 * 32 + 16 * 16 + 8 * 8 ) )
	{
		level = 4;
		id.x -= ( 128 * 128 + 64 * 64 + 32 * 32 + 16 * 16 );
	}
	else if ( id.x < ( 128 * 128 + 64 * 64 + 32 * 32 + 16 * 16 + 8 * 8 + 4 * 4 ) )
	{
		level = 5;
		id.x -= ( 128 * 128 + 64 * 64 + 32 * 32 + 16 * 16 + 8 * 8 );
	}
	else if ( id.x < ( 128 * 128 + 64 * 64 + 32 * 32 + 16 * 16 + 8 * 8 + 4 * 4 + 2 * 2 ) )
	{
		level = 6;
		id.x -= ( 128 * 128 + 64 * 64 + 32 * 32 + 16 * 16 + 8 * 8 + 4 * 4 );
	}
	else
	{
		return;
	}

	vec3 dir, adir, frameZ;
	{
		id.z = id.y;
		int res = BASE_RESOLUTION >> level;
		id.y = id.x / res;
		id.x -= id.y * res;

		vec2 uv;
		uv.x = ( float(id.x) * 2.0 + 1.0 ) / float(res) - 1.0;
		uv.y = -( float(id.y) * 2.0 + 1.0 ) / float(res) + 1.0;

		get_dir( dir, uv, id.z );
		frameZ = normalize( dir );

		adir = abs( dir );		
	}

	vec4 color = vec4(0);
	for ( int axis = 0; axis < 3; axis++ )
	{
		int otherAxis0 = 1 - ( axis & 1 ) - ( axis >> 1 );
		int otherAxis1 = 2 - ( axis >> 1 );

		float frameweight = ( max( adir[otherAxis0], adir[otherAxis1] ) - 0.75 ) / 0.25;
        if ( frameweight > 0.0 )
		{
			vec3 UpVector;
			switch ( axis )
			{
			case 0:
				UpVector = vec3( 1, 0, 0 );
				break;
			case 1:
				UpVector = vec3( 0, 1, 0 );
				break;
			default:
				UpVector = vec3( 0, 0, 1 );
				break;
			}

			vec3 frameX = normalize( cross( UpVector, frameZ ) );
			vec3 frameY = cross( frameZ, frameX );

			float Nx = dir[otherAxis0];
			float Ny = dir[otherAxis1];
			float Nz = adir[axis];

			float NmaxXY = max( abs( Ny ), abs( Nx ) );
			Nx /= NmaxXY;
			Ny /= NmaxXY;

			float theta;
			if ( Ny < Nx )
			{
				if ( Ny <= -0.999 )
					theta = Nx;
				else
					theta = Ny;
			}
			else
			{
				if ( Ny >= 0.999 )
					theta = -Nx;
				else
					theta = -Ny;
			}

			float phi;
			if ( Nz <= -0.999 )
			{
				phi = -NmaxXY;
			}
			else if ( Nz >= 0.999 )
			{
				phi = NmaxXY;
			}
			else
			{
				phi = Nz;
			}

			float theta2 = theta*theta;
			float phi2 = phi*phi;
			
			for ( int iSuperTap = 0; iSuperTap < NUM_TAPS / 4; iSuperTap++ )
			{
				int index = ( NUM_TAPS / 4 ) * axis + iSuperTap;
				vec4 coeffsDir0[3];
				vec4 coeffsDir1[3];
				vec4 coeffsDir2[3];
				vec4 coeffsLevel[3];
				vec4 coeffsWeight[3];
				
				for ( int iCoeff = 0; iCoeff < 3; iCoeff++ )
				{
					coeffsDir0[iCoeff] = coeffs[level][0][iCoeff][index];
					coeffsDir1[iCoeff] = coeffs[level][1][iCoeff][index];
					coeffsDir2[iCoeff] = coeffs[level][2][iCoeff][index];
					coeffsLevel[iCoeff] = coeffs[level][3][iCoeff][index];
					coeffsWeight[iCoeff] = coeffs[level][4][iCoeff][index];
				}

				for ( int iSubTap = 0; iSubTap < 4; iSubTap++ )
				{
					vec3 sample_dir
						= frameX * ( coeffsDir0[0][iSubTap] + coeffsDir0[1][iSubTap] * theta2 + coeffsDir0[2][iSubTap] * phi2 )
						+ frameY * ( coeffsDir1[0][iSubTap] + coeffsDir1[1][iSubTap] * theta2 + coeffsDir1[2][iSubTap] * phi2 )
						+ frameZ * ( coeffsDir2[0][iSubTap] + coeffsDir2[1][iSubTap] * theta2 + coeffsDir2[2][iSubTap] * phi2 );

					float sample_level = coeffsLevel[0][iSubTap] + coeffsLevel[1][iSubTap] * theta2 + coeffsLevel[2][iSubTap] * phi2;

					float sample_weight = coeffsWeight[0][iSubTap] + coeffsWeight[1][iSubTap] * theta2 + coeffsWeight[2][iSubTap] * phi2;
					sample_weight *= frameweight;

					// adjust for jacobian
					sample_dir /= max( abs( sample_dir.x ), max( abs( sample_dir.y ), abs( sample_dir.z ) ) );
					sample_level += 0.75f * log2( dot( sample_dir, sample_dir ) );

					// sample cubemap
					color.xyz += textureLod(tex_in, sample_dir, sample_level).xyz * sample_weight;
					color.w += sample_weight;
				}
			}
		}		
	}
	color /= color.w;
    color.x = max( 0.0, color.x );
    color.y = max( 0.0, color.y );
    color.z = max( 0.0, color.z );
    color.w = 1.0;

	switch ( level )
	{
	case 0:
		imageStore(tex_out0, id, color);
		break;
	case 1:
		imageStore(tex_out1, id, color);
		break;
	case 2:
		imageStore(tex_out2, id, color);
		break;
	case 3:
		imageStore(tex_out3, id, color);
		break;
	case 4:
		imageStore(tex_out4, id, color);
		break;
	case 5:
		imageStore(tex_out5, id, color);
		break;
	case 6:
		imageStore(tex_out6, id, color);
		break;
	}
	
}
)";

EnvironmentMapCreator::EnvironmentMapCreator() : m_buf_coeffs(sizeof(s_coeffs), GL_UNIFORM_BUFFER)
{
	GLShader comp_downsample(GL_COMPUTE_SHADER, g_compute_downsample.c_str());
	m_prog_downsample = (std::unique_ptr<GLProgram>)(new GLProgram(comp_downsample));

	GLShader comp_filter(GL_COMPUTE_SHADER, g_compute_filter.c_str());
	m_prog_filter = (std::unique_ptr<GLProgram>)(new GLProgram(comp_filter));
	 
	glGenTextures(1, &m_tex_src);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex_src);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 8, GL_RGBA8, 128, 128);

	m_buf_coeffs.upload(s_coeffs);
}

EnvironmentMapCreator::~EnvironmentMapCreator()
{
	glDeleteTextures(1, &m_tex_src);
}

void EnvironmentMapCreator::CreateSH(glm::vec4 shCoefficients[9], unsigned tex_id, int tex_dim, const glm::quat& rotation)
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex_id);

	std::vector<float> faces[6];
	for (int i = 0; i < 6; i++)
	{
		faces[i].resize(tex_dim * tex_dim * 4);
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, GL_FLOAT, faces[i].data());
	}

	float pixelSize = 2.0f / tex_dim;
	float totalWeight = 0.0f;
	memset(shCoefficients, 0, sizeof(glm::vec4) * 9);

	for (int i = 0; i < 6; i++)
	{
		std::vector<float>& face = faces[i];

		for (int j = 0; j < tex_dim * tex_dim; j += 4)
		{
			float dir_x[4];
			float dir_y[4];
			float dir_z[4];
			float weight[4];
			glm::vec3 color[4];

			for (int k = 0; k < 4; k++)
			{
				int pixelIndex = j + k;

				float col = -1.0f + ((float)(pixelIndex % tex_dim) + 0.5f) * pixelSize;
				float row = -1.0f + ((float)(pixelIndex / tex_dim) + 0.5f) * pixelSize;
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
				dir = rotation* dir;
				dir_x[k] = dir.x;
				dir_y[k] = dir.y;
				dir_z[k] = dir.z;

				const float* pixel = &face[(size_t)pixelIndex * 4];
				color[k] = { pixel[0], pixel[1], pixel[2] };
			}

#if 1
			float shBasis[36];
			SHEval3(dir_x, dir_y, dir_z, shBasis);

			for (int k = 0; k < 36; k++)
			{
				int idx_col = k % 4;
				int idx_coeff = k / 4;
				shCoefficients[idx_coeff] += glm::vec4(shBasis[k] * weight[idx_col] * color[idx_col], 0.0f);
			}
#else
			for (int k = 0; k < 4; k++)
			{
				float shBasis[9];
				SHEval3(dir_x[k], dir_y[k], dir_z[k], shBasis);

				for (int l = 0; l < 9; l++)
				{
					shCoefficients[l] += glm::vec4(shBasis[l] * weight[k] * color[k], 0.0f);
				}
			}
#endif
		}
	}

	float norm = (4.0f * PI) / totalWeight;

	for (int k = 0; k < 9; k++)
	{
		shCoefficients[k] *= norm;
	}
}

void EnvironmentMapCreator::CreateReflection(ReflectionMap& reflection, const GLCubemap* cubemap)
{
	// downsample pass
	glUseProgram(m_prog_downsample->m_id);

	{
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->tex_id);
		glUniform1i(0, 0);

		glBindImageTexture(0, m_tex_src, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);

		glUniform1f(1, 0.0f);

		glDispatchCompute(128 / 8, 128 / 8, 6);
	}

	for (int level = 0; level < 7; level++)
	{
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex_src);
		glUniform1i(0, 0);

		glBindImageTexture(0, m_tex_src, level + 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);

		glUniform1f(1, (float)level);

		int w, h;
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, level + 1, GL_TEXTURE_WIDTH, &w);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, level + 1, GL_TEXTURE_HEIGHT, &h);
		glDispatchCompute((w + 7) / 8, (h + 7) / 8, 6);
	}

	reflection.allocate();

	// filter pass
	glUseProgram(m_prog_filter->m_id);

	{
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex_src);
		glUniform1i(0, 0);

		glBindImageTexture(0, reflection.tex_id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
		glBindImageTexture(1, reflection.tex_id, 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
		glBindImageTexture(2, reflection.tex_id, 2, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
		glBindImageTexture(3, reflection.tex_id, 3, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
		glBindImageTexture(4, reflection.tex_id, 4, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
		glBindImageTexture(5, reflection.tex_id, 5, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
		glBindImageTexture(6, reflection.tex_id, 6, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_buf_coeffs.m_id);

		glDispatchCompute((128 * 128 + 64 * 64 + 32 * 32 + 16 * 16 + 8 * 8 + 4 * 4 + 2 * 2 + 63) / 64, 6, 1);
	}

	glUseProgram(0);

}

void EnvironmentMapCreator::Create(const GLCubemap * cubemap, EnvironmentMap * envMap, bool irradiance_only)
{	
	if (!irradiance_only)	
	{
		if (envMap->reflection == nullptr)
		{
			envMap->reflection = std::unique_ptr<ReflectionMap>(new ReflectionMap);
		}
		CreateReflection(*envMap->reflection, cubemap);		
	}
	CreateSH(envMap->shCoefficients, cubemap->tex_id);
}

void EnvironmentMapCreator::Create(const CubeImage* image, EnvironmentMap* envMap, bool irradiance_only)
{
	GLCubemap cubemap;
	cubemap.load_memory_rgba(image->images[0].width(), image->images[0].height(),
		image->images[0].data(), image->images[1].data(), image->images[2].data(), image->images[3].data(), image->images[4].data(), image->images[5].data());
	Create(&cubemap, envMap, irradiance_only);
}

void EnvironmentMapCreator::Create(const CubeBackground* background, EnvironmentMap* envMap, bool irradiance_only)
{
	Create(&background->cubemap, envMap, irradiance_only);
}

void EnvironmentMapCreator::Create(const CubeRenderTarget* target, EnvironmentMap* envMap, bool irradiance_only)
{
	Create(target->m_cube_map.get(), envMap, irradiance_only);
}