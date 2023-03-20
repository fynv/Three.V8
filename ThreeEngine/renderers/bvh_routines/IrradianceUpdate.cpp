#include <GL/glew.h>
#include "IrradianceUpdate.h"
#include "renderers/BVHRenderTarget.h"
#include "lights/ProbeRayList.h"
#include "lights/ProbeGrid.h"
#include "lights/LODProbeGrid.h"
#include "renderers/ProbeRenderTarget.h"

static std::string g_compute_sh =
R"(#version 430

#DEFINES#

layout (location = 0) uniform sampler2D uTexSource;
layout (location = 1) uniform sampler2D uTexSHIrrWeight;

layout (std140, binding = 0) uniform ProbeRayList
{
	mat4 uPRLRotation;
	int uRPLNumProbes;
	int uPRLNumDirections;	
};


layout (std430, binding = 1) buffer Probes
{
	vec4 bProbeData[];
};

layout (location = 2) uniform int uIDStartProbe;
layout (location = 3) uniform float uMixRate;


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
	int id = ivec3(gl_GlobalInvocationID).x;
	if (id >= uRPLNumProbes * 9) return;
	int coeff_id = id % 9;
	int probe_id_in = id / 9;
	int probe_id_out = probe_id_in + uIDStartProbe;

	vec3 coeff = vec3(0.0);
	for (int ray_id = 0; ray_id < uPRLNumDirections; ray_id++)
	{
		vec3 sf = sphericalFibonacci(ray_id, uPRLNumDirections);
		vec3 in_dir = vec3(uPRLRotation * vec4(sf, 0.0));		
		vec3 col = texelFetch(uTexSource, ivec2(ray_id, probe_id_in), 0).xyz;
		float weight = texelFetch(uTexSHIrrWeight, ivec2(ray_id, coeff_id), 0).x;
		coeff += col * weight;
	}	
	
#if MODE == 9
	if (uMixRate<1.0)
	{
		vec3 last = bProbeData[probe_id_out * 9 + coeff_id].xyz;
		coeff = uMixRate * coeff + (1.0 - uMixRate)*last;
	}
	bProbeData[probe_id_out * 9 + coeff_id] = vec4(coeff, 1.0);
#elif MODE == 10
	if (uMixRate<1.0)
	{
		vec3 last = bProbeData[probe_id_out * 10 + 1 + coeff_id].xyz;
		coeff = uMixRate * coeff + (1.0 - uMixRate)*last;
	}
	bProbeData[probe_id_out * 10 + 1 + coeff_id] = vec4(coeff, 1.0);
#endif
}
)";

static std::string g_compute_irr =
R"(#version 430

#DEFINES#

layout (std140, binding = 0) uniform ProbeRayList
{
	mat4 uPRLRotation;
	int uRPLNumProbes;
	int uPRLNumDirections;
};

layout (std430, binding = 1) buffer Probes
{
	vec4 bProbeData[];
};

layout (binding=0, r11f_g11f_b10f) uniform image2D uImgOut;

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

#elif HAS_LOD_PROBE_GRID

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

#endif

layout (location = 0) uniform int uIDStartProbe;

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

vec3 shGetIrradianceAt( in vec3 normal, in vec4 shCoefficients[ 9 ] ) 
{
	// normal is assumed to have unit length

	float x = normal.x, y = normal.y, z = normal.z;

	// band 0
	vec3 result = shCoefficients[ 0 ].xyz * 0.886227;

	// band 1
	result += shCoefficients[ 1 ].xyz * 2.0 * 0.511664 * y;
	result += shCoefficients[ 2 ].xyz * 2.0 * 0.511664 * z;
	result += shCoefficients[ 3 ].xyz * 2.0 * 0.511664 * x;

	// band 2
	result += shCoefficients[ 4 ].xyz * 2.0 * 0.429043 * x * y;
	result += shCoefficients[ 5 ].xyz * 2.0 * 0.429043 * y * z;
	result += shCoefficients[ 6 ].xyz * ( 0.743125 * z * z - 0.247708 );
	result += shCoefficients[ 7 ].xyz * 2.0 * 0.429043 * x * z;
	result += shCoefficients[ 8 ].xyz * 0.429043 * ( x * x - y * y );

	return result;
}


