#include <GL/glew.h>
#include "StandardRoutine.h"
#include "materials/MeshStandardMaterial.h"

static std::string s_vertex_common =
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

#GLOBAL#

void main()
{
	vec4 wolrd_pos = uModelMat * vec4(aPos, 1.0);
	gl_Position = uProjMat*(uViewMat*wolrd_pos);
	vViewDir = uEyePos - wolrd_pos.xyz;
	vec4 world_norm = uNormalMat * vec4(aNorm, 0.0);
	vNorm = world_norm.xyz;

#MAIN#
}
)";

static std::string s_vertex_color[2] =
{
R"(layout (location = 2) in vec3 aColor;
layout (location = 2) out vec3 vColor;
)",
R"(	{
		vColor = aColor;
	}
)"
};

static std::string s_vertex_uv[2] =
{
R"(layout (location = 3) in vec2 aUV;
layout (location = 3) out vec2 vUV;
)",
R"(	{
		vUV = aUV;
	}
)"
};

static std::string s_vertex_tangent[2] =
{
R"(layout (location = 4) in vec3 aTangent;
layout (location = 5) in vec3 aBitangent;
layout (location = 4) out vec3 vTangent;
layout (location = 5) out vec3 vBitangent;
)",
R"(	{
		vec4 world_tangent = uModelMat * vec4(aTangent, 0.0);
		vTangent =  world_tangent.xyz;

		vec4 world_bitangent = uModelMat * vec4(aBitangent, 0.0);
		vBitangent =  world_bitangent.xyz;
	}
)"
};

static std::string s_frag_common =
R"(#version 430
layout (location = 0) in vec3 vViewDir;
layout (location = 1) in vec3 vNorm;

layout (std140, binding = 2) uniform Material
{
	vec4 uColor;
	vec2 uNormalScale;
	float uMetallicFactor;
	float uRoughnessFactor;
};

#GLOBAL#


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

const IncidentLight directLight = IncidentLight(vec3(4.0, 4.0, 4.0), normalize(vec3(1.0f, 2.0f, 1.0f)), true);

out vec4 outColor;

void main()
{
	vec3 base_color = uColor.xyz;
	float metallicFactor = uMetallicFactor;
	float roughnessFactor = uRoughnessFactor;

	vec3 viewDir = normalize(vViewDir);
	vec3 norm = normalize(vNorm);
	if (dot(viewDir,norm)<0.0) norm = -norm;

#MAIN#	

	PhysicalMaterial material;
	material.diffuseColor = base_color * ( 1.0 - metallicFactor );

	vec3 dxy = max(abs(dFdx(norm)), abs(dFdy(norm)));
	float geometryRoughness = max(max(dxy.x, dxy.y), dxy.z);

	material.roughness = max( roughnessFactor, 0.0525 );
	material.roughness += geometryRoughness;
	material.roughness = min( material.roughness, 1.0 );
	
	material.specularColor = mix( vec3( 0.04 ), base_color, metallicFactor );
	material.specularF90 = 1.0;
		
	float dotNL =  saturate(dot(norm, directLight.direction));
	vec3 irradiance = dotNL * directLight.color;

	vec3 specular = irradiance * BRDF_GGX( directLight.direction, viewDir, norm, material.specularColor, material.specularF90, material.roughness );
	vec3 diffuse = irradiance * BRDF_Lambert( material.diffuseColor );

	vec3 ambient = 0.3 * BRDF_Lambert( base_color );
	vec3 col = specular + diffuse + ambient;

	outColor = vec4(col, 1.0);
}
)";

static std::string s_frag_color[2] =
{
R"(layout (location = 2) in vec3 vColor;
)",
R"(	{
		base_color *= vColor;
	}
)"
};

static std::string s_frag_uv =
R"(layout (location = 3) in vec2 vUV;
)";

static std::string s_frag_color_tex[2] =
{
R"(layout (location = 0) uniform sampler2D uTexColor;
)",
R"(	{
		base_color *= texture(uTexColor, vUV).xyz;
	}
)"
};

