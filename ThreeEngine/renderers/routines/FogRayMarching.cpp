#include <string>
#include <GL/glew.h>
#include "FogRayMarching.h"


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

layout (std140, binding = 2) uniform DirectionalLight
{
	vec4 light_color;
	vec4 light_origin;
	vec4 light_direction;
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
	float step = dis/float(max_num_steps);
	if (step<min_step) step = min_step;

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
		float l_shadow = 1.0;
		float zEye = -dot(pos_world - light_origin.xyz, light_direction.xyz);
		if (zEye>0.0)
		{
			float att = pow(1.0 - fog_rgba.w, zEye);
			l_shadow *= att;
		}		
		float att =  pow(1.0 - fog_rgba.w, sample_t);

		vec3 irradiance = light_color.xyz * l_shadow;

		col+=fog_rgba.xyz*irradiance*RECIPROCAL_PI * _step_alpha* att;
	}
	outColor = vec4(col, 0.0);
}
)";


static std::string g_frag_shadow =
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

layout (std140, binding = 2) uniform DirectionalLight
{
	vec4 light_color;
	vec4 light_origin;
	vec4 light_direction;
};

layout (std140, binding = 3) uniform DirectionalShadow
{
	mat4 VPSBMat;
	mat4 shadowProjMat;
	mat4 shadowViewMat;
    vec2 shadowLeftRight;
	vec2 shadowBottomTop;
	vec2 shadowNearFar;
};


#if MSAA
layout (location = 0) uniform sampler2DMS uDepthTex;
#else
layout (location = 0) uniform sampler2D uDepthTex;
#endif

layout (location = 1) uniform sampler2D uShadowTex;


vec3 computeShadowCoords(in mat4 VPSB, in vec3 pos_world)
{
	vec4 shadowCoords = VPSB * vec4(pos_world, 1.0);
	return shadowCoords.xyz;
}

float borderDepthTexture(vec2 uv)
{
	return ((uv.x <= 1.0) && (uv.y <= 1.0) &&
	 (uv.x >= 0.0) && (uv.y >= 0.0)) ? textureLod(uShadowTex, uv, 0.0).x : 1.0;
}

float borderPCFTexture(vec3 uvz)
{
    float d = borderDepthTexture(uvz.xy);
    return clamp(1.0 - (uvz.z - d)*5000.0, 0.0, 1.0);
}


float computeShadowCoef(in mat4 VPSB, in vec3 pos_world)
{
	vec3 shadowCoords;
	shadowCoords = computeShadowCoords(VPSB, pos_world);
	return borderPCFTexture(shadowCoords);
}


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

	float start = 0.0;
	float end = dis;
	{
		vec3 origin_light = (shadowViewMat * vec4(uEyePos, 1.0)).xyz;
		vec3 dir_light = (shadowViewMat * vec4(dir, 0.0)).xyz;

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
	
	float step = (end - start)/float(max_num_steps);
	if (step<min_step) step = min_step;
	
	float step_alpha =  1.0 - pow(1.0 - fog_rgba.w, step);

	uint seed = InitRandomSeed(uint(vUV.x*100000.0), uint(vUV.y*100000.0));
	float delta = RandomFloat(seed);

	vec3 col = vec3(0.0);
	start += step * (delta - 0.5);
	for (float t = start; t<end; t+=step)
	{
		float _step = min(step, dis - t);
		float _step_alpha = (_step == step) ? step_alpha :  (1.0 - pow(1.0 - fog_rgba.w, _step));
		float sample_t = max(t + _step*0.5, 0.0);

		vec3 pos_world = uEyePos + dir * sample_t;
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
			vec3 irradiance = light_color.xyz * l_shadow;

			col+=fog_rgba.xyz*irradiance*RECIPROCAL_PI * _step_alpha* att;
		}		
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


FogRayMarching::FogRayMarching(bool msaa): m_msaa(msaa)
{
	std::string defines = "";
	if (msaa)
	{
		defines += "#define MSAA 1\n";
	}
	else
	{
		defines += "#define MSAA 0\n";
	}

	std::string s_frag = g_frag;
	replace(s_frag, "#DEFINES#", defines.c_str());

	std::string s_frag_shadow = g_frag_shadow;
	replace(s_frag_shadow, "#DEFINES#", defines.c_str());

	GLShader vert_shader(GL_VERTEX_SHADER, g_vertex.c_str());
	GLShader frag_shader(GL_FRAGMENT_SHADER, s_frag.c_str());
	GLShader frag_shader_shadow(GL_FRAGMENT_SHADER, s_frag_shadow.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(vert_shader, frag_shader));
	m_prog_shadow = (std::unique_ptr<GLProgram>)(new GLProgram(vert_shader, frag_shader_shadow));

}

void FogRayMarching::_render_no_shadow(const RenderParams& params)
{
	glUseProgram(m_prog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.constant_camera->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.constant_fog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, params.constant_diretional_light->m_id);

	glActiveTexture(GL_TEXTURE0);
	if (m_msaa)
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

void FogRayMarching::_render_shadowed(const RenderParams& params)
{
	glUseProgram(m_prog_shadow->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.constant_camera->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.constant_fog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, params.constant_diretional_light->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, params.constant_diretional_shadow->m_id);

	glActiveTexture(GL_TEXTURE0);
	if (m_msaa)
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, params.tex_depth->tex_id);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, params.tex_depth->tex_id);
	}
	glUniform1i(0, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, params.tex_shadow);
	glUniform1i(1, 1);

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glUseProgram(0);

}

void FogRayMarching::render(const RenderParams& params)
{
	if (params.constant_diretional_shadow == nullptr)
	{
		_render_no_shadow(params);
	}
	else
	{
		_render_shadowed(params);
	}
}

