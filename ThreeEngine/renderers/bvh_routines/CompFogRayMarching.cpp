#include <string>
#include <GL/glew.h>
#include "renderers/BVHRenderTarget.h"
#include "cameras/Camera.h"
#include "scenes/Fog.h"
#include "CompFogRayMarching.h"
#include "lights/ProbeRayList.h"
#include "renderers/LightmapRayList.h"

static std::string g_compute =
R"(#version 430

#DEFINES#

layout (std140, binding = 0) uniform Fog
{
	vec4 fog_rgba;
	int max_num_steps;
	float min_step;
};

layout (std140, binding = 1) uniform DirectionalLight
{
	vec4 light_color;
	vec4 light_origin;
	vec4 light_direction;
};

layout (location = 0) uniform sampler2D uDepthTex;

#define PI 3.14159265359
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

layout (binding=0, rgba16f) uniform image2D uImgColor;

layout(local_size_x = 8, local_size_y = 8) in;

ivec2 g_id_io;
vec3 g_origin;
vec3 g_dir;

void render()
{
	float dis = texelFetch(uDepthTex, g_id_io, 0).x;

	float step = dis/float(max_num_steps)*2.0;
	if (step<min_step*2.0) step = min_step*2.0;

	float step_alpha =  1.0 - pow(1.0 - fog_rgba.w, step);

	uint seed = InitRandomSeed(uint(g_id_io.x), uint(g_id_io.y));
	float delta = RandomFloat(seed);

	vec3 col = vec3(0.0);
	float start = step * (delta - 0.5);
	for (float t = start; t<dis; t+=step)
	{
		float _step = min(step, dis - t);
		float _step_alpha = (_step == step) ? step_alpha :  (1.0 - pow(1.0 - fog_rgba.w, _step));
		float sample_t = max(t + _step*0.5, 0.0);

		vec3 pos_world = g_origin + g_dir * sample_t;
		float l_shadow = 1.0;
		float zEye = -dot(pos_world - light_origin.xyz, light_direction.xyz);
		if (zEye>0.0)
		{
			float att = pow(1.0 - fog_rgba.w, zEye);
			l_shadow *= att;
		}		
		float att =  pow(1.0 - fog_rgba.w, sample_t);

		// About 0.25
		// consider partical shape as sphere, 0.25 is the average max(dot(N, L),0);
		vec3 irradiance = light_color.xyz * l_shadow * 0.25;

		col+=fog_rgba.xyz*irradiance*RECIPROCAL_PI * _step_alpha* att;
	}
	vec4 base = imageLoad(uImgColor, g_id_io);
	imageStore(uImgColor, g_id_io, base + vec4(col, 0.0));
}

#if TO_CAMERA
layout (std140, binding = 2) uniform Camera
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
	
	g_id_io = id;
	g_origin = uEyePos;
	g_dir = dir;
	render();
	
}
#elif TO_PROBES

layout (std140, binding = 2) uniform ProbeRayList
{
	mat4 uPRLRotation;
	int uRPLNumProbes;
	int uPRLNumDirections;
};

layout (std430, binding = 3) buffer ProbePositions
{
	vec4 uProbePos[];
};

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

void main()
{	
	ivec2 local_id = ivec3(gl_LocalInvocationID).xy;	
	ivec2 group_id = ivec3(gl_WorkGroupID).xy;
	int probe_id = group_id.y;
	int ray_id = local_id.x + local_id.y*8 + group_id.x * 64;	
	g_id_io = ivec2(ray_id, probe_id);
	g_origin = uProbePos[probe_id].xyz;

	vec3 sf = sphericalFibonacci(ray_id, uPRLNumDirections);
	vec4 dir = uPRLRotation * vec4(sf, 0.0);
	g_dir = dir.xyz;	
	
	render();
}
#elif TO_LIGHTMAP
layout (std140, binding = 2) uniform LightmapRayList
{
	int uTexelBegin;
	int uTexelEnd;
	int uNumRays;
	int uTexelsPerRow;
	int uNumRows;
	int uJitter;
};

layout (location = 1) uniform sampler2D uTexPosition;
layout (location = 2) uniform sampler2D uTexNormal;
layout (location = 3) uniform usamplerBuffer uValidList;

vec3 RandomDirection(inout uint seed)
{
	float z = RandomFloat(seed) * 2.0 - 1.0;
	float xy = sqrt(1.0 - z*z);
	float alpha = RandomFloat(seed) * PI * 2.0;
	return vec3(xy * cos(alpha), xy * sin(alpha), z);
}

