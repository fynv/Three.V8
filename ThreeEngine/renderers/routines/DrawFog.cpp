#include <string>
#include <GL/glew.h>
#include "DrawFog.h"
#include "cameras/Camera.h"
#include "cameras/Reflector.h"

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
};

#if IS_REFLECT
layout (std140, binding = 2) uniform MatrixReflector
{
	mat4 uMatrixReflector;
};
#endif

#if MSAA
layout (location = 0) uniform sampler2DMS uDepthTex;
#else
layout (location = 0) uniform sampler2D uDepthTex;
#endif

#define PI 3.14159265359
#define RECIPROCAL_PI 0.3183098861837907

#if HAS_ENVIRONMENT_MAP
layout (std140, binding = 3) uniform EnvironmentMap
{
	vec4 uSHCoefficients[9];
};

vec3 GetIrradiance()
{
	return uSHCoefficients[0].xyz * 0.886227;
}
#elif HAS_AMBIENT_LIGHT
layout (std140, binding = 3) uniform AmbientLight
{
	vec4 uAmbientColor;
};

vec3 GetIrradiance()
{
	return uAmbientColor.xyz * PI;
}
#elif HAS_HEMISPHERE_LIGHT

layout (std140, binding = 3) uniform HemisphereLight
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

void main()
{
	ivec2 coord = ivec2(gl_FragCoord.xy);	
	float depth = 0;
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

#if !IS_REFLECT
	float length = dis;
#else
	vec3 dir = (uInvViewMat * vec4(pos_view.xyz/dis, 0.0)).xyz;
	vec4 pos_refl_eye = uMatrixReflector * vec4(uEyePos, 1.0);
	vec4 dir_refl = uMatrixReflector * vec4(dir, 0.0);
	if (dir_refl.z * pos_refl_eye.z>0) 
    {
		outColor = vec4(0.0);
        return;
    }
	float length = dis + pos_refl_eye.z/dir_refl.z;
#endif

	float alpha = 1.0 - pow(1.0 - fog_rgba.w, length);
	
	vec3 irradiance = GetIrradiance();	

	vec3 col = fog_rgba.xyz* irradiance * RECIPROCAL_PI * alpha;

	outColor = vec4(col, alpha);
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

DrawFog::DrawFog(const Options& options) : m_options(options)
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

	if (options.is_reflect)
	{
		defines += "#define IS_REFLECT 1\n";		
	}
	else
	{
		defines += "#define IS_REFLECT 0\n";	
	}

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

	replace(s_frag, "#DEFINES#", defines.c_str());

	GLShader vert_shader(GL_VERTEX_SHADER, g_vertex.c_str());
	GLShader frag_shader(GL_FRAGMENT_SHADER, s_frag.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(vert_shader, frag_shader));
}

void DrawFog::render(const RenderParams& params)
{
	glDisable(GL_CULL_FACE);

	glUseProgram(m_prog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.camera->m_constant.m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.constant_fog->m_id);

	if (m_options.is_reflect)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, params.camera->reflector->m_constant.m_id);
	}

	if (m_options.has_environment_map)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 3, params.lights->environment_map->m_constant.m_id);
	}

	if (m_options.has_ambient_light)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 3, params.lights->ambient_light->m_constant.m_id);
	}

	if (m_options.has_hemisphere_light)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, 3, params.lights->hemisphere_light->m_constant.m_id);
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

