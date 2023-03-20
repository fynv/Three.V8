#include <GL/glew.h>
#include "VisibilityUpdate.h"
#include "renderers/BVHRenderTarget.h"
#include "lights/ProbeRayList.h"
#include "lights/ProbeGrid.h"
#include "lights/LODProbeGrid.h"
#include "renderers/ProbeRenderTarget.h"

static std::string g_compute =
R"(#version 430

#DEFINES#

layout (location = 0) uniform sampler2D uTexSource;

layout (std140, binding = 0) uniform ProbeRayList
{
	mat4 uPRLRotation;
	int uRPLNumProbes;
	int uPRLNumDirections;	
};

layout (binding=0, rg16f) uniform image2D uImgOut;

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

#elif HAS_LOD_PROBE_GRID

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

#endif

layout (location = 1) uniform int uIDStartProbe;
layout (location = 2) uniform float uMixRate;


vec2 signNotZero(in vec2 v)
{
	return vec2((v.x >= 0.0) ? 1.0 : -1.0, (v.y >= 0.0)? 1.0: -1.0);
}


vec3 oct_to_vec3(in vec2 e)
{
	vec3 v = vec3(vec2(e.x, e.y), 1.0 - abs(e.x) - abs(e.y));
	if (v.z < 0.0)
	{
		vec2 tmp = (1.0 - abs(vec2(v.y, v.x))) * signNotZero(vec2(v.x, v.y));
		v.x = tmp.x;
		v.y = tmp.y;
	}
	return normalize(v);
}


#define PI 3.14159265359

vec3 sphericalFibonacci(float i, float n)
{
	const float PHI = sqrt(5.0) * 0.5 + 0.5;
	float m = i * (PHI - 1.0);
	float frac_m = m - floor(m);
	float phi = 2.0 *  PI * frac_m;
	float cosTheta = 1.0 - (2.0 * i + 1.0) * (1.0 / n);
    float sinTheta = sqrt(clamp(1.0 - cosTheta * cosTheta, 0.0, 1.0));
	return vec3(cos(phi)*sinTheta, sin(phi)*sinTheta, cosTheta);	
}

layout(local_size_x = 64) in;