static std::string s_frag_metalness_map[2] =
{
R"(layout (location = 1) uniform sampler2D uTexMetalness;
)",
R"(	{
		metallicFactor *= texture(uTexMetalness, vUV).z;
	}
)"
};

static std::string s_frag_roughness_map[2] =
{
R"(layout (location = 2) uniform sampler2D uTexRoughness;
)",
R"(	{
		roughnessFactor *= texture(uTexRoughness, vUV).y;
	}
)"
};

static std::string s_frag_normal_map[2] =
{
R"(layout (location = 3) uniform sampler2D uTexNormal;
layout (location = 4) in vec3 vTangent;
layout (location = 5) in vec3 vBitangent;
)",
R"(	{
		vec3 T = normalize(vTangent);
		vec3 B = normalize(vBitangent);
		vec3 bump =  texture(uTexNormal, vUV).xyz;
		bump = (2.0 * bump - 1.0) * vec3(uNormalScale.x, uNormalScale.y, 1.0);
		norm = normalize(bump.x*T + bump.y*B + bump.z*norm);
	}
)"
};

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
	s_vertex = s_vertex_common;
	s_frag = s_frag_common;

	std::string s_v_global = "";
	std::string s_v_main = "";
	std::string s_f_global = "";
	std::string s_f_main = "";

	if (options.has_color)
	{
		s_v_global += s_vertex_color[0];
		s_v_main += s_vertex_color[1];
		s_f_global += s_frag_color[0];
		s_f_main += s_frag_color[1];
	}

	bool has_uv = options.has_color_texture || options.has_metalness_map || options.has_roughness_map;

	if (has_uv)
	{
		s_v_global += s_vertex_uv[0];
		s_v_main += s_vertex_uv[1];
		s_f_global += s_frag_uv;		
	}

	if (options.has_color_texture)
	{
		s_f_global += s_frag_color_tex[0];
		s_f_main += s_frag_color_tex[1];
	}

	if (options.has_metalness_map)
	{
		s_f_global += s_frag_metalness_map[0];
		s_f_main += s_frag_metalness_map[1];
	}

	if (options.has_roughness_map)
	{
		s_f_global += s_frag_roughness_map[0];
		s_f_main += s_frag_roughness_map[1];
	}

	if (options.has_normal_map)
	{
		s_v_global += s_vertex_tangent[0];
		s_v_main += s_vertex_tangent[1];
		s_f_global += s_frag_normal_map[0];
		s_f_main += s_frag_normal_map[1];
	}

	replace(s_vertex, "#GLOBAL#", s_v_global.c_str());
	replace(s_vertex, "#MAIN#", s_v_main.c_str());
	replace(s_frag, "#GLOBAL#", s_f_global.c_str());
	replace(s_frag, "#MAIN#", s_f_main.c_str());
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

	glUseProgram(m_prog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.constant_camera->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.constant_model->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, material.constant_material.m_id);

	glBindBuffer(GL_ARRAY_BUFFER, geo.pos_buf->m_id);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, geo.normal_buf->m_id);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);

	if (m_options.has_color)
	{
		glBindBuffer(GL_ARRAY_BUFFER, params.primitive->color_buf->m_id);
		glVertexAttribPointer(2, params.primitive->type_color, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(2);
	}

	bool has_uv = m_options.has_color_texture || m_options.has_metalness_map || m_options.has_roughness_map || m_options.has_normal_map;
	if (has_uv)
	{
		glBindBuffer(GL_ARRAY_BUFFER, params.primitive->uv_buf->m_id);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(3);
	}

	if (m_options.has_normal_map)
	{
		glBindBuffer(GL_ARRAY_BUFFER, geo.tangent_buf->m_id);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(4);

		glBindBuffer(GL_ARRAY_BUFFER, geo.bitangent_buf->m_id);
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
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

