#include <GL/glew.h>
#include "models/ModelComponents.h"
#include "lights/DirectionalLight.h"
#include "StandardRoutine.h"

static std::string g_vertex =
R"(#version 430
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;

layout (std140, binding = 0) uniform Camera
{
	mat4 uProjMat;
	mat4 uViewMat;	
	vec3 uEyePos;
};

layout (std140, binding = 1) uniform Model
{
	mat4 uModelMat;
	mat4 uNormalMat;
};

layout (location = 0) out vec3 vViewDir;
layout (location = 1) out vec3 vNorm;

#DEFINES#

#if HAS_COLOR
layout (location = 2) in vec4 aColor;
layout (location = 2) out vec4 vColor;
#endif

#if HAS_UV
layout (location = 3) in vec2 aUV;
layout (location = 3) out vec2 vUV;
#endif

#if HAS_NORMAL_MAP
layout (location = 4) in vec3 aTangent;
layout (location = 4) out vec3 vTangent;
layout (location = 5) in vec3 aBitangent;
layout (location = 5) out vec3 vBitangent;
#endif

layout (location = 6) out vec3 vWorldPos;

void main()
{
	vec4 wolrd_pos = uModelMat * vec4(aPos, 1.0);
	gl_Position = uProjMat*(uViewMat*wolrd_pos);
	vWorldPos = wolrd_pos.xyz;
	vViewDir = uEyePos - wolrd_pos.xyz;
	vec4 world_norm = uNormalMat * vec4(aNorm, 0.0);
	vNorm = world_norm.xyz;

#if HAS_COLOR
	vColor = aColor;
#endif

#if HAS_UV
	vUV = aUV;
#endif

#if HAS_NORMAL_MAP
	vec4 world_tangent = uModelMat * vec4(aTangent, 0.0);
	vTangent =  world_tangent.xyz;

	vec4 world_bitangent = uModelMat * vec4(aBitangent, 0.0);
	vBitangent =  world_bitangent.xyz;
#endif
}
)";

static std::string g_frag =
R"(#version 430
layout (location = 0) in vec3 vViewDir;
layout (location = 1) in vec3 vNorm;

layout (std140, binding = 2) uniform Material
{
	vec4 uColor;
	vec4 uEmissive;
	vec2 uNormalScale;
	float uMetallicFactor;
	float uRoughnessFactor;
	float uAlphaCutoff;
	int uDoubleSided;
};

#DEFINES#

#if HAS_COLOR
layout (location = 2) in vec4 vColor;
#endif

#if HAS_UV
layout (location = 3) in vec2 vUV;
#endif

#if HAS_COLOR_TEX
layout (location = 0) uniform sampler2D uTexColor;
#endif

#if HAS_METALNESS_MAP
layout (location = 1) uniform sampler2D uTexMetalness;
#endif

#if HAS_ROUGHNESS_MAP
layout (location = 2) uniform sampler2D uTexRoughness;
#endif

#if HAS_NORMAL_MAP
layout (location = 3) uniform sampler2D uTexNormal;
layout (location = 4) in vec3 vTangent;
layout (location = 5) in vec3 vBitangent;
#endif

#if HAS_EMISSIVE_MAP
layout (location = 4) uniform sampler2D uTexEmissive;
#endif

layout (location = 6) in vec3 vWorldPos;

struct IncidentLight {
	vec3 color;
	vec3 direction;
	bool visible;
};

#define EPSILON 1e-6
#define RECIPROCAL_PI 0.3183098861837907

#ifndef saturate
#define saturate( a ) clamp( a, 0.0, 1.0 )
#endif

float pow2( const in float x ) { return x*x; }

struct PhysicalMaterial 
{
	vec3 diffuseColor;
	float roughness;
	vec3 specularColor;
	float specularF90;
};


vec3 F_Schlick(const in vec3 f0, const in float f90, const in float dotVH) 
{
	float fresnel = exp2( ( - 5.55473 * dotVH - 6.98316 ) * dotVH );
	return f0 * ( 1.0 - fresnel ) + ( f90 * fresnel );
}

float V_GGX_SmithCorrelated( const in float alpha, const in float dotNL, const in float dotNV ) 
{
	float a2 = pow2( alpha );
	float gv = dotNL * sqrt( a2 + ( 1.0 - a2 ) * pow2( dotNV ) );
	float gl = dotNV * sqrt( a2 + ( 1.0 - a2 ) * pow2( dotNL ) );
	return 0.5 / max( gv + gl, EPSILON );
}

