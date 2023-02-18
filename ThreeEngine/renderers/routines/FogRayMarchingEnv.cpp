#include <string>
#include <GL/glew.h>
#include "FogRayMarchingEnv.h"

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


static std::string g_frag =
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
	float uDiffuseThresh;
	float uDiffuseHigh;
	float uDiffuseLow;
	float uSpecularThresh;
	float uSpecularHigh;
	float uSpecularLow;
};

layout (std430, binding = 3) buffer Probes
{
	vec4 bSHCoefficients[];
};


layout (std430, binding = 4) buffer ProbeVisibility
{
	float bProbeVisibility[];
};

#if PROBE_REFERENCE_RECORDED
layout (std430, binding = 5) buffer ProbeReferences
{
	uint bReferenced[];
};
#endif

float quantize_vis(float limit, float dis)
{
	float x = (dis-0.9*limit)/(0.1*limit);
	if (x<0.0) x = 0.0;
	return pow(0.01, x);
}

float get_visibility(in vec3 pos_world, in ivec3 vert, in vec3 vert_world)
{
	vec3 size_grid = uCoverageMax.xyz - uCoverageMin.xyz;
	vec3 spacing = size_grid/vec3(uDivisions);

	float y0 = pow((float(vert.y) + 0.5f) / float(uDivisions.y), uYpower);
	float y1 = pow((float(vert.y+1) + 0.5f) / float(uDivisions.y), uYpower);
	spacing.y = (y1-y0)*size_grid.y;

	float len_xyz = length(spacing);
	float len_xy = length(vec2(spacing.x, spacing.y));
	float len_yz = length(vec2(spacing.y, spacing.z));
	float len_zx = length(vec2(spacing.z, spacing.x));
	int idx = vert.x + (vert.y + vert.z*uDivisions.y)*uDivisions.x;
	int offset = idx*26;
	vec3 dir = pos_world - vert_world;	
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

	float limit = 0.0;
	if (major_dir == 0)
	{
		dir_abs/=dir_abs.x;
		if (dir.x<0)
		{
			float dis0 = bProbeVisibility[offset];
			if (dir.y<0)
			{
				float dis14 = bProbeVisibility[offset+14] * spacing.x/len_xy;
				float a = (1.0 - dir_abs.y) * dis0 + dir_abs.y *dis14;
				float b = 0.0;
				if (dir.z<0)
				{
					float dis10 = bProbeVisibility[offset+10] * spacing.x/len_zx;
					float dis18 = bProbeVisibility[offset+18] * spacing.x/len_xyz;
					b = (1.0 - dir_abs.y) * dis10 + dir_abs.y *dis18;
				}
				else
				{
					float dis11 = bProbeVisibility[offset+11] * spacing.x/len_zx;
					float dis22 = bProbeVisibility[offset+22] * spacing.x/len_xyz;
					b = (1.0 - dir_abs.y) * dis11 + dir_abs.y *dis22;					
				}
				limit = (1.0 - dir_abs.z) * a + dir_abs.z * b;
			}
			else
			{
				float dis16 = bProbeVisibility[offset+16] * spacing.x/len_xy;
				float a = (1.0 - dir_abs.y) * dis0 + dir_abs.y *dis16;
				float b = 0.0;
				if (dir.z<0)
				{
					float dis10 = bProbeVisibility[offset+10] * spacing.x/len_zx;
					float dis20 = bProbeVisibility[offset+20] * spacing.x/len_xyz;
					b = (1.0 - dir_abs.y) * dis10 + dir_abs.y *dis20;					
				}
				else
				{
					float dis11 = bProbeVisibility[offset+11] * spacing.x/len_zx;
					float dis24 = bProbeVisibility[offset+24] * spacing.x/len_xyz;
					b = (1.0 - dir_abs.y) * dis11 + dir_abs.y *dis24;
				}
				limit = (1.0 - dir_abs.z) * a + dir_abs.z * b;
			}			
		}
		else
		{
			float dis1 = bProbeVisibility[offset + 1];
			if (dir.y<0)
			{
				float dis15 = bProbeVisibility[offset+15] * spacing.x/len_xy;
				float a = (1.0 - dir_abs.y) * dis1 + dir_abs.y *dis15;
				float b = 0.0;
				if (dir.z<0)
				{
					float dis12 = bProbeVisibility[offset+12] * spacing.x/len_zx;
					float dis19 = bProbeVisibility[offset+19] * spacing.x/len_xyz;
					b = (1.0 - dir_abs.y) * dis12 + dir_abs.y *dis19;
				}
				else
				{
					float dis13 = bProbeVisibility[offset+13] * spacing.x/len_zx;
					float dis23 = bProbeVisibility[offset+23] * spacing.x/len_xyz;
					b = (1.0 - dir_abs.y) * dis13 + dir_abs.y *dis23;
				}
				limit = (1.0 - dir_abs.z) * a + dir_abs.z * b;
			}
			else
			{
				float dis17 = bProbeVisibility[offset+17] * spacing.x/len_xy;
				float a = (1.0 - dir_abs.y) * dis1 + dir_abs.y *dis17;
				float b = 0.0;
				if (dir.z<0)
				{
					float dis12 = bProbeVisibility[offset+12] * spacing.x/len_zx;
					float dis21 = bProbeVisibility[offset+21] * spacing.x/len_xyz;
					b = (1.0 - dir_abs.y) * dis12 + dir_abs.y *dis21;
				}
				else
				{
					float dis13 = bProbeVisibility[offset+13] * spacing.x/len_zx;
					float dis25 = bProbeVisibility[offset+25] * spacing.x/len_xyz;
					b = (1.0 - dir_abs.y) * dis13 + dir_abs.y *dis25;
				}
				limit = (1.0 - dir_abs.z) * a + dir_abs.z * b;
			}
		}
		return quantize_vis(limit, abs(dir.x));
	}
	else if (major_dir == 1)
	{
		dir_abs/=dir_abs.y;
		if (dir.y<0)
		{
			float dis2 = bProbeVisibility[offset + 2];
			if (dir.x<0)
			{
				float dis14 = bProbeVisibility[offset+14] * spacing.y/len_xy;
				float a = (1.0 - dir_abs.x) * dis2 + dir_abs.x *dis14;
				float b = 0.0;
				if (dir.z<0)
				{
					float dis6 = bProbeVisibility[offset+6] * spacing.y/len_yz;
					float dis18 = bProbeVisibility[offset+18] * spacing.y/len_xyz;
					b = (1.0 - dir_abs.x) * dis6 + dir_abs.x *dis18;
				}
				else
				{
					float dis8 = bProbeVisibility[offset+8] * spacing.y/len_yz;
					float dis22 = bProbeVisibility[offset+22] * spacing.y/len_xyz;
					b = (1.0 - dir_abs.x) * dis8 + dir_abs.x *dis22;
				}
				limit = (1.0 - dir_abs.z) * a + dir_abs.z * b;
			}
			else
			{
				float dis15 = bProbeVisibility[offset+15] * spacing.y/len_xy;
				float a = (1.0 - dir_abs.x) * dis2 + dir_abs.x *dis15;
				float b = 0.0;
				if (dir.z<0)
				{
					float dis6 = bProbeVisibility[offset+6] * spacing.y/len_yz;
					float dis19 = bProbeVisibility[offset+19] * spacing.y/len_xyz;
					b = (1.0 - dir_abs.x) * dis6 + dir_abs.x *dis19;
				}
				else
				{
					float dis8 = bProbeVisibility[offset+8] * spacing.y/len_yz;
					float dis23 = bProbeVisibility[offset+23] * spacing.y/len_xyz;
					b = (1.0 - dir_abs.x) * dis8 + dir_abs.x *dis23;
				}
				limit = (1.0 - dir_abs.z) * a + dir_abs.z * b;
			}
		}
		else
		{
			float dis3 = bProbeVisibility[offset + 3];
			if (dir.x<0)
			{
				float dis16 = bProbeVisibility[offset+16]*spacing.y/len_xy;
				float a = (1.0 - dir_abs.x) * dis3 + dir_abs.x *dis16;
				float b = 0.0;
				if (dir.z<0)
				{
					float dis7 = bProbeVisibility[offset+7]*spacing.y/len_yz;
					float dis20 = bProbeVisibility[offset+20]*spacing.y/len_xyz;
					b = (1.0 - dir_abs.x) * dis7 + dir_abs.x *dis20;
				}
				else
				{
					float dis9 = bProbeVisibility[offset+9]*spacing.y/len_yz;
					float dis24 = bProbeVisibility[offset+24]*spacing.y/len_xyz;
					b = (1.0 - dir_abs.x) * dis9 + dir_abs.x *dis24;
				}
				limit = (1.0 - dir_abs.z) * a + dir_abs.z * b;
			}
			else
			{
				float dis17 = bProbeVisibility[offset+17];
				float a = (1.0 - dir_abs.x) * dis3 + dir_abs.x *dis17;
				float b = 0.0;
				if (dir.z<0)
				{
					float dis7 = bProbeVisibility[offset+7]*spacing.y/len_yz;
					float dis21 = bProbeVisibility[offset+21]*spacing.y/len_xyz;
					b = (1.0 - dir_abs.x) * dis7 + dir_abs.x *dis21;
				}
				else
				{
					float dis9 = bProbeVisibility[offset+9]*spacing.y/len_yz;
					float dis25 = bProbeVisibility[offset+25]*spacing.y/len_xyz;
					b = (1.0 - dir_abs.x) * dis9 + dir_abs.x *dis25;
				}
				limit = (1.0 - dir_abs.z) * a + dir_abs.z * b;		
			}
		}
		return quantize_vis(limit, abs(dir.y));
	}
	else if (major_dir == 2)
	{
		dir_abs/=dir_abs.z;
		if (dir.z<0)
		{
			float dis4 = bProbeVisibility[offset + 4];
			if (dir.x<0)
			{
				float dis10 = bProbeVisibility[offset+10]*spacing.z/len_zx;
				float a = (1.0 - dir_abs.x) * dis4 + dir_abs.x *dis10;
				float b = 0.0;
				if (dir.y<0)
				{
					float dis6 = bProbeVisibility[offset+6]*spacing.z/len_yz;
					float dis18 = bProbeVisibility[offset+18]*spacing.z/len_xyz;
					b = (1.0 - dir_abs.x) * dis6 + dir_abs.x *dis18;					
				}
				else
				{
					float dis7 = bProbeVisibility[offset+7]*spacing.z/len_yz;
					float dis20 = bProbeVisibility[offset+20]*spacing.z/len_xyz;
					b = (1.0 - dir_abs.x) * dis7 + dir_abs.x *dis20;
				}
				limit = (1.0 - dir_abs.y) * a + dir_abs.y * b;
			}
			else
			{
				float dis12 = bProbeVisibility[offset+12]*spacing.z/len_zx;
				float a = (1.0 - dir_abs.x) * dis4 + dir_abs.x *dis12;
				float b = 0.0;
				if (dir.y<0)
				{
					float dis6 = bProbeVisibility[offset+6]*spacing.z/len_yz;
					float dis19 = bProbeVisibility[offset+19]*spacing.z/len_xyz;
					b = (1.0 - dir_abs.x) * dis6 + dir_abs.x *dis19;
				}
				else
				{
					float dis7 = bProbeVisibility[offset+7]*spacing.z/len_yz;
					float dis21 = bProbeVisibility[offset+21]*spacing.z/len_xyz;
					b = (1.0 - dir_abs.x) * dis7 + dir_abs.x *dis21;
				}
				limit = (1.0 - dir_abs.y) * a + dir_abs.y * b;
			}
		}		
		else
		{
			float dis5 = bProbeVisibility[offset + 5];
			if (dir.x<0)
			{
				float dis11 = bProbeVisibility[offset+11]*spacing.z/len_zx;
				float a = (1.0 - dir_abs.x) * dis5 + dir_abs.x *dis11;
				float b = 0.0;
				if (dir.y<0)
				{
					float dis8 = bProbeVisibility[offset+8]*spacing.z/len_yz;
					float dis22 = bProbeVisibility[offset+22]*spacing.z/len_xyz;
					b = (1.0 - dir_abs.x) * dis8 + dir_abs.x *dis22;
				}
				else
				{
					float dis9 = bProbeVisibility[offset+9]*spacing.z/len_yz;
					float dis24 = bProbeVisibility[offset+24]*spacing.z/len_xyz;
					b = (1.0 - dir_abs.x) * dis9 + dir_abs.x *dis24;
				}
				limit = (1.0 - dir_abs.y) * a + dir_abs.y * b;
			}
			else
			{
				float dis13 = bProbeVisibility[offset+13]*spacing.z/len_zx;
				float a = (1.0 - dir_abs.x) * dis5 + dir_abs.x *dis13;
				float b = 0.0;
				if (dir.y<0)
				{
					float dis8 = bProbeVisibility[offset+8]*spacing.z/len_yz;
					float dis23 = bProbeVisibility[offset+23]*spacing.z/len_xyz;
					b = (1.0 - dir_abs.x) * dis8 + dir_abs.x *dis23;
				}
				else
				{
					float dis9 = bProbeVisibility[offset+9]*spacing.z/len_yz;
					float dis25 = bProbeVisibility[offset+25]*spacing.z/len_xyz;
					b = (1.0 - dir_abs.x) * dis9 + dir_abs.x *dis25;
				}
				limit = (1.0 - dir_abs.y) * a + dir_abs.y * b;
			}
		}
		return quantize_vis(limit, abs(dir.z));
	}
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
				vec3 w = vec3(1.0) - abs(vec3(x,y,z) - frac_voxel);
				float weight = w.x * w.y * w.z;
				if (weight>0.0)
				{
					ivec3 vert = i_voxel + ivec3(x,y,z);
					vec3 vert_normalized = (vec3(vert) + vec3(0.5))/vec3(uDivisions);
					vert_normalized.y = pow(vert_normalized.y, uYpower); 
					vec3 vert_world = vert_normalized * size_grid + uCoverageMin.xyz;
					weight*= get_visibility(pos_world, vert,vert_world);
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
	float step = dis/float(max_num_steps)*4.0;
	if (step<min_step*4.0) step = min_step*4.0;

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


FogRayMarchingEnv::FogRayMarchingEnv(const Options& options) : m_options(options)
{
	std::string s_frag = g_frag;

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

	replace(s_frag, "#DEFINES#", defines.c_str());

	GLShader vert_shader(GL_VERTEX_SHADER, g_vertex.c_str());
	GLShader frag_shader(GL_FRAGMENT_SHADER, s_frag.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(vert_shader, frag_shader));
}

void FogRayMarchingEnv::render(const RenderParams& params)
{
	glDisable(GL_CULL_FACE);

	glUseProgram(m_prog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.constant_camera->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.constant_fog->m_id);	

	if (m_options.has_probe_grid)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, params.lights->probe_grid->m_constant.m_id);
		if (params.lights->probe_grid->m_probe_buf != nullptr)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, params.lights->probe_grid->m_probe_buf->m_id);
		}
		if (params.lights->probe_grid->m_visibility_buf != nullptr)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, params.lights->probe_grid->m_visibility_buf->m_id);
		}
		if (m_options.probe_reference_recorded)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, params.lights->probe_grid->m_ref_buf->m_id);
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

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glUseProgram(0);

}