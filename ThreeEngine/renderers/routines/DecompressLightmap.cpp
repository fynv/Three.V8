#include <string>
#include <GL/glew.h>
#include "DecompressLightmap.h"
#include "lights/ProbeGrid.h"
#include "lights/LODProbeGrid.h"
#include "renderers/LightmapRenderTarget.h"

static std::string g_compute =
R"(#version 430
#DEFINES#

layout (location = 0) uniform sampler2D uTexPosition;
layout (location = 1) uniform sampler2D uTexNormal;
layout (location = 2) uniform usamplerBuffer uValidList;
layout (location = 3) uniform usampler2D uTexProbeVis;

layout (binding=0, rgba16f) uniform image2D uImgLightmap;


#if HAS_PROBE_GRID
layout (std140, binding = 0) uniform ProbeGrid
{
	vec4 uCoverageMin;
	vec4 uCoverageMax;
	ivec4 uDivisions;	
	float uYpower;
	float uNormalBias;
	int uVisRes;
	int uPackSize;
	int uPackRes;
	int uIrrRes;
	int uIrrPackRes;
	float uDiffuseThresh;
	float uDiffuseHigh;
	float uDiffuseLow;
	float uSpecularThresh;
	float uSpecularHigh;
	float uSpecularLow;
};

#elif HAS_LOD_PROBE_GRID
layout (std140, binding = 0) uniform ProbeGrid
{
	vec4 uCoverageMin;
	vec4 uCoverageMax;
	ivec4 uBaseDivisions;	
	int uSubDivisionLevel;
	float uNormalBias;
	int uNumProbes;
	int uVisRes;
	int uPackSize;
	int uPackRes;
	int uIrrRes;
	int uIrrPackRes;
	float uDiffuseThresh;
	float uDiffuseHigh;
	float uDiffuseLow;
	float uSpecularThresh;
	float uSpecularHigh;
	float uSpecularLow;
};

layout (std430, binding = 1) buffer Probes
{
	vec4 bProbeData[];
};

layout (std430, binding = 2) buffer ProbeIndex
{
	int bIndexData[];
};

#endif

layout (location = 4) uniform sampler2D uTexIrr;
layout (location = 5) uniform sampler2D uTexVis;
layout (location = 6) uniform int count_valid;

vec2 signNotZero(in vec2 v)
{
	return vec2((v.x >= 0.0) ? 1.0 : -1.0, (v.y >= 0.0)? 1.0: -1.0);
}

vec2 vec3_to_oct(in vec3 v)
{
	vec2 p = v.xy * (1.0/ (abs(v.x) + abs(v.y) + abs(v.z)));
	return (v.z <= 0.0) ? ((1.0 - abs(p.yx)) * signNotZero(p)) : p;
}

vec3 get_irradiance_common(int idx, in vec3 dir)
{
	vec2 probe_uv = vec3_to_oct(dir)*0.5 + 0.5;
	int pack_x = idx % uPackSize;
	int pack_y = idx / uPackSize;
	vec2 uv = (vec2(pack_x, pack_y) * float(uIrrRes + 2) + (probe_uv * float(uIrrRes) + 1.0))/float(uIrrPackRes);
	return texture(uTexIrr, uv).xyz;
}


vec2 get_mean_dis_common(in vec3 dir, int idx)
{
	vec2 probe_uv = vec3_to_oct(dir)*0.5 + 0.5;
	int pack_x = idx % uPackSize;
	int pack_y = idx / uPackSize;
	vec2 uv = (vec2(pack_x, pack_y) * float(uVisRes + 2) + (probe_uv * float(uVisRes) + 1.0))/float(uPackRes);
	return texture(uTexVis, uv).xy;
}

#if HAS_PROBE_GRID

int get_probe_idx(in ivec3 ipos)
{
	return vert.x + (vert.y + vert.z*uDivisions.y)*uDivisions.x;
}

#elif HAS_LOD_PROBE_GRID

int get_probe_idx(in ivec3 ipos)
{
	int base_offset = uBaseDivisions.x * uBaseDivisions.y * uBaseDivisions.z;

	ivec3 ipos_base = ipos / (1<<uSubDivisionLevel);
	int node_idx = ipos_base.x + (ipos_base.y + ipos_base.z * uBaseDivisions.y) * uBaseDivisions.x;
	int probe_idx = bIndexData[node_idx];

	int lod = 0;
	int digit_mask = 1 << (uSubDivisionLevel -1);
	while(lod<uSubDivisionLevel && probe_idx>=uNumProbes)
	{		
		int offset = base_offset + (probe_idx - uNumProbes)*8;
		int sub = 0;
		if ((ipos.x & digit_mask) !=0) sub+=1;
		if ((ipos.y & digit_mask) !=0) sub+=2;
		if ((ipos.z & digit_mask) !=0) sub+=4;
		node_idx = offset + sub;
		probe_idx = bIndexData[node_idx];

		lod++;
		digit_mask >>=1;	
	}

	return probe_idx;
}

#endif

#define RECIPROCAL_PI 0.3183098861837907

layout(local_size_x = 64) in;