float D_GGX( const in float alpha, const in float dotNH ) 
{
	float a2 = pow2( alpha );
	float denom = pow2( dotNH ) * ( a2 - 1.0 ) + 1.0; 
	return RECIPROCAL_PI * a2 / pow2( denom );
}

vec3 BRDF_Lambert(const in vec3 diffuseColor) 
{
	return RECIPROCAL_PI * diffuseColor;
}

vec3 BRDF_GGX( const in vec3 lightDir, const in vec3 viewDir, const in vec3 normal, const in vec3 f0, const in float f90, const in float roughness ) 
{
	float alpha = pow2(roughness);

	vec3 halfDir = normalize(lightDir + viewDir);

	float dotNL = saturate(dot(normal, lightDir));
	float dotNV = saturate(dot(normal, viewDir));
	float dotNH = saturate(dot(normal, halfDir));
	float dotVH = saturate(dot(viewDir, halfDir));

	vec3 F = F_Schlick(f0, f90, dotVH);
	float V = V_GGX_SmithCorrelated(alpha, dotNL, dotNV);
	float D = D_GGX( alpha, dotNH );
	return F*(V*D);
}

struct DirectionalLight
{
	mat4 shadowVPSBMatrix;
	vec4 color;
	vec4 direction;
	int has_shadow;
};

#if NUM_DIRECTIONAL_LIGHTS>0
layout (std140, binding = 3) uniform DirectionalLights
{
	DirectionalLight uDirectionalLights[NUM_DIRECTIONAL_LIGHTS];
};
#endif

#if NUM_DIRECTIONAL_SHADOWS>0
layout (location = 5) uniform sampler2D uShadowTex[NUM_DIRECTIONAL_SHADOWS];

vec3 computeShadowCoords(in mat4 VPSB)
{
	vec4 shadowCoords = VPSB * vec4(vWorldPos, 1.0);
	return shadowCoords.xyz;
}

float computeShadowCoef(in mat4 VPSB, int shadow_id)
{
	vec3 shadowCoords;
	shadowCoords = computeShadowCoords(VPSB);
	float d = texture(uShadowTex[shadow_id], shadowCoords.xy).x;
	return clamp(1.0 - (shadowCoords.z - d)*1000.0, 0.0, 1.0);
}

#endif

layout (location = 0) out vec4 out0;

#if ALPHA_BLEND
layout (location = 1) out float out1;
#endif

