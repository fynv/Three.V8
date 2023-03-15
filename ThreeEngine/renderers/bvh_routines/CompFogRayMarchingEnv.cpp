#include <string>
#include <GL/glew.h>
#include "renderers/BVHRenderTarget.h"
#include "CompFogRayMarchingEnv.h"

static std::string g_compute_part0 =
R"(#version 430

#DEFINES#

layout (std140, binding = 0) uniform Fog
{
	vec4 fog_rgba;
	int max_num_steps;
	float min_step;
};

layout (location = 0) uniform sampler2D uDepthTex;


#define RECIPROCAL_PI 0.3183098861837907

uint InitRandomSeed(uint val0, uint val1)
{
	uint v0 = val0, v1 = val1, s0 = 0u;

	for (uint n = 0u; n < 16u; n++)
	{
		s0 += 0x9e3779b9u;
		v0 += ((v1 << 4) + 0xa341316cu) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4u);
		v1 += ((v0 << 4) + 0xad90777du) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761eu);
	}

	return v0;
}

uint RandomInt(inout uint seed)
{
    return (seed = 1664525u * seed + 1013904223u);
}

float RandomFloat(inout uint seed)
{
	return (float(RandomInt(seed) & 0x00FFFFFFu) / float(0x01000000));
}


#if HAS_PROBE_GRID
layout (std140, binding = 1) uniform ProbeGrid
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

layout (std430, binding = 2) buffer Probes
{
	vec4 bSHCoefficients[];
};

#if PROBE_REFERENCE_RECORDED
layout (std430, binding = 3) buffer ProbeReferences
{
	uint bReferenced[];
};
#endif

#endif


#if HAS_LOD_PROBE_GRID
layout (std140, binding = 1) uniform ProbeGrid
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


layout (std430, binding = 2) buffer Probes
{
	vec4 bProbeData[];
};


layout (std430, binding = 3) buffer ProbeIndex
{
	int bIndexData[];
};

#endif

#if HAS_PROBE_GRID || HAS_LOD_PROBE_GRID

layout (location = 1) uniform sampler2D uTexVis;


vec2 signNotZero(in vec2 v)
{
	return vec2((v.x >= 0.0) ? 1.0 : -1.0, (v.y >= 0.0)? 1.0: -1.0);
}

vec2 vec3_to_oct(in vec3 v)
{
	vec2 p = v.xy * (1.0/ (abs(v.x) + abs(v.y) + abs(v.z)));
	return (v.z <= 0.0) ? ((1.0 - abs(p.yx)) * signNotZero(p)) : p;
}

