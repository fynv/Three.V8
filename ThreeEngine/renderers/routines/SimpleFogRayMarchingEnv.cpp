#include <string>
#include <GL/glew.h>
#include "SimpleFogRayMarchingEnv.h"

static std::string g_vertex =
R"(#version 430
layout (location = 0) out vec2 vUV;
void main()
{
	vec2 grid = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	vec2 vpos = grid * vec2(2.0, 2.0) + vec2(-1.0, -1.0);
	gl_Position = vec4(vpos, 1.0, 1.0);
	vUV = vec2(grid.x, grid.y);
}
)";


static std::string g_frag_part0 =
R"(#version 430

#DEFINES#

layout (location = 0) in vec2 vUV;
layout (location = 0) out vec4 outColor;

layout (std140, binding = 0) uniform Camera
{
	mat4 uProjMat;
	mat4 uViewMat;	
	mat4 uInvProjMat;
	mat4 uInvViewMat;	
	vec3 uEyePos;
};

layout (std140, binding = 1) uniform Fog
{
	vec4 fog_rgba;
	int max_num_steps;
	float min_step;
};


#if MSAA
layout (location = 0) uniform sampler2DMS uDepthTex;
#else
layout (location = 0) uniform sampler2D uDepthTex;
#endif

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
layout (std140, binding = 2) uniform ProbeGrid
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

layout (std430, binding = 3) buffer ProbeSH0
{
	vec4 bProbeSH0[];
};

#if PROBE_REFERENCE_RECORDED
layout (std430, binding = 4) buffer ProbeReferences
{
	uint bReferenced[];
};
#endif

#endif

#if HAS_LOD_PROBE_GRID
layout (std140, binding = 2) uniform ProbeGrid
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


layout (std430, binding = 3) buffer ProbePosLod
{
	vec4 bProbePosLod[];
};

layout (std430, binding = 4) buffer ProbeSH0
{
	vec4 bProbeSH0[];
};


layout (std430, binding = 5) buffer ProbeIndex
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

vec2 get_mean_dis_common(in vec3 dir, int idx)
{
	vec2 probe_uv = vec3_to_oct(dir)*0.5 + 0.5;
	int pack_x = idx % uPackSize;
	int pack_y = idx / uPackSize;
	vec2 uv = (vec2(pack_x, pack_y) * float(uVisRes + 2) + (probe_uv * float(uVisRes) + 1.0))/float(uPackRes);
	return texture(uTexVis, uv).xy;
}

float get_visibility_common(in vec3 wpos, int idx, in vec3 vert_world, float scale)
{		
	vec3 dir = wpos - vert_world;
	float dis = length(dir);
	dir = normalize(dir);	
	
	vec2 mean_dis_var = get_mean_dis_common(dir, idx);
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
	int idx = vert.x + (vert.y + vert.z*uDivisions.y)*uDivisions.x;
	return get_visibility_common(pos_world, idx, vert_world, 1.0);
}


