#include <string>
#include <GL/glew.h>
#include "CompDrawFog.h"
#include "renderers/BVHRenderTarget.h"

static std::string g_compute =
R"(#version 430

#DEFINES#

layout (std140, binding = 0) uniform Fog
{
	vec4 fog_rgba;
};

layout (location = 0) uniform sampler2D uDepthTex;

#define PI 3.14159265359
#define RECIPROCAL_PI 0.3183098861837907

#if HAS_ENVIRONMENT_MAP
layout (std140, binding = 1) uniform EnvironmentMap
{
	vec4 uSHCoefficients[9];
};

vec3 GetIrradiance()
{
	return uSHCoefficients[0].xyz * 0.886227;
}
#elif HAS_AMBIENT_LIGHT
layout (std140, binding = 1) uniform AmbientLight
{
	vec4 uAmbientColor;
};

vec3 GetIrradiance()
{
	return uAmbientColor.xyz * PI;
}
#elif HAS_HEMISPHERE_LIGHT

layout (std140, binding = 1) uniform HemisphereLight
{
	vec4 uHemisphereSkyColor;
	vec4 uHemisphereGroundColor;
};

vec3 GetIrradiance()
{
	return (uHemisphereSkyColor.xyz + uHemisphereGroundColor.xyz)*0.5*PI;
}

#else
vec3 GetIrradiance()
{
	return vec3(0.0);
}
#endif

layout (binding=0, rgba16f) uniform image2D uImgColor;

layout(local_size_x = 8, local_size_y = 8) in;

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
	
	float dis = texelFetch(uDepthTex, id, 0).x;
	float alpha = 1.0 - pow(1.0 - fog_rgba.w, dis);

	vec3 irradiance = GetIrradiance();	

	vec3 col = fog_rgba.xyz* irradiance * RECIPROCAL_PI * alpha;
	
	vec4 base = imageLoad(uImgColor, id);
	imageStore(uImgColor, id, (1.0 - alpha) * base + vec4(col, alpha));
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

CompDrawFog::CompDrawFog(const Options& options) : m_options(options)
{
	std::string s_compute = g_compute;

	std::string defines = "";

	if (options.has_environment_map)
	{
		defines += "#define HAS_ENVIRONMENT_MAP 1\n";
	}
	else
	{
		defines += "#define HAS_ENVIRONMENT_MAP 0\n";
	}

	if (options.has_ambient_light)
	{
		defines += "#define HAS_AMBIENT_LIGHT 1\n";
	}
	else
	{
		defines += "#define HAS_AMBIENT_LIGHT 0\n";
	}

	if (options.has_hemisphere_light)
	{
		defines += "#define HAS_HEMISPHERE_LIGHT 1\n";
	}
	else
	{
		defines += "#define HAS_HEMISPHERE_LIGHT 0\n";
	}

	replace(s_compute, "#DEFINES#", defines.c_str());

	GLShader comp_shader(GL_COMPUTE_SHADER, s_compute.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
}

void CompDrawFog::render(const RenderParams& params)
{
	int width = params.target->m_width;
	int height = params.target->m_height;

	glUseProgram(m_prog->m_id);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.constant_fog->m_id);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, params.target->m_tex_depth->tex_id);
	glUniform1i(0, 0);

	if (m_options.has_environment_map)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.lights->environment_map->m_constant.m_id);
	}

	if (m_options.has_ambient_light)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.lights->ambient_light->m_constant.m_id);
	}

	if (m_options.has_hemisphere_light)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.lights->hemisphere_light->m_constant.m_id);
	}	

	glBindBufferBase(GL_UNIFORM_BUFFER, 2, params.constant_camera->m_id);

	glBindImageTexture(0, params.target->m_tex_video->tex_id, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);

	glm::ivec2 blocks = { (width + 7) / 8, (height + 7) / 8 };
	glDispatchCompute(blocks.x, blocks.y, 1);

	glUseProgram(0);
}