float get_visibility_common(in vec3 pos_world, in vec3 spacing, int idx, in vec3 vert_world, float scale)
{
	vec3 dir = pos_world - vert_world;	
	float dis = length(dir);
	dir = normalize(dir);
	vec3 dir_abs = abs(dir)/spacing;

	int major_dir = 0;
	if (dir_abs.y>dir_abs.x)
	{
		if (dir_abs.z>dir_abs.y)
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
		if (dir_abs.z>dir_abs.x)
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
	
	float max_dis = length(dir_abs);
	vec2 probe_uv = vec3_to_oct(dir)*0.5 + 0.5;
	int pack_x = idx % uPackSize;
	int pack_y = idx / uPackSize;
	vec2 uv = (vec2(pack_x, pack_y) * float(uVisRes + 2) + (probe_uv * float(uVisRes) + 1.0))/float(uPackRes);
	vec2 mean_dis_var = max_dis * texture(uTexVis, uv).xy;
	float mean_dis = mean_dis_var.x;
	float mean_var = mean_dis_var.y;
	float mean_var2 = mean_var * mean_var;
	float delta = max(dis - mean_dis, 0.0);
	float delta2 = delta*delta * scale * scale;	
	return mean_var2/(mean_var2 + delta2);
}

#endif


#if HAS_PROBE_GRID


float get_visibility(in vec3 pos_world, in ivec3 vert, in vec3 vert_world)
{
	vec3 size_grid = uCoverageMax.xyz - uCoverageMin.xyz;
	vec3 spacing = size_grid/vec3(uDivisions);

	float y0 = pow((float(vert.y) + 0.5f) / float(uDivisions.y), uYpower);
	float y1 = pow((float(vert.y+1) + 0.5f) / float(uDivisions.y), uYpower);
	spacing.y = (y1-y0)*size_grid.y;
	
	int idx = vert.x + (vert.y + vert.z*uDivisions.y)*uDivisions.x;
	return get_visibility_common(pos_world, spacing, idx, vert_world, 1.0);
}


void acc_coeffs(inout vec4 coeffs0, in ivec3 vert, in float weight)
{
	int idx = vert.x + (vert.y + vert.z*uDivisions.y)*uDivisions.x;
	int offset = idx*9;
	coeffs0 += bSHCoefficients[offset]*weight;
#if PROBE_REFERENCE_RECORDED
	bReferenced[idx] = 1;
#endif
}


vec3 getIrradiance(in vec3 pos_world)
{
	vec3 size_grid = uCoverageMax.xyz - uCoverageMin.xyz;
	vec3 pos_normalized = (pos_world - uCoverageMin.xyz)/size_grid;
	pos_normalized.y = pow(pos_normalized.y, 1.0/uYpower);	
	vec3 pos_voxel = pos_normalized * vec3(uDivisions) - vec3(0.5);
	pos_voxel = clamp(pos_voxel, vec3(0.0), vec3(uDivisions) - vec3(1.0));
	
	ivec3 i_voxel = clamp(ivec3(pos_voxel), ivec3(0), ivec3(uDivisions) - ivec3(2));
	vec3 frac_voxel = pos_voxel - vec3(i_voxel);

	float sum_weight = 0.0;
	vec4 coeffs0 = vec4(0.0);

	for (int z=0;z<2;z++)
	{
		for (int y=0;y<2;y++)
		{
			for (int x=0;x<2;x++)
			{	
				ivec3 vert = i_voxel + ivec3(x,y,z);
				vec3 vert_normalized = (vec3(vert) + vec3(0.5))/vec3(uDivisions);
				vert_normalized.y = pow(vert_normalized.y, uYpower); 
				vec3 vert_world = vert_normalized * size_grid + uCoverageMin.xyz;
				float weight = get_visibility(pos_world, vert,vert_world);	
				const float crushThreshold = 0.2;
				if (weight < crushThreshold) {
					weight *= weight * weight / (crushThreshold*crushThreshold); 
				}			
	
				vec3 w = vec3(1.0) - abs(vec3(x,y,z) - frac_voxel);
				weight *= w.x * w.y * w.z;
				if (weight>0.0)
				{					
					sum_weight += weight;					
					acc_coeffs(coeffs0, vert, weight);
				}
			}
		}
	}

	if (sum_weight>0)
	{
		coeffs0/=sum_weight;
		return coeffs0.xyz * 0.886227;
	}

	return vec3(0.0);
}

#endif

)";

static std::string g_compute_part1 =
R"(

#if HAS_LOD_PROBE_GRID


float get_visibility(in vec3 wpos, int idx, int lod, in vec3 vert_world)
{
	vec3 size_grid = uCoverageMax.xyz - uCoverageMin.xyz;
	vec3 spacing = size_grid/vec3(uBaseDivisions);	
	spacing *= 1.0 / float(1 << lod);
	float scale = float(1<<(uBaseDivisions -lod));
	return get_visibility_common(wpos, spacing, idx, vert_world, scale);
}


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

void acc_coeffs(inout vec4 coeffs0, int idx, in float weight)
{
	int offset = idx*10 + 1;
	coeffs0+=bProbeData[offset]*weight;
}

vec3 getIrradiance(in vec3 pos_world)
{	
	ivec3 divs = uBaseDivisions.xyz * (1<<uSubDivisionLevel);

	vec3 size_grid = uCoverageMax.xyz - uCoverageMin.xyz;
	vec3 pos_normalized = (pos_world - uCoverageMin.xyz)/size_grid;	
	vec3 pos_voxel = pos_normalized * vec3(divs) - vec3(0.5);
	pos_voxel = clamp(pos_voxel, vec3(0.0), vec3(divs) - vec3(1.0));

	ivec3 i_voxel = clamp(ivec3(pos_voxel), ivec3(0), ivec3(divs) - ivec3(2));
	vec3 frac_voxel = pos_voxel - vec3(i_voxel);

	vec4 coeffs = vec4(0.0);
	float sum_weight = 0.0;

	for (int z=0;z<2;z++)
	{
		for (int y=0;y<2;y++)
		{
			for (int x=0;x<2;x++)
			{				
				ivec3 vert = i_voxel + ivec3(x,y,z);					
				int idx_probe = get_probe_idx(vert);
				vec4 pos_lod = bProbeData[idx_probe*10];
				vec3 probe_world = pos_lod.xyz;
				float weight = get_visibility(pos_world, idx_probe, int(pos_lod.w), probe_world);		
				const float crushThreshold = 0.2;
				if (weight < crushThreshold) {
					weight *= weight * weight / (crushThreshold*crushThreshold); 
				}
				vec3 w = vec3(1.0) - abs(vec3(x,y,z) - frac_voxel);
				weight *= w.x * w.y * w.z;
				if (weight>0.0)
				{
					sum_weight += weight;
					acc_coeffs(coeffs, idx_probe, weight);
				}
			}
		}
	}	

	if (sum_weight > 0.0)	
	{
		coeffs /= sum_weight;
		return coeffs.xyz * 0.886227;
	}
	return vec3(0.0);
}