vec3 RandomDiffuse(inout uint seed, in vec3 base_dir)
{
	vec3 dir = RandomDirection(seed);
	float d = dot(dir, base_dir);
	vec3 c = d * base_dir;
	vec3 s = dir - c;
	float z2 = clamp(abs(d), 0.0, 1.0);
	float xy = sqrt(1.0 - z2);	
	vec3 s_dir =  sqrt(z2) * base_dir;
	if (length(s)>0.0)
	{		
		s_dir += xy * normalize(s);
	}
	return s_dir;
}

void main()
{
	ivec2 local_id = ivec3(gl_LocalInvocationID).xy;	
	ivec2 group_id = ivec3(gl_WorkGroupID).xy;
	g_id_io = ivec2(local_id.x + local_id.y * 8 + group_id.x * 64, group_id.y);
	int idx_texel_out = g_id_io.x/uNumRays + g_id_io.y*uTexelsPerRow;	
	int idx_texel_in = idx_texel_out + uTexelBegin;
	if (idx_texel_in >= uTexelEnd) return;

	int idx_ray = g_id_io.x % uNumRays;

	ivec2 texel_coord = ivec2(texelFetch(uValidList, idx_texel_in).xy);	
	g_origin = texelFetch(uTexPosition, texel_coord, 0).xyz;
	vec3 norm = texelFetch(uTexNormal, texel_coord, 0).xyz;
	uint seed = InitRandomSeed(uJitter, idx_texel_out * uNumRays +  idx_ray);
	g_dir = RandomDiffuse(seed, norm);	

	render();
}
#endif
)";

static std::string g_compute_shadow =
R"(#version 430

#DEFINES#

layout (std140, binding = 0) uniform Fog
{
	vec4 fog_rgba;
	int max_num_steps;
	float min_step;
};

layout (std140, binding = 1) uniform DirectionalLight
{
	vec4 light_color;
	vec4 light_origin;
	vec4 light_direction;
};


layout (std140, binding = 2) uniform DirectionalShadow
{
	mat4 VPSBMat;
	mat4 shadowProjMat;
	mat4 shadowViewMat;
    vec2 shadowLeftRight;
	vec2 shadowBottomTop;
	vec2 shadowNearFar;
};

layout (location = 0) uniform sampler2D uDepthTex;
layout (location = 1) uniform sampler2DShadow uShadowTex;


vec3 computeShadowCoords(in mat4 VPSB, in vec3 pos_world)
{
	vec4 shadowCoords = VPSB * vec4(pos_world, 1.0);
	return shadowCoords.xyz;
}

float borderPCFTexture(vec3 uvz)
{
    return ((uvz.x <= 1.0) && (uvz.y <= 1.0) &&
	 (uvz.x >= 0.0) && (uvz.y >= 0.0)) ? texture(uShadowTex, uvz) : 
	 ((uvz.z <= 1.0) ? 1.0 : 0.0);
}

float computeShadowCoef(in mat4 VPSB, in vec3 pos_world)
{
	vec3 shadowCoords;
	shadowCoords = computeShadowCoords(VPSB, pos_world);
	return borderPCFTexture(shadowCoords);
}

#define PI 3.14159265359
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


layout (binding=0, rgba16f) uniform image2D uImgColor;

layout(local_size_x = 8, local_size_y = 8) in;

ivec2 g_id_io;
vec3 g_origin;
vec3 g_dir;