layout(local_size_x = 64) in;

void main()
{
	ivec2 local_id = ivec3(gl_LocalInvocationID).xy;	
	ivec2 group_id = ivec3(gl_WorkGroupID).xy;	
	int probe_id =  group_id.y + uIDStartProbe;
	int pixel_id = local_id.x + group_id.x *  64;

	ivec2 coord_probe = ivec2(pixel_id % uIrrRes, pixel_id / uIrrRes);
	vec2 probe_uv = (vec2(coord_probe) + 0.5)/vec2(uIrrRes);
	vec3 dir = oct_to_vec3(probe_uv*2.0 - 1.0);	

	vec4 shCoefficients[9];
	
#if MODE == 9
	for (int i=0; i<9; i++)
	{
		shCoefficients[i] = bProbeData[probe_id * 9 + i];
	}
#elif MODE == 10
	for (int i=0; i<9; i++)
	{
		shCoefficients[i] = bProbeData[probe_id * 10 + 1 + i];
	}
#endif
	vec3 irr = shGetIrradianceAt(dir, shCoefficients);

	int pack_x = probe_id % uPackSize;
	int pack_y = probe_id / uPackSize;
	ivec2 coord_out = ivec2(pack_x, pack_y) * (uIrrRes + 2) + coord_probe + 1;

	imageStore(uImgOut, coord_out, vec4(irr, 1.0));	

	if (coord_probe.x == 0)
	{
		ivec2 coord_probe2 = ivec2(-1, uIrrRes-1 - coord_probe.y);
		ivec2 coord_out2 = ivec2(pack_x, pack_y) * (uIrrRes + 2) + coord_probe2 + 1;
		imageStore(uImgOut, coord_out2, vec4(irr, 1.0));

		if (coord_probe.y == 0)
		{
			coord_probe2 = ivec2(uIrrRes, uIrrRes);
			coord_out2 = ivec2(pack_x, pack_y) * (uIrrRes + 2) + coord_probe2 + 1;
			imageStore(uImgOut, coord_out2, vec4(irr, 1.0));
		}
		else if (coord_probe.y == uIrrRes-1)
		{
			coord_probe2 = ivec2(uIrrRes, -1);
			coord_out2 = ivec2(pack_x, pack_y) * (uIrrRes + 2) + coord_probe2 + 1;
			imageStore(uImgOut, coord_out2, vec4(irr, 1.0));
		}
	}
	else if (coord_probe.x == uIrrRes-1)
	{
		ivec2 coord_probe2 = ivec2(uIrrRes, uIrrRes-1 - coord_probe.y);
		ivec2 coord_out2 = ivec2(pack_x, pack_y) * (uIrrRes + 2) + coord_probe2 + 1;
		imageStore(uImgOut, coord_out2, vec4(irr, 1.0));
		
		if (coord_probe.y == 0)
		{
			coord_probe2 = ivec2(-1, uIrrRes);
			coord_out2 = ivec2(pack_x, pack_y) * (uIrrRes + 2) + coord_probe2 + 1;
			imageStore(uImgOut, coord_out2, vec4(irr, 1.0));
		}
		else if (coord_probe.y == uIrrRes-1)
		{
			coord_probe2 = ivec2(-1, -1);
			coord_out2 = ivec2(pack_x, pack_y) * (uIrrRes + 2) + coord_probe2 + 1;
			imageStore(uImgOut, coord_out2, vec4(irr, 1.0));
		}
	}
	
	if (coord_probe.y == 0)
	{
		ivec2 coord_probe2 = ivec2(uIrrRes-1 - coord_probe.x, -1);
		ivec2 coord_out2 = ivec2(pack_x, pack_y) * (uIrrRes + 2) + coord_probe2 + 1;
		imageStore(uImgOut, coord_out2, vec4(irr, 1.0));
	}
	else if (coord_probe.y == uIrrRes-1)
	{
		ivec2 coord_probe2 = ivec2(uIrrRes-1 - coord_probe.x, uIrrRes);
		ivec2 coord_out2 = ivec2(pack_x, pack_y) * (uIrrRes + 2) + coord_probe2 + 1;
		imageStore(uImgOut, coord_out2, vec4(irr, 1.0));
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


IrradianceUpdate::IrradianceUpdate(bool is_lod_probe_grid, bool use_target)
	: m_is_lod_probe_grid(is_lod_probe_grid)
	, m_use_target(use_target)
{
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
	
	if (m_use_target || !is_lod_probe_grid)
	{
		defines += "#define MODE 9\n";
	}
	else
	{
		defines += "#define MODE 10\n";
	}

	{
		std::string s_compute = g_compute_sh;

		replace(s_compute, "#DEFINES#", defines.c_str());

		GLShader comp_shader(GL_COMPUTE_SHADER, s_compute.c_str());
		m_prog_sh = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
	}

	{
		std::string s_compute = g_compute_irr;

		replace(s_compute, "#DEFINES#", defines.c_str());

		GLShader comp_shader(GL_COMPUTE_SHADER, s_compute.c_str());
		m_prog_irr = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
	}
}


void IrradianceUpdate::update(const RenderParams& params)
{
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	const BVHRenderTarget* source = params.source;

	glUseProgram(m_prog_sh->m_id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, source->m_tex_video->tex_id);
	glUniform1i(0, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, params.prl->TexSHIrrWeight->tex_id);
	glUniform1i(1, 1);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.prl->m_constant.m_id);

	if (params.target != nullptr)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, params.target->m_probe_buf->m_id);
	}
	else if (m_is_lod_probe_grid)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, params.lod_probe_grid->m_probe_buf->m_id);
	}
	else
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, params.probe_grid->m_probe_buf->m_id);
	}

	glUniform1i(2, params.id_start_probe);
	glUniform1f(3, params.mix_rate);

	{
		int blocks = (params.prl->num_probes *9 + 63) / 64;
		glDispatchCompute(blocks, 1, 1);
	}	

	glUseProgram(0);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	const ProbeGrid* probe_grid = nullptr;
	const LODProbeGrid* lod_probe_grid = nullptr;

	int width;
	if (m_is_lod_probe_grid)
	{
		lod_probe_grid = params.lod_probe_grid;
		width = lod_probe_grid->irr_res * lod_probe_grid->irr_res;
	}
	else
	{
		probe_grid = params.probe_grid;
		width = probe_grid->irr_res * probe_grid->irr_res;
	}
	int height = source->m_height;

	unsigned id_target;
	if (params.target != nullptr)
	{
		id_target = params.target->m_tex_irradiance->tex_id;
	}
	else if (m_is_lod_probe_grid)
	{
		id_target = lod_probe_grid->m_tex_irradiance->tex_id;
	}
	else
	{
		id_target = probe_grid->m_tex_irradiance->tex_id;
	}

	glUseProgram(m_prog_irr->m_id);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.prl->m_constant.m_id);

	if (params.target != nullptr)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, params.target->m_probe_buf->m_id);
	}
	else if (m_is_lod_probe_grid)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, params.lod_probe_grid->m_probe_buf->m_id);
	}
	else
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, params.probe_grid->m_probe_buf->m_id);
	}

	glBindImageTexture(0, id_target, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R11F_G11F_B10F);

	if (m_is_lod_probe_grid)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, lod_probe_grid->m_constant.m_id);		
	}
	else
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, probe_grid->m_constant.m_id);
	}

	glUniform1i(0, params.id_start_probe);

	{
		glm::ivec2 blocks = { (width + 63) / 64, height };		
		glDispatchCompute(blocks.x, blocks.y, 1);
	}

	glUseProgram(0);

}
