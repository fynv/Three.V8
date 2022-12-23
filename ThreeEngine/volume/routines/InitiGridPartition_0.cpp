#include <string>
#include <GL/glew.h>
#include "InitiGridPartition.h"
#include "volume/VolumeData.h"
#include "volume/GridPartition.h"

static std::string g_compute =
R"(#version 430

#DEFINES#

layout (location = 0) uniform sampler3D uTex;

#if BYTES_PER_PIXEL == 1
layout (binding=0, rg8) uniform image3D uMinMax;
#elif BYTES_PER_PIXEL == 2
layout (binding=0, rg16) uniform image3D uMinMax;
#endif

layout (std140, binding = 1) uniform Const
{
	ivec4 uSize;
	ivec4 uBsize;
	ivec4 uBnum;
};

layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in;

void main()
{
	ivec3 id = ivec3(gl_GlobalInvocationID);
	if (id.x>=uBnum.x || id.y>=uBnum.y || id.z>=uBnum.z) return;
	
	ivec3 begin = id * uBsize.xyz;
	ivec3 end = begin + uBsize.xyz;

	float minV = 1.0;
	float maxV = 0.0;

	for (int z = begin.z; z<=end.z && z<uSize.z; z++)
	{
		for (int y = begin.y; y<=end.y && y<uSize.y; y++)
		{
			for (int x = begin.x; x<=end.x && x<uSize.x; x++)
			{
				float v = texelFetch(uTex, ivec3(x,y,z), 0).x;
				if (v<minV) minV = v;
				if (v>maxV) maxV = v;
			}
		}
	}

	imageStore(uMinMax, id, vec4(minV,maxV,minV,maxV));
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


InitGridPartition::InitGridPartition(int bytes_per_pixel)
{
	std::string defines = "";
	{
		char line[64];
		sprintf(line, "#define BYTES_PER_PIXEL %d\n", bytes_per_pixel);
		defines += line;
	}

	std::string s_compute = g_compute;
	replace(s_compute, "#DEFINES#", defines.c_str());

	GLShader comp_shader(GL_COMPUTE_SHADER, s_compute.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
}

void InitGridPartition::Init(const VolumeData& data, GridPartition& partition, float blockLogRate)
{
	partition.bytes_per_pixel = data.bytes_per_pixel;
	partition.bsize.x = (int)powf((float)data.size.x, blockLogRate);
	partition.bsize.y = (int)powf((float)data.size.y, blockLogRate);
	partition.bsize.z = (int)powf((float)data.size.z, blockLogRate);
	partition.bnum.x = (data.size.x + partition.bsize.x - 1) / partition.bsize.x;
	partition.bnum.y = (data.size.y + partition.bsize.y - 1) / partition.bsize.y;
	partition.bnum.z = (data.size.z + partition.bsize.z - 1) / partition.bsize.z;

	glBindTexture(GL_TEXTURE_3D, partition.minmax_texture.tex_id);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	GLenum type = GL_RG8;
	if (data.bytes_per_pixel == 2) type = GL_RG16;
	glTexImage3D(GL_TEXTURE_3D, 0, type, partition.bnum.x, partition.bnum.y, partition.bnum.z, 0, GL_RG, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_3D, 0);

	GLBuffer const_buf(sizeof(glm::ivec4) * 3);
	glm::ivec4 consts[3];
	consts[0] = glm::ivec4(data.size, 0);
	consts[1] = glm::ivec4(partition.bsize, 0);
	consts[2] = glm::ivec4(partition.bnum, 0);
	const_buf.upload(consts);

	glm::ivec3 group_sizes = (partition.bnum + glm::ivec3(3)) / glm::ivec3(4);


	glUseProgram(m_prog->m_id);

	glActiveTexture(GL_TEXTURE0);	
	glBindTexture(GL_TEXTURE_3D, data.texture.tex_id);
	glUniform1i(0, 0);
		
	glBindImageTexture(0, partition.minmax_texture.tex_id, 0, GL_TRUE, 0, GL_WRITE_ONLY, type);	
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, const_buf.m_id);

	glDispatchCompute(group_sizes.x, group_sizes.y, group_sizes.z);
	glUseProgram(0);	
	
}