void render()
{
	float dis = texelFetch(uDepthTex, g_id_io, 0).x;

	float start = 0.0;
	float end = dis;
	{
		vec3 origin_light = (shadowViewMat * vec4(g_origin, 1.0)).xyz;
		vec3 dir_light = (shadowViewMat * vec4(g_dir, 0.0)).xyz;

		vec3 min_dis = vec3(0.0);
		vec3 max_dis = vec3(dis);		
		if (dir_light.x!=0.0)
		{
			float dis1 = (shadowLeftRight.x - origin_light.x)/dir_light.x;
			float dis2 = (shadowLeftRight.y - origin_light.x)/dir_light.x;
			min_dis.x = min(dis1, dis2);
			max_dis.x = max(dis1, dis2);			
		}

		if (dir_light.y!=0.0)
		{
			float dis1 = (shadowBottomTop.x - origin_light.y)/dir_light.y;
			float dis2 = (shadowBottomTop.y - origin_light.y)/dir_light.y;
			min_dis.y = min(dis1, dis2);
			max_dis.y = max(dis1, dis2);			
		}		

		if (dir_light.z!=0.0)
		{
			float dis1 = (-shadowNearFar.x - origin_light.z)/dir_light.z;
			float dis2 = (-shadowNearFar.y - origin_light.z)/dir_light.z;
			min_dis.z = min(dis1, dis2);
			max_dis.z = max(dis1, dis2);			
		}		

		start = max(max(0.0, min_dis.x), max(min_dis.y, min_dis.z));
		end = min(min(dis, max_dis.x), min(max_dis.y, max_dis.z));		
	}
	
	float step = (end - start)/float(max_num_steps)*2.0;
	if (step<min_step*2.0) step = min_step*2.0;
	
	float step_alpha =  1.0 - pow(1.0 - fog_rgba.w, step);

	uint seed = InitRandomSeed(uint(g_id_io.x), uint(g_id_io.y));
	float delta = RandomFloat(seed);

	vec3 col = vec3(0.0);
	start += step * (delta - 0.5);
	for (float t = start; t<end; t+=step)
	{
		float _step = min(step, dis - t);
		float _step_alpha = (_step == step) ? step_alpha :  (1.0 - pow(1.0 - fog_rgba.w, _step));
		float sample_t = max(t + _step*0.5, 0.0);

		vec3 pos_world = g_origin + g_dir * sample_t;
		float l_shadow = computeShadowCoef(VPSBMat, pos_world); 
		if (l_shadow>0.0)
		{
			float zEye = -dot(pos_world - light_origin.xyz, light_direction.xyz);
			if (zEye>0.0)
			{
				float att = pow(1.0 - fog_rgba.w, zEye);
				l_shadow *= att;
			}
			float att =  pow(1.0 - fog_rgba.w, sample_t);
			vec3 irradiance = light_color.xyz * l_shadow * 0.25;

			col+=fog_rgba.xyz*irradiance*RECIPROCAL_PI * _step_alpha* att;
		}		
	}
	vec4 base = imageLoad(uImgColor, g_id_io);
	imageStore(uImgColor, g_id_io, base + vec4(col, 0.0));

}

#if TO_CAMERA

layout (std140, binding = 3) uniform Camera
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

	g_id_io = id;
	g_origin = uEyePos;
	g_dir = dir;
	render();	
}
#elif TO_PROBES


layout (std140, binding = 3) uniform ProbeRayList
{
	mat4 uPRLRotation;
	int uRPLNumProbes;
	int uPRLNumDirections;
};

layout (std430, binding = 4) buffer ProbePositions
{
	vec4 uProbePos[];
};

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

void main()
{	
	ivec2 local_id = ivec3(gl_LocalInvocationID).xy;	
	ivec2 group_id = ivec3(gl_WorkGroupID).xy;
	int probe_id = group_id.y;
	int ray_id = local_id.x + local_id.y*8 + group_id.x * 64;	
	g_id_io = ivec2(ray_id, probe_id);
	g_origin = uProbePos[probe_id].xyz;

	vec3 sf = sphericalFibonacci(ray_id, uPRLNumDirections);
	vec4 dir = uPRLRotation * vec4(sf, 0.0);
	g_dir = dir.xyz;	
	
	render();
}
layout (std140, binding = 3) uniform LightmapRayList
{
	int uTexelBegin;
	int uTexelEnd;
	int uNumRays;
	int uTexelsPerRow;
	int uNumRows;
	int uJitter;
};

layout (location = 2) uniform sampler2D uTexPosition;
layout (location = 3) uniform sampler2D uTexNormal;
layout (location = 4) uniform usamplerBuffer uValidList;

vec3 RandomDirection(inout uint seed)
{
	float z = RandomFloat(seed) * 2.0 - 1.0;
	float xy = sqrt(1.0 - z*z);
	float alpha = RandomFloat(seed) * PI * 2.0;
	return vec3(xy * cos(alpha), xy * sin(alpha), z);
}

vec3 RandomDiffuse(inout uint seed, in vec3 base_dir)
{
	vec3 dir = RandomDirection(seed);
	float d = dot(dir, base_dir);
	vec3 c = d * base_dir;
	vec3 s = dir - c;
	float z2 = clamp(abs(d), 0.0, 1.0);
	float xy = sqrt(1.0 - z2);	
	vec3 s_dir =  sqrt(z2) * base_dir;
	if (length(s)>0.0)
	{		
		s_dir += xy * normalize(s);
	}
	return s_dir;
}

void main()
{
	ivec2 local_id = ivec3(gl_LocalInvocationID).xy;	
	ivec2 group_id = ivec3(gl_WorkGroupID).xy;
	g_id_io = ivec2(local_id.x + local_id.y * 8 + group_id.x * 64, group_id.y);
	int idx_texel_out = g_id_io.x/uNumRays + g_id_io.y*uTexelsPerRow;	
	int idx_texel_in = idx_texel_out + uTexelBegin;
	if (idx_texel_in >= uTexelEnd) return;

	int idx_ray = g_id_io.x % uNumRays;

	ivec2 texel_coord = ivec2(texelFetch(uValidList, idx_texel_in).xy);	
	g_origin = texelFetch(uTexPosition, texel_coord, 0).xyz;
	vec3 norm = texelFetch(uTexNormal, texel_coord, 0).xyz;
	uint seed = InitRandomSeed(uJitter, idx_texel_out * uNumRays +  idx_ray);
	g_dir = RandomDiffuse(seed, norm);	

	render();
}
#endif

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