#endif

layout (binding=0, rgba16f) uniform image2D uImgColor;

layout(local_size_x = 8, local_size_y = 8) in;

layout (std140, binding = 4) uniform Camera
{
	mat4 uProjMat;
	mat4 uViewMat;	
	mat4 uInvProjMat;
	mat4 uInvViewMat;	
	vec3 uEyePos;
};

void main()
{
	ivec2 size = imageSize(uImgColor);
	ivec2 id = ivec3(gl_GlobalInvocationID).xy;	
	if (id.x>= size.x || id.y >=size.y) return;

	ivec2 screen = ivec2(id.x, id.y);
	vec4 clip = vec4((vec2(screen) + 0.5)/vec2(size)*2.0-1.0, 0.0, 1.0);
	vec4 view = uInvProjMat * clip; view /= view.w;
	vec3 world = vec3(uInvViewMat*view);
	vec3 dir = normalize(world - uEyePos);

	float dis = texelFetch(uDepthTex, id, 0).x;

	float step = dis/float(max_num_steps)*8.0;
	if (step<min_step*8.0) step = min_step*8.0;

	float step_alpha =  1.0 - pow(1.0 - fog_rgba.w, step);

	uint seed = InitRandomSeed(uint(screen.x), uint(screen.y));
	float delta = RandomFloat(seed);

	vec3 col = vec3(0.0);
	float start = step * (delta - 0.5);
	for (float t = start; t<dis; t+=step)
	{
		float _step = min(step, dis - t);
		float _step_alpha = (_step == step) ? step_alpha :  (1.0 - pow(1.0 - fog_rgba.w, _step));
		float sample_t = max(t + _step*0.5, 0.0);
		
		vec3 pos_world = uEyePos + dir * sample_t;
		float att =  pow(1.0 - fog_rgba.w, sample_t);
		
		vec3 irradiance = getIrradiance(pos_world);

		col+=fog_rgba.xyz*irradiance*RECIPROCAL_PI * _step_alpha* att;
	}

	vec4 base = imageLoad(uImgColor, id);
	imageStore(uImgColor, id, base + vec4(col, 0.0));
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

CompFogRayMarchingEnv::CompFogRayMarchingEnv(const Options& options) : m_options(options)
{
	std::string s_compute = g_compute_part0 + g_compute_part1;

	std::string defines = "";

	if (options.has_probe_grid)
	{
		defines += "#define HAS_PROBE_GRID 1\n";
	}
	else
	{
		defines += "#define HAS_PROBE_GRID 0\n";
	}

	if (options.probe_reference_recorded)
	{
		defines += "#define PROBE_REFERENCE_RECORDED 1\n";
	}
	else
	{
		defines += "#define PROBE_REFERENCE_RECORDED 0\n";
	}

	if (options.has_lod_probe_grid)
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

void CompFogRayMarchingEnv::render(const RenderParams& params)
{
	int width = params.target->m_width;
	int height = params.target->m_height;

	glUseProgram(m_prog->m_id);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.constant_fog->m_id);

	if (m_options.has_probe_grid)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.lights->probe_grid->m_constant.m_id);
		if (params.lights->probe_grid->m_probe_buf != nullptr)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, params.lights->probe_grid->m_probe_buf->m_id);
		}
		if (m_options.probe_reference_recorded)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, params.lights->probe_grid->m_ref_buf->m_id);
		}
	}

	if (m_options.has_lod_probe_grid)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.lights->lod_probe_grid->m_constant.m_id);
		if (params.lights->lod_probe_grid->m_probe_buf != nullptr)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, params.lights->lod_probe_grid->m_probe_buf->m_id);
		}
		if (params.lights->lod_probe_grid->m_sub_index_buf != nullptr)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, params.lights->lod_probe_grid->m_sub_index_buf->m_id);
		}
	}


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, params.target->m_tex_depth->tex_id);
	glUniform1i(0, 0);

	if (m_options.has_probe_grid)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, params.lights->probe_grid->m_tex_visibility->tex_id);
		glUniform1i(1, 1);
	}

	if (m_options.has_lod_probe_grid)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, params.lights->lod_probe_grid->m_tex_visibility->tex_id);
		glUniform1i(1, 1);
	}

	glBindBufferBase(GL_UNIFORM_BUFFER, 4, params.constant_camera->m_id);

	glBindImageTexture(0, params.target->m_tex_video->tex_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);

	glm::ivec2 blocks = { (width + 7) / 8, (height + 7) / 8 };
	glDispatchCompute(blocks.x, blocks.y, 1);

	glUseProgram(0);
}