void acc_coeffs(inout vec4 coeffs0, in ivec3 vert, in float weight)
{
	int idx = vert.x + (vert.y + vert.z*uDivisions.y)*uDivisions.x;
	coeffs0 += bProbeSH0[idx]*weight;
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

static std::string g_frag_part1 =
R"(

#if HAS_LOD_PROBE_GRID

float get_visibility(in vec3 wpos, int idx, int lod, in vec3 vert_world)
{
	float scale = float(1<<(uSubDivisionLevel -lod));
	return get_visibility_common(wpos, idx, vert_world, scale);
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
	coeffs0 += bProbeSH0[idx]*weight;
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
				vec4 pos_lod = bProbePosLod[idx_probe];
				vec3 probe_world = pos_lod.xyz;
				float weight = get_visibility(pos_world, idx_probe, int(pos_lod.w), probe_world);
				const float crushThreshold = 0.2;
				if (weight < crushThreshold) {
					weight *= weight * weight / (crushThreshold*crushThreshold); 
				}
				vec3 w = vec3(1.0) - abs(vec3(x,y,z) - frac_voxel);
				weight *= w.x * w.y * w.z;
				if (weight > 0.0)
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

void main()
{
	ivec2 coord = ivec2(gl_FragCoord.xy);	
	float depth = 0.0;
#if MSAA
	{
		float depth0 =  texelFetch(uDepthTex, coord, 0).x;
		float depth1 =  texelFetch(uDepthTex, coord, 1).x;
		float depth2 =  texelFetch(uDepthTex, coord, 2).x;
		float depth3 =  texelFetch(uDepthTex, coord, 3).x;
		depth = 0.5 * (depth0 + depth1 + depth2 + depth3) - 1.0;
	}
#else
	depth = texelFetch(uDepthTex, coord, 0).x*2.0-1.0;
#endif

	float x_proj = vUV.x*2.0 -1.0;
	float y_proj = vUV.y*2.0 -1.0;
	vec4 pos_view = uInvProjMat * vec4(x_proj, y_proj, depth, 1.0);
	pos_view *= 1.0/pos_view.w;
	float dis = length(pos_view.xyz);

	vec3 dir = (uInvViewMat * vec4(pos_view.xyz/dis, 0.0)).xyz;		
	float step = dis/float(max_num_steps)*8.0;
	if (step<min_step*8.0) step = min_step*8.0;

	float step_alpha =  1.0 - pow(1.0 - fog_rgba.w, step);

	uint seed = InitRandomSeed(uint(vUV.x*100000.0), uint(vUV.y*100000.0));
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
	outColor = vec4(col, 0.0);
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


SimpleFogRayMarchingEnv::SimpleFogRayMarchingEnv(const Options& options) : m_options(options)
{
	std::string s_frag = g_frag_part0 + g_frag_part1;

	std::string defines = "";
	if (options.msaa)
	{
		defines += "#define MSAA 1\n";
	}
	else
	{
		defines += "#define MSAA 0\n";
	}

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

	replace(s_frag, "#DEFINES#", defines.c_str());

	GLShader vert_shader(GL_VERTEX_SHADER, g_vertex.c_str());
	GLShader frag_shader(GL_FRAGMENT_SHADER, s_frag.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(vert_shader, frag_shader));
}

void SimpleFogRayMarchingEnv::render(const RenderParams& params)
{
	glDisable(GL_CULL_FACE);

	glUseProgram(m_prog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.constant_camera->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.constant_fog->m_id);

	if (m_options.has_probe_grid)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, params.lights->probe_grid->m_constant.m_id);
		if (params.lights->probe_grid->m_probe_bufs[0] != nullptr)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, params.lights->probe_grid->m_probe_bufs[0]->m_id);
		}
		if (m_options.probe_reference_recorded)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, params.lights->probe_grid->m_ref_buf->m_id);
		}
	}

	if (m_options.has_lod_probe_grid)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, params.lights->lod_probe_grid->m_constant.m_id);
		if (params.lights->lod_probe_grid->m_probe_bufs[0] != nullptr)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, params.lights->lod_probe_grid->m_probe_bufs[0]->m_id);
		}
		if (params.lights->lod_probe_grid->m_probe_bufs[1] != nullptr)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, params.lights->lod_probe_grid->m_probe_bufs[1]->m_id);
		}
		if (params.lights->lod_probe_grid->m_sub_index_buf != nullptr)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, params.lights->lod_probe_grid->m_sub_index_buf->m_id);
		}
	}

	glActiveTexture(GL_TEXTURE0);
	if (m_options.msaa)
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, params.tex_depth->tex_id);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, params.tex_depth->tex_id);
	}
	glUniform1i(0, 0);

	if (m_options.has_probe_grid)
	{
		if (params.lights->probe_grid->m_tex_visibility != nullptr)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, params.lights->probe_grid->m_tex_visibility->tex_id);
			glUniform1i(1, 1);
		}
	}

	if (m_options.has_lod_probe_grid)
	{
		if (params.lights->lod_probe_grid->m_tex_visibility != nullptr)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, params.lights->lod_probe_grid->m_tex_visibility->tex_id);
			glUniform1i(1, 1);
		}
	}

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glUseProgram(0);

}