void main()
{
	vec4 base_color = uColor;
#if HAS_COLOR
	base_color *= vColor;
#endif

#if HAS_COLOR_TEX
	base_color *= texture(uTexColor, vUV);
#endif

#if ALPHA_MASK
	base_color.w = base_color.w > uAlphaCutoff ? 1.0 : 0.0;
#endif

	if (base_color.w == 0.0) discard;

	float metallicFactor = uMetallicFactor;
	float roughnessFactor = uRoughnessFactor;

#if HAS_METALNESS_MAP
	metallicFactor *= texture(uTexMetalness, vUV).z;
#endif

#if HAS_ROUGHNESS_MAP
	roughnessFactor *= texture(uTexRoughness, vUV).y;
#endif

	vec3 viewDir = normalize(vViewDir);
	vec3 norm = normalize(vNorm);

#if HAS_NORMAL_MAP
	{
		vec3 T = normalize(vTangent);
		vec3 B = normalize(vBitangent);
		vec3 bump =  texture(uTexNormal, vUV).xyz;
		bump = (2.0 * bump - 1.0) * vec3(uNormalScale.x, uNormalScale.y, 1.0);
		norm = normalize(bump.x*T + bump.y*B + bump.z*norm);
	}
#endif

	if (uDoubleSided!=0)
	{
		if (dot(viewDir,norm)<0.0) norm = -norm;
	}

	PhysicalMaterial material;
	material.diffuseColor = base_color.xyz * ( 1.0 - metallicFactor );

	vec3 dxy = max(abs(dFdx(norm)), abs(dFdy(norm)));
	float geometryRoughness = max(max(dxy.x, dxy.y), dxy.z);

	material.roughness = max( roughnessFactor, 0.0525 );
	material.roughness += geometryRoughness;
	material.roughness = min( material.roughness, 1.0 );
	
	material.specularColor = mix( vec3( 0.04 ), base_color.xyz, metallicFactor );
	material.specularF90 = 1.0;

	vec3 emissive = uEmissive.xyz;
#if HAS_EMISSIVE_MAP
	emissive *= texture(uTexEmissive, vUV).xyz;
#endif

	vec3 specular = vec3(0.0);
	vec3 diffuse = vec3(0.0);

#if NUM_DIRECTIONAL_LIGHTS>0
	int shadow_id = 0;
	for (int i=0; i< NUM_DIRECTIONAL_LIGHTS; i++)
	{	
		DirectionalLight light_source = uDirectionalLights[i];
		float l_shadow = 1.0;
#if NUM_DIRECTIONAL_SHADOWS>0
		if (light_source.has_shadow!=0)
		{
			l_shadow = computeShadowCoef(light_source.shadowVPSBMatrix, shadow_id);
			shadow_id++;
		}
#endif
		IncidentLight directLight = IncidentLight(light_source.color.xyz * l_shadow, light_source.direction.xyz, true);	

		float dotNL =  saturate(dot(norm, directLight.direction));
		vec3 irradiance = dotNL * directLight.color;

		specular += irradiance * BRDF_GGX( directLight.direction, viewDir, norm, material.specularColor, material.specularF90, material.roughness );
		diffuse += irradiance * BRDF_Lambert( material.diffuseColor );
	}
#endif

	vec3 ambient = 0.3 * BRDF_Lambert( base_color.xyz );
	vec3 col = emissive + specular + ambient;
#if !IS_HIGHTLIGHT
	col += diffuse;
#endif
	col = clamp(col, 0.0, 1.0);	

#if ALPHA_BLEND
	float alpha = base_color.w;
	float a = min(1.0, alpha) * 8.0 + 0.01;
	float b = -gl_FragCoord.z * 0.95 + 1.0;
	float weight = clamp(a * a * a * 1e8 * b * b * b, 1e-2, 3e2);
	out0 = vec4(col * alpha, alpha) * weight;
	out1 = alpha;
#elif IS_HIGHTLIGHT
	out0 = vec4(col, 0.0);
#else
	out0 = vec4(col, 1.0);
#endif
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

void StandardRoutine::s_generate_shaders(const Options& options, std::string& s_vertex, std::string& s_frag)
{
	s_vertex = g_vertex;
	s_frag = g_frag;

	std::string defines = "";

	if (options.alpha_mode == AlphaMode::Mask)
	{
		defines += "#define ALPHA_MASK 1\n";
	}
	else
	{
		defines += "#define ALPHA_MASK 0\n";
	}

	if (options.alpha_mode == AlphaMode::Blend)
	{
		if (options.is_highlight_pass)
		{
			defines += "#define ALPHA_BLEND 0\n";
			defines += "#define IS_HIGHTLIGHT 1\n";
		}
		else
		{
			defines += "#define ALPHA_BLEND 1\n";
			defines += "#define IS_HIGHTLIGHT 0\n";
		}
	}
	else
	{
		defines += "#define ALPHA_BLEND 0\n";
		defines += "#define IS_HIGHTLIGHT 0\n";
	}

	if (options.has_color)
	{
		defines += "#define HAS_COLOR 1\n";
	}
	else
	{
		defines += "#define HAS_COLOR 0\n";
	}

	bool has_uv = options.has_color_texture || options.has_metalness_map || options.has_roughness_map || options.has_normal_map || options.has_emissive_map;

	if (has_uv)
	{
		defines += "#define HAS_UV 1\n";
	}
	else
	{
		defines += "#define HAS_UV 0\n";
	}
	
	if (options.has_color_texture)
	{
		defines += "#define HAS_COLOR_TEX 1\n";
	}
	else
	{
		defines += "#define HAS_COLOR_TEX 0\n";
	}

	if (options.has_metalness_map)
	{
		defines += "#define HAS_METALNESS_MAP 1\n";
	}
	else
	{
		defines += "#define HAS_METALNESS_MAP 0\n";
	}

	if (options.has_roughness_map)
	{
		defines += "#define HAS_ROUGHNESS_MAP 1\n";
	}
	else
	{
		defines += "#define HAS_ROUGHNESS_MAP 0\n";
	}

	if (options.has_normal_map)
	{
		defines += "#define HAS_NORMAL_MAP 1\n";
	}
	else
	{
		defines += "#define HAS_NORMAL_MAP 0\n";
	}

	if (options.has_emissive_map)
	{
		defines += "#define HAS_EMISSIVE_MAP 1\n";
	}
	else
	{
		defines += "#define HAS_EMISSIVE_MAP 0\n";
	}

	{
		char line[64];
		sprintf(line, "#define NUM_DIRECTIONAL_LIGHTS %d\n", options.num_directional_lights);
		defines += line;
	}

	{
		char line[64];
		sprintf(line, "#define NUM_DIRECTIONAL_SHADOWS %d\n", options.num_directional_shadows);
		defines += line;
	}

	replace(s_vertex, "#DEFINES#", defines.c_str());
	replace(s_frag, "#DEFINES#", defines.c_str());
}

StandardRoutine::StandardRoutine(const Options& options) : m_options(options)
{
	std::string s_vertex, s_frag;
	s_generate_shaders(options, s_vertex, s_frag);
	
	m_vert_shader = std::unique_ptr<GLShader>(new GLShader(GL_VERTEX_SHADER, s_vertex.c_str()));
	m_frag_shader = std::unique_ptr<GLShader>(new GLShader(GL_FRAGMENT_SHADER, s_frag.c_str()));
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(*m_vert_shader, *m_frag_shader));
}

