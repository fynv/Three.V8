#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <string>
#include "ReflectionDistanceCreator.h"
#include "renderers/GLRenderTarget.h"

static std::string g_comp_linear =
R"(#version 430

#DEFINES#

#if MSAA
layout (location = 0) uniform highp sampler2DMS uDepthTex;
#else
layout (location = 0) uniform sampler2D uDepthTex;
#endif

layout (location = 1) uniform mat4 uInvProjMat;

layout (binding=0, r32f) uniform highp writeonly image2D uDisImg;

layout(local_size_x = 8, local_size_y = 8) in;

#if MSAA
float FetchDepth(in ivec2 id)
{
	float depth0 =  texelFetch(uDepthTex, id, 0).x;
	float depth1 =  texelFetch(uDepthTex, id, 1).x;
	float depth2 =  texelFetch(uDepthTex, id, 2).x;
	float depth3 =  texelFetch(uDepthTex, id, 3).x;
	return 0.25 * (depth0 + depth1 + depth2 + depth3);
}
#else
float FetchDepth(in ivec2 id)
{
	return texelFetch(uDepthTex, id, 0).x;
}
#endif

void main()
{
	ivec2 size = imageSize(uDisImg);
	ivec2 id = ivec3(gl_GlobalInvocationID).xy;
	if (id.x>= size.x || id.y >=size.y) return;
	vec2 UV = (vec2(id)+0.5)/vec2(size);
	float depth = FetchDepth(id);
	vec3 pos_clip = vec3(UV, depth)*2.0-1.0;
	vec4 view_pos = uInvProjMat* vec4(pos_clip, 1.0);
	view_pos/=view_pos.w;
	float dis = length(view_pos.xyz);
	imageStore(uDisImg, id, vec4(dis));
}
)";

static std::string g_comp_filter =
R"(#version 430
layout (location = 0) uniform samplerCube tex_in;
layout (binding=0, r32f) uniform imageCube tex_out;

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
	int res_out = imageSize(tex_out).x;
	if (id.x < res_out && id.y < res_out)
	{
		float inv_res_out = 1.0 / float(res_out);
		float u0 = ( float(id.x) * 2.0 + 1.0 - 0.75 ) * inv_res_out - 1.0;
		float u1 = ( float(id.x) * 2.0 + 1.0 + 0.75 ) * inv_res_out - 1.0;

		float v0 = ( float(id.y) * 2.0 + 1.0 - 0.75 ) * -inv_res_out + 1.0;
		float v1 = ( float(id.y) * 2.0 + 1.0 + 0.75 ) * -inv_res_out + 1.0;

		vec4 weights;
		weights.x = calcWeight( u0, v0 );
		weights.y = calcWeight( u1, v0 );
		weights.z = calcWeight( u0, v1 );
		weights.w = calcWeight( u1, v1 );

		float wsum = 0.5 / ( weights.x + weights.y + weights.z + weights.w );
		weights = weights*wsum + 0.125;

		vec3 dir;
		float distance;

		switch ( id.z )
		{
		case 0:
			get_dir_0( dir, u0, v0 );
			distance = textureLod(tex_in, dir, lod).x * weights.x;

			get_dir_0( dir, u1, v0 );
			distance += textureLod(tex_in, dir, lod).x * weights.y;			

			get_dir_0( dir, u0, v1 );
			distance += textureLod(tex_in, dir, lod).x * weights.z;

			get_dir_0( dir, u1, v1 );
			distance += textureLod(tex_in, dir, lod).x * weights.w;
			break;

		case 1:
			get_dir_1( dir, u0, v0 );
			distance = textureLod(tex_in, dir, lod).x * weights.x;

			get_dir_1( dir, u1, v0 );
			distance += textureLod(tex_in, dir, lod).x * weights.y;			

			get_dir_1( dir, u0, v1 );
			distance += textureLod(tex_in, dir, lod).x * weights.z;

			get_dir_1( dir, u1, v1 );
			distance += textureLod(tex_in, dir, lod).x * weights.w;
			break;

		case 2:
			get_dir_2( dir, u0, v0 );
			distance = textureLod(tex_in, dir, lod).x * weights.x;

			get_dir_2( dir, u1, v0 );
			distance += textureLod(tex_in, dir, lod).x * weights.y;			

			get_dir_2( dir, u0, v1 );
			distance += textureLod(tex_in, dir, lod).x * weights.z;

			get_dir_2( dir, u1, v1 );
			distance += textureLod(tex_in, dir, lod).x * weights.w;
			break;

		case 3:
			get_dir_3( dir, u0, v0 );
			distance = textureLod(tex_in, dir, lod).x * weights.x;

			get_dir_3( dir, u1, v0 );
			distance += textureLod(tex_in, dir, lod).x * weights.y;			

			get_dir_3( dir, u0, v1 );
			distance += textureLod(tex_in, dir, lod).x * weights.z;

			get_dir_3( dir, u1, v1 );
			distance += textureLod(tex_in, dir, lod).x * weights.w;
			break;

		case 4:
			get_dir_4( dir, u0, v0 );
			distance = textureLod(tex_in, dir, lod).x * weights.x;

			get_dir_4( dir, u1, v0 );
			distance += textureLod(tex_in, dir, lod).x * weights.y;			

			get_dir_4( dir, u0, v1 );
			distance += textureLod(tex_in, dir, lod).x * weights.z;

			get_dir_4( dir, u1, v1 );
			distance += textureLod(tex_in, dir, lod).x * weights.w;
			break;

		case 5:
			get_dir_5( dir, u0, v0 );
			distance = textureLod(tex_in, dir, lod).x * weights.x;

			get_dir_5( dir, u1, v0 );
			distance += textureLod(tex_in, dir, lod).x * weights.y;			

			get_dir_5( dir, u0, v1 );
			distance += textureLod(tex_in, dir, lod).x * weights.z;

			get_dir_5( dir, u1, v1 );
			distance += textureLod(tex_in, dir, lod).x * weights.w;
			break;
		}

		imageStore(tex_out, id, vec4(distance));
	}

}
)";