void main()
{
	ivec2 local_id = ivec3(gl_LocalInvocationID).xy;	
	ivec2 group_id = ivec3(gl_WorkGroupID).xy;
	int probe_id_in = group_id.y;
	int probe_id_out = probe_id_in + uIDStartProbe;
	int pixel_id = local_id.x + group_id.x *  64;	

	ivec2 coord_probe = ivec2(pixel_id % uVisRes, pixel_id / uVisRes);
	vec2 probe_uv = (vec2(coord_probe) + 0.5)/vec2(uVisRes);
	vec3 out_dir = oct_to_vec3(probe_uv*2.0 - 1.0);

	float acc_dis = 0.0;
	float acc_sqr_dis = 0.0;
	float acc_weight = 0.0;	

	for (int ray_id = 0; ray_id < uPRLNumDirections; ray_id++)
	{		
		vec3 sf = sphericalFibonacci(ray_id, uPRLNumDirections);
		vec3 in_dir = vec3(uPRLRotation * vec4(sf, 0.0));
		
		float weight = pow(max(0.0, dot(in_dir, out_dir)), 50.0);
		if (weight > 1e-6)
		{
			ivec2 id_in = ivec2(ray_id, probe_id_in);
			float dis = texelFetch(uTexSource, id_in, 0).x;
			if (dis > 65500.0) continue;
			acc_dis += weight * dis;
			acc_sqr_dis += weight * dis * dis;
			acc_weight += weight;
		}
	}

	float mean_dis, mean_var;
	if (acc_weight > 0.0)
	{
		mean_dis = acc_dis/acc_weight;
		float mean_sqr_dis = acc_sqr_dis/acc_weight;
		mean_var = sqrt(mean_sqr_dis - mean_dis * mean_dis);
	}
	else
	{
		mean_dis = 65504.0;
		mean_var = 0.0;
	}
	
	int pack_x = probe_id_out % uPackSize;
	int pack_y = probe_id_out / uPackSize;
	ivec2 coord_out = ivec2(pack_x, pack_y) * (uVisRes + 2) + coord_probe + 1;

	if (uMixRate<1.0)
	{
		vec2 last = imageLoad(uImgOut, coord_out).xy;
		mean_dis = uMixRate * mean_dis + (1.0 - uMixRate) * last.x;
		mean_var = uMixRate * mean_var + (1.0 - uMixRate) * last.y;
	}

	mean_dis = clamp(mean_dis, 0.0, 65504.0);
	mean_var = clamp(mean_var, 0.0, 65504.0);

	imageStore(uImgOut, coord_out, vec4(mean_dis, mean_var, mean_dis, mean_var));

	if (coord_probe.x == 0)
	{
		ivec2 coord_probe2 = ivec2(-1, uVisRes-1 - coord_probe.y);
		ivec2 coord_out2 = ivec2(pack_x, pack_y) * (uVisRes + 2) + coord_probe2 + 1;
		imageStore(uImgOut, coord_out2, vec4(mean_dis, mean_var, mean_dis, mean_var));

		if (coord_probe.y == 0)
		{
			coord_probe2 = ivec2(uVisRes, uVisRes);
			coord_out2 = ivec2(pack_x, pack_y) * (uVisRes + 2) + coord_probe2 + 1;
			imageStore(uImgOut, coord_out2, vec4(mean_dis, mean_var, mean_dis, mean_var));
		}
		else if (coord_probe.y == uVisRes-1)
		{
			coord_probe2 = ivec2(uVisRes, -1);
			coord_out2 = ivec2(pack_x, pack_y) * (uVisRes + 2) + coord_probe2 + 1;
			imageStore(uImgOut, coord_out2, vec4(mean_dis, mean_var, mean_dis, mean_var));
		}
	}
	else if (coord_probe.x == uVisRes-1)
	{
		ivec2 coord_probe2 = ivec2(uVisRes, uVisRes-1 - coord_probe.y);
		ivec2 coord_out2 = ivec2(pack_x, pack_y) * (uVisRes + 2) + coord_probe2 + 1;
		imageStore(uImgOut, coord_out2, vec4(mean_dis, mean_var, mean_dis, mean_var));
		
		if (coord_probe.y == 0)
		{
			coord_probe2 = ivec2(-1, uVisRes);
			coord_out2 = ivec2(pack_x, pack_y) * (uVisRes + 2) + coord_probe2 + 1;
			imageStore(uImgOut, coord_out2, vec4(mean_dis, mean_var, mean_dis, mean_var));
		}
		else if (coord_probe.y == uVisRes-1)
		{
			coord_probe2 = ivec2(-1, -1);
			coord_out2 = ivec2(pack_x, pack_y) * (uVisRes + 2) + coord_probe2 + 1;
			imageStore(uImgOut, coord_out2, vec4(mean_dis, mean_var, mean_dis, mean_var));
		}
	}
	
	if (coord_probe.y == 0)
	{
		ivec2 coord_probe2 = ivec2(uVisRes-1 - coord_probe.x, -1);
		ivec2 coord_out2 = ivec2(pack_x, pack_y) * (uVisRes + 2) + coord_probe2 + 1;
		imageStore(uImgOut, coord_out2, vec4(mean_dis, mean_var, mean_dis, mean_var));
	}
	else if (coord_probe.y == uVisRes-1)
	{
		ivec2 coord_probe2 = ivec2(uVisRes-1 - coord_probe.x, uVisRes);
		ivec2 coord_out2 = ivec2(pack_x, pack_y) * (uVisRes + 2) + coord_probe2 + 1;
		imageStore(uImgOut, coord_out2, vec4(mean_dis, mean_var, mean_dis, mean_var));
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

VisibilityUpdate::VisibilityUpdate(bool is_lod_probe_grid) : m_is_lod_probe_grid(is_lod_probe_grid)
{
	std::string s_compute = g_compute;

	std::string defines = "";
	if (is_lod_probe_grid)
	{
		defines += "#define HAS_PROBE_GRID 0\n";
		defines += "#define HAS_LOD_PROBE_GRID 1\n";
	}
	else
	{
		defines += "#define HAS_PROBE_GRID 1\n";
		defines += "#define HAS_LOD_PROBE_GRID 0\n";
	}

	replace(s_compute, "#DEFINES#", defines.c_str());

	GLShader comp_shader(GL_COMPUTE_SHADER, s_compute.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
}


void VisibilityUpdate::update(const RenderParams& params)
{
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	const BVHRenderTarget* source = params.source;

	const ProbeGrid* probe_grid = nullptr;
	const LODProbeGrid* lod_probe_grid = nullptr;

	int width;
	if (m_is_lod_probe_grid)
	{
		lod_probe_grid = params.lod_probe_grid;
		width = lod_probe_grid->vis_res * lod_probe_grid->vis_res;
	}
	else
	{
		probe_grid = params.probe_grid;
		width = probe_grid->vis_res * probe_grid->vis_res;
	}
	int height = source->m_height;

	unsigned id_target;
	if (params.target != nullptr)
	{
		id_target = params.target->m_tex_visibility->tex_id;
	}
	else if (m_is_lod_probe_grid)
	{
		id_target = lod_probe_grid->m_tex_visibility->tex_id;
	}
	else
	{
		id_target = probe_grid->m_tex_visibility->tex_id;
	}

	glUseProgram(m_prog->m_id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, source->m_tex_depth->tex_id);
	glUniform1i(0,0);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.prl->m_constant.m_id);

	glBindImageTexture(0, id_target, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RG16F);

	if (m_is_lod_probe_grid)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, lod_probe_grid->m_constant.m_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lod_probe_grid->m_probe_buf->m_id);
	}
	else
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, probe_grid->m_constant.m_id);
	}

	glUniform1i(1, params.id_start_probe);
	glUniform1f(2, params.mix_rate);

	glm::ivec2 blocks = { (width + 63) / 64, height };
	glDispatchCompute(blocks.x, blocks.y, 1);

	glUseProgram(0);


}