void main()
{
	int id = int(gl_GlobalInvocationID.x);
	if (id >= count_valid) return;
	ivec2 texel_coord = ivec2(texelFetch(uValidList, id).xy);	
	vec3 pos = texelFetch(uTexPosition, texel_coord, 0).xyz;
	vec3 norm = texelFetch(uTexNormal, texel_coord, 0).xyz;
	uint mask = texelFetch(uTexProbeVis, texel_coord, 0).x;

	vec3 size_grid = uCoverageMax.xyz - uCoverageMin.xyz;
	vec3 pos_normalized = (pos - uCoverageMin.xyz)/size_grid;

	#if HAS_PROBE_GRID
	pos_normalized.y = pow(pos_normalized.y, 1.0/uYpower);
	ivec3 divs = uDivisions;
#elif HAS_LOD_PROBE_GRID
	ivec3 divs = uBaseDivisions.xyz * (1<<uSubDivisionLevel);
#endif

	vec3 pos_voxel = pos_normalized * vec3(divs) - vec3(0.5);
	pos_voxel = clamp(pos_voxel, vec3(0.0), vec3(divs) - vec3(1.0));

	ivec3 i_voxel = clamp(ivec3(pos_voxel), ivec3(0), ivec3(divs) - ivec3(2));
	vec3 frac_voxel = pos_voxel - vec3(i_voxel);	
	
	float sum_weight = 0.0;
	vec3 irr = vec3(0.0);

	for (int i=0; i<8; i++)
	{
		if ((mask & (1<<i))==0) continue;

		int x = i & 1;
		int y = (i >> 1) & 1;
		int z = (i >> 2) & 1;

		vec3 w = vec3(1.0) - abs(vec3(x,y,z) - frac_voxel);
		float weight = w.x * w.y * w.z;
		if (weight <= 0.0) continue;
		
		ivec3 vert = i_voxel + ivec3(x,y,z);
		int idx_probe = get_probe_idx(vert);

#if HAS_PROBE_GRID
		vec3 vert_normalized = (vec3(vert) + vec3(0.5))/vec3(uDivisions);
		vert_normalized.y = pow(vert_normalized.y, uYpower); 
		vec3 probe_world = vert_normalized * size_grid + uCoverageMin.xyz;
#elif HAS_LOD_PROBE_GRID		
		vec4 pos_lod = bProbeData[idx_probe];
		vec3 probe_world = pos_lod.xyz;
#endif
		vec3 sample_dir = norm;

		// parallax correction
		float distance = get_mean_dis_common(sample_dir, idx_probe).x;					
		vec3 pos_to = pos + distance * sample_dir;
		sample_dir = normalize(pos_to - probe_world);		

		sum_weight += weight;
		irr += get_irradiance_common(idx_probe, sample_dir) * weight;
	}

	if (sum_weight > 0.0) irr/=sum_weight;
	imageStore(uImgLightmap, texel_coord, vec4(irr * RECIPROCAL_PI, 1.0));
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

DecompressLightmap::DecompressLightmap(bool has_probe_grid, bool has_lod_probe_grid)
	: has_probe_grid(has_probe_grid)
	, has_lod_probe_grid(has_lod_probe_grid)
{
	std::string s_compute = g_compute;

	std::string defines = "";
	if (has_probe_grid)
	{
		defines += "#define HAS_PROBE_GRID 1\n";
	}
	else
	{
		defines += "#define HAS_PROBE_GRID 0\n";
	}
	if (has_lod_probe_grid)
	{
		defines += "#define HAS_LOD_PROBE_GRID 1\n";
	}
	else
	{
		defines += "#define HAS_LOD_PROBE_GRID 0\n";
	}

	replace(s_compute, "#DEFINES#", defines.c_str());

	GLShader comp_shader(GL_COMPUTE_SHADER, s_compute.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
}


void DecompressLightmap::decompress(const RenderParams& params)
{
	const LightmapRenderTarget* atlas = params.atlas;

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glUseProgram(m_prog->m_id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atlas->m_tex_position->tex_id);
	glUniform1i(0, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, atlas->m_tex_normal->tex_id);
	glUniform1i(1, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_BUFFER, atlas->valid_list->tex_id);
	glUniform1i(2, 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, params.probe_visibility_map->tex_id);
	glUniform1i(3, 3);

	glBindImageTexture(0, params.light_map->tex_id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);

	if (has_probe_grid)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.probe_grid->m_constant.m_id);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, params.probe_grid->m_tex_irradiance->tex_id);
		glUniform1i(4, 4);

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, params.probe_grid->m_tex_visibility->tex_id);
		glUniform1i(5, 5);

	}
	else if (has_lod_probe_grid)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.lod_probe_grid->m_constant.m_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, params.lod_probe_grid->m_probe_bufs[0]->m_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, params.lod_probe_grid->m_sub_index_buf->m_id);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, params.lod_probe_grid->m_tex_irradiance->tex_id);
		glUniform1i(4, 4);

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, params.lod_probe_grid->m_tex_visibility->tex_id);
		glUniform1i(5, 5);
	}

	glUniform1i(6, atlas->count_valid);

	int blocks = (atlas->count_valid + 63) / 64;
	glDispatchCompute(blocks, 1, 1);

	glUseProgram(0);

}