inline void replace(std::string& str, const char* target, const char* source)
{
	int start = 0;
	size_t target_len = strlen(target);
	size_t source_len = strlen(source);
	while (true)
	{
		size_t pos = str.find(target, start);
		if (pos == std::string::npos) break;
		str.replace(pos, target_len, source);
		start = pos + source_len;
	}
}

ReflectionDistanceCreator::ReflectionDistanceCreator(bool msaa) : m_msaa(msaa)
{
	std::string s_comp_linear = g_comp_linear;

	std::string defines = "";
	if (msaa)
	{
		defines += "#define MSAA 1\n";
	}
	else
	{
		defines += "#define MSAA 0\n";
	}

	replace(s_comp_linear, "#DEFINES#", defines.c_str());

	GLShader comp_linear(GL_COMPUTE_SHADER, s_comp_linear.c_str());
	m_prog_linearize = (std::unique_ptr<GLProgram>)(new GLProgram(comp_linear));

	GLShader comp_filter(GL_COMPUTE_SHADER, g_comp_filter.c_str());
	m_prog_filter = (std::unique_ptr<GLProgram>)(new GLProgram(comp_filter));

	glGenTextures(1, &m_tex_tmp0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex_tmp0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 3, GL_R32F, 256, 256);	
}

ReflectionDistanceCreator::~ReflectionDistanceCreator()
{
	glDeleteTextures(1, &m_tex_tmp0);	
}

const double PI = 3.14159265359;

void ReflectionDistanceCreator::Create(const CubeRenderTarget* target, ReflectionMap* reflection, float z_near, float z_far)
{	
	glm::mat4 projectionMatrix = glm::perspective((float)PI * 0.5f, 1.0f, z_near, z_far);
	glm::mat4 projectionMatrixInverse = glm::inverse(projectionMatrix);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glUseProgram(m_prog_linearize->m_id);
	for (int i = 0; i < 6; i++)
	{
		GLRenderTarget* face = target->m_faces[i].get();
		glActiveTexture(GL_TEXTURE0);
		if (m_msaa)
		{
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, face->m_tex_depth->tex_id);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, face->m_tex_depth->tex_id);
		}		
		glUniform1i(0, 0);
		glUniformMatrix4fv(1, 1, GL_FALSE, (float*)&projectionMatrixInverse);		
		glBindImageTexture(0, m_tex_tmp0, 0, GL_FALSE, i, GL_WRITE_ONLY, GL_R32F);

		glm::ivec2 blocks = { 256 / 8, 256 / 8 };
		glDispatchCompute(blocks.x, blocks.y, 1);		
	}

	glUseProgram(m_prog_filter->m_id);

	{
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex_tmp0);
		glUniform1i(0, 0);

		glBindImageTexture(0, m_tex_tmp0, 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);

		glUniform1f(1, 0.0f);

		glDispatchCompute(128 / 8, 128 / 8, 6);
	}

	{
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex_tmp0);
		glUniform1i(0, 0);

		glBindImageTexture(0, reflection->tex_id_dis, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);

		glUniform1f(1, 1.0f);

		glDispatchCompute(64 / 8, 64 / 8, 6);
	}

	{
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, reflection->tex_id_dis);
		glUniform1i(0, 0);

		glBindImageTexture(0, m_tex_tmp0, 2, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);

		glUniform1f(1, 0.0f);

		glDispatchCompute(64 / 8, 64 / 8, 6);
	}

	{
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex_tmp0);
		glUniform1i(0, 0);

		glBindImageTexture(0, reflection->tex_id_dis, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);

		glUniform1f(1, 2.0f);

		glDispatchCompute(64 / 8, 64 / 8, 6);
	}

	glUseProgram(0);	
}