CompFogRayMarching::CompFogRayMarching(int target_mode) : m_target_mode(target_mode)
{
	std::string defines = "";
	if (target_mode == 0)
	{
		defines += "#define TO_CAMERA 1\n";
	}
	else
	{
		defines += "#define TO_CAMERA 0\n";
	}

	if (target_mode == 1)
	{
		defines += "#define TO_PROBES 1\n";
	}
	else
	{
		defines += "#define TO_PROBES 0\n";
	}

	if (target_mode == 2)
	{
		defines += "#define TO_LIGHTMAP 1\n";
	}
	else
	{
		defines += "#define TO_LIGHTMAP 0\n";
	}

	{
		std::string s_compute = g_compute;
		replace(s_compute, "#DEFINES#", defines.c_str());

		GLShader comp_shader(GL_COMPUTE_SHADER, s_compute.c_str());
		m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
	}

	{
		std::string s_compute = g_compute_shadow;
		replace(s_compute, "#DEFINES#", defines.c_str());

		GLShader comp_shader_shadow(GL_COMPUTE_SHADER, s_compute.c_str());
		m_prog_shadow = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader_shadow));
	}
}


void CompFogRayMarching::_render_no_shadow(const RenderParams& params)
{
	int width = params.target->m_width;
	int height = params.target->m_height;

	glUseProgram(m_prog->m_id);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.fog->m_constant.m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.constant_diretional_light->m_id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, params.target->m_tex_depth->tex_id);
	glUniform1i(0, 0);

	glBindImageTexture(0, params.target->m_tex_video->tex_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);

	if (m_target_mode == 0)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, params.camera->m_constant.m_id);

		glm::ivec2 blocks = { (width + 7) / 8, (height + 7) / 8 };
		glDispatchCompute(blocks.x, blocks.y, 1);
	}
	else if (m_target_mode == 1)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, params.prl->m_constant.m_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, params.prl->buf_positions->m_id);

		glm::ivec2 blocks = { (width + 63) / 64, height };
		glDispatchCompute(blocks.x, blocks.y, 1);
	}
	else if (m_target_mode == 2)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, params.lmrl->m_constant.m_id);

		{
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, params.lmrl->source->m_tex_position->tex_id);
			glUniform1i(1, 1);
		}

		{
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, params.lmrl->source->m_tex_normal->tex_id);
			glUniform1i(2, 2);
		}

		{
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_BUFFER, params.lmrl->source->valid_list->tex_id);
			glUniform1i(3, 3);
		}

		glm::ivec2 blocks = { (width + 63) / 64, height };
		glDispatchCompute(blocks.x, blocks.y, 1);
	}

	glUseProgram(0);
}


void CompFogRayMarching::_render_shadowed(const RenderParams& params)
{
	int width = params.target->m_width;
	int height = params.target->m_height;

	glUseProgram(m_prog_shadow->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.fog->m_constant.m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.constant_diretional_light->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, params.constant_diretional_shadow->m_id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, params.target->m_tex_depth->tex_id);
	glUniform1i(0, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, params.tex_shadow);
	glUniform1i(1, 1);

	glBindImageTexture(0, params.target->m_tex_video->tex_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);

	if (m_target_mode == 0)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 3, params.camera->m_constant.m_id);

		glm::ivec2 blocks = { (width + 7) / 8, (height + 7) / 8 };
		glDispatchCompute(blocks.x, blocks.y, 1);
	}
	else if (m_target_mode == 1)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 3, params.prl->m_constant.m_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, params.prl->buf_positions->m_id);

		glm::ivec2 blocks = { (width + 63) / 64, height };
		glDispatchCompute(blocks.x, blocks.y, 1);
	}
	else if (m_target_mode == 2)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 3, params.lmrl->m_constant.m_id);

		{
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, params.lmrl->source->m_tex_position->tex_id);
			glUniform1i(2, 2);
		}

		{
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, params.lmrl->source->m_tex_normal->tex_id);
			glUniform1i(3,3);
		}

		{
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_BUFFER, params.lmrl->source->valid_list->tex_id);
			glUniform1i(4, 4);
		}

		glm::ivec2 blocks = { (width + 63) / 64, height };
		glDispatchCompute(blocks.x, blocks.y, 1);
	}

	glUseProgram(0);

}

void CompFogRayMarching::render(const RenderParams& params)
{
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	if (params.constant_diretional_shadow == nullptr)
	{
		_render_no_shadow(params);
	}
	else
	{
		_render_shadowed(params);
	}
}