void StandardRoutine::render(const RenderParams& params)
{
	const MeshStandardMaterial& material = *(MeshStandardMaterial*)params.material_list[params.primitive->material_idx];
	const GeometrySet& geo = params.primitive->geometry[params.primitive->geometry.size() - 1];

	glEnable(GL_DEPTH_TEST);	
	glDepthFunc(GL_LEQUAL);

	if (material.doubleSided)
	{
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glEnable(GL_CULL_FACE);
	}

	glUseProgram(m_prog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.constant_camera->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.constant_model->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, material.constant_material.m_id);

	if (m_options.num_directional_lights > 0)
	{		
		glBindBufferBase(GL_UNIFORM_BUFFER, 3, params.lights->constant_directional_lights->m_id);
	}

	glBindBuffer(GL_ARRAY_BUFFER, geo.pos_buf->m_id);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, geo.normal_buf->m_id);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);

	if (m_options.has_color)
	{
		glBindBuffer(GL_ARRAY_BUFFER, params.primitive->color_buf->m_id);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(2);
	}

	bool has_uv = m_options.has_color_texture || m_options.has_metalness_map || m_options.has_roughness_map || m_options.has_normal_map || m_options.has_emissive_map;
	if (has_uv)
	{
		glBindBuffer(GL_ARRAY_BUFFER, params.primitive->uv_buf->m_id);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(3);
	}

	if (m_options.has_normal_map)
	{
		glBindBuffer(GL_ARRAY_BUFFER, geo.tangent_buf->m_id);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(4);

		glBindBuffer(GL_ARRAY_BUFFER, geo.bitangent_buf->m_id);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(5);
	}

	if (m_options.has_color_texture)
	{
		const GLTexture2D& tex = *params.tex_list[material.tex_idx_map];
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex.tex_id);
		glUniform1i(0, 0);
	}

	if (m_options.has_metalness_map)
	{
		const GLTexture2D& tex = *params.tex_list[material.tex_idx_metalnessMap];
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex.tex_id);
		glUniform1i(1, 1);
	}

	if (m_options.has_roughness_map)
	{
		const GLTexture2D& tex = *params.tex_list[material.tex_idx_roughnessMap];
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, tex.tex_id);
		glUniform1i(2, 2);
	}

	if (m_options.has_normal_map)
	{
		const GLTexture2D& tex = *params.tex_list[material.tex_idx_normalMap];
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, tex.tex_id);
		glUniform1i(3, 3);
	}

	if (m_options.has_emissive_map)
	{
		const GLTexture2D& tex = *params.tex_list[material.tex_idx_emissiveMap];
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, tex.tex_id);
		glUniform1i(4, 4);
	}

	if (m_options.num_directional_shadows > 0)
	{
		std::vector<int> values(m_options.num_directional_shadows);
		for (int i = 0; i < m_options.num_directional_shadows; i++)
		{
			glActiveTexture(GL_TEXTURE5+i);
			glBindTexture(GL_TEXTURE_2D, params.lights->directional_shadow_texs[i]);
			values[i] = 5 + i;
		}
		glUniform1iv(5, m_options.num_directional_shadows, values.data());
	}

	if (params.primitive->index_buf != nullptr)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, params.primitive->index_buf->m_id);
		if (params.primitive->type_indices == 1)
		{
			glDrawElements(GL_TRIANGLES, params.primitive->num_face * 3, GL_UNSIGNED_BYTE, nullptr);
		}
		else if (params.primitive->type_indices == 2)
		{
			glDrawElements(GL_TRIANGLES, params.primitive->num_face * 3, GL_UNSIGNED_SHORT, nullptr);
		}
		else if (params.primitive->type_indices == 4)
		{
			glDrawElements(GL_TRIANGLES, params.primitive->num_face * 3, GL_UNSIGNED_INT, nullptr);
		}
	}
	else
	{
		glDrawArrays(GL_TRIANGLES, 0, params.primitive->num_pos);
	}

	glUseProgram(0);
}

