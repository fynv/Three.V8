#include <glm.hpp>
#include <string>
#include <GL/glew.h>
#include "DrawIsosurface.h"
#include "cameras/Camera.h"
#include "volume/VolumeIsosurfaceModel.h"


static std::string g_vertex =
R"(#version 430

layout (location = 0) out vec2 vPosProj;

void main()
{
    vec2 grid = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    vec2 pos_proj = grid * vec2(2.0, 2.0) + vec2(-1.0, -1.0);
	vPosProj = pos_proj;
    gl_Position = vec4(pos_proj, 1.0, 1.0);
};
)";


static std::string g_frag_part0 =
R"(#version 430

#DEFINES#

layout (std140, binding = 0) uniform Camera
{
    mat4 uProjMat;
    mat4 uViewMat;	
    mat4 uInvProjMat;
    mat4 uInvViewMat;	
    vec3 uEyePos;
};

layout (std140, binding = 1) uniform Model
{
	mat4 uInvModelMat;
	mat4 uModelMat;
	mat4 uNormalMat;
	ivec4 uSize;
	vec4 uSpacing;
	ivec4 uBsize;
	ivec4 uBnum;
	vec4 uColor;
	float uMetallicFactor;
	float uRoughnessFactor;
	float uStep;
	float uIsovalue;
};

layout (location = 0) uniform sampler3D uTex;
layout (location = 1) uniform sampler3D uGrid;

#if MSAA
layout (location = 2) uniform sampler2DMS uDepthTex;
#else
layout (location = 2) uniform sampler2D uDepthTex;
#endif


struct IncidentLight {
	vec3 color;
	vec3 direction;
	bool visible;
};

#define EPSILON 1e-6
#define PI 3.14159265359
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

float BRDF_Lambert_Toon()
{
	return RECIPROCAL_PI;
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

float BRDF_GGX_Toon(const in vec3 lightDir, const in vec3 viewDir, const in vec3 normal, const in float roughness )
{
	float alpha = pow2(roughness);

	vec3 halfDir = normalize(lightDir + viewDir);

	float dotNL = saturate(dot(normal, lightDir));
	float dotNV = saturate(dot(normal, viewDir));
	float dotNH = saturate(dot(normal, halfDir));	

	float V = V_GGX_SmithCorrelated(alpha, dotNL, dotNV);
	float D = D_GGX( alpha, dotNH );
	return V*D;
}

float luminance(in vec3 color)
{
	return color.x * 0.2126 + color.y * 0.7152 + color.z *0.0722;
}

struct DirectionalLight
{	
	vec4 color;
	vec4 origin;
	vec4 direction;
	int has_shadow;
	float diffuse_thresh;
	float diffuse_high;
	float diffuse_low;
	float specular_thresh;
	float specular_high;
	float specular_low;
};


#if NUM_DIRECTIONAL_LIGHTS>0
layout (std140, binding = BINDING_DIRECTIONAL_LIGHTS) uniform DirectionalLights
{
	DirectionalLight uDirectionalLights[NUM_DIRECTIONAL_LIGHTS];
};
#endif

#if NUM_DIRECTIONAL_SHADOWS>0
struct DirectionalShadow
{
	mat4 VPSBMat;
	mat4 projMat;
	mat4 viewMat;
    vec2 leftRight;
	vec2 bottomTop;
	vec2 nearFar;
	float lightRadius;
	float padding;
};

layout (std140, binding = BINDING_DIRECTIONAL_SHADOWS) uniform DirectionalShadows
{
	DirectionalShadow uDirectionalShadows[NUM_DIRECTIONAL_SHADOWS];
};

layout (location = LOCATION_TEX_DIRECTIONAL_SHADOW) uniform sampler2DShadow uDirectionalShadowTex[NUM_DIRECTIONAL_SHADOWS];

vec3 computeShadowCoords(in mat4 VPSB, in vec3 world_pos)
{
	vec4 shadowCoords = VPSB * vec4(world_pos, 1.0);
	return shadowCoords.xyz;
}

float borderPCFTexture(sampler2DShadow shadowTex, vec3 uvz)
{
	return ((uvz.x <= 1.0) && (uvz.y <= 1.0) &&
	 (uvz.x >= 0.0) && (uvz.y >= 0.0)) ? texture(shadowTex, uvz) : 
	 ((uvz.z <= 1.0) ? 1.0 : 0.0);
}


float computeShadowCoef(in mat4 VPSB, sampler2DShadow shadowTex, in vec3 world_pos)
{
	vec3 shadowCoords;
	shadowCoords = computeShadowCoords(VPSB, world_pos);
	return borderPCFTexture(shadowTex, shadowCoords);
}
#endif
)";

static std::string g_frag_part1 =
R"(
vec3 shGetIrradianceAt( in vec3 normal, in vec4 shCoefficients[ 9 ] ) {

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

#if HAS_REFLECTION_MAP
layout (location = LOCATION_TEX_REFLECTION_MAP) uniform samplerCube uReflectionMap;

vec3 GetReflectionAt(in vec3 reflectVec, in samplerCube reflectMap, float roughness)
{
	float gloss;
	if (roughness<0.053) 
	{
		gloss = 1.0;
	}
	else
	{
		float r2 = roughness*roughness;
		float r4 = r2*r2;
		gloss = log(2.0/r4 - 1.0)/log(2.0)/18.0;
	}

	float mip = (1.0-gloss)*6.0;
	return textureLod(reflectMap, reflectVec, mip).xyz;
}

vec3 getRadiance(in vec3 world_pos, in vec3 reflectVec, float roughness)
{
	return GetReflectionAt(reflectVec, uReflectionMap, roughness);
}

#endif

#if HAS_ENVIRONMENT_MAP
layout (std140, binding = BINDING_ENVIRONMEN_MAP) uniform EnvironmentMap
{
	vec4 uSHCoefficients[9];
	float uDiffuseThresh;
	float uDiffuseHigh;
	float uDiffuseLow;
	float uSpecularThresh;
	float uSpecularHigh;
	float uSpecularLow;
};

vec3 getIrradiance(in vec3 world_pos, in vec3 normal)
{
	return shGetIrradianceAt(normal, uSHCoefficients);
}

#if !HAS_REFLECTION_MAP
vec3 getRadiance(in vec3 world_pos, in vec3 reflectVec, float roughness)
{
	return shGetIrradianceAt(reflectVec, uSHCoefficients) * RECIPROCAL_PI;
}
#endif

#endif


#if HAS_PROBE_GRID
layout (std140, binding = BINDING_PROBE_GRID) uniform ProbeGrid
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

layout (std430, binding = BINDING_PROBES) buffer Probes
{
	vec4 bSHCoefficients[];
};

layout (std430, binding = BINDING_PROBE_VISIBILITY) buffer ProbeVisibility
{
	float bProbeVisibility[];
};

#if PROBE_REFERENCE_RECORDED
layout (std430, binding = BINDING_PROBE_REFERENCES) buffer ProbeReferences
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

float get_visibility(in vec3 world_pos, in ivec3 vert, in vec3 vert_world)
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
	vec3 dir = world_pos - vert_world;	
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

void acc_coeffs(inout vec4 coeffs[9], in ivec3 vert, in float weight)
{
	int idx = vert.x + (vert.y + vert.z*uDivisions.y)*uDivisions.x;
	int offset = idx*9;
	for (int i=0;i<9;i++)
	{
		coeffs[i]+=bSHCoefficients[offset+i]*weight;
	}
#if PROBE_REFERENCE_RECORDED
	bReferenced[idx] = 1;
#endif
}

vec3 getIrradiance(in vec3 world_pos, in vec3 normal)
{
	vec3 size_grid = uCoverageMax.xyz - uCoverageMin.xyz;
	vec3 pos_normalized = (world_pos - uCoverageMin.xyz)/size_grid;
	pos_normalized.y = pow(pos_normalized.y, 1.0/uYpower);	
	vec3 pos_voxel = pos_normalized * vec3(uDivisions) - vec3(0.5);
	pos_voxel = clamp(pos_voxel, vec3(0.0), vec3(uDivisions) - vec3(1.0));
	
	ivec3 i_voxel = clamp(ivec3(pos_voxel), ivec3(0), ivec3(uDivisions) - ivec3(2));
	vec3 frac_voxel = pos_voxel - vec3(i_voxel);

	float sum_weight = 0.0;
	vec4 coeffs[9];
	for (int i=0; i<9; i++) coeffs[i] = vec4(0.0);

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
				vec3 dir = normalize(vert_world - world_pos);
				if (dot(dir, normal)>=0.0)
				{
					vec3 w = vec3(1.0) - abs(vec3(x,y,z) - frac_voxel);
					float weight = w.x * w.y * w.z;
					if (weight>0.0)
					{
						weight*= get_visibility(world_pos, vert,vert_world);
						sum_weight += weight;
						acc_coeffs(coeffs, vert, weight);
					}
				}
			}
		}
	}

	if (sum_weight>0)
	{
		for (int i=0; i<9; i++) coeffs[i]/=sum_weight;
		return shGetIrradianceAt(normal, coeffs);
	}

	return vec3(0.0);
}


#if !HAS_REFLECTION_MAP
vec3 getRadiance(in vec3 world_pos, in vec3 reflectVec, float roughness)
{
	return getIrradiance(world_pos, reflectVec) * RECIPROCAL_PI;
}
#endif

#endif

)";

static std::string g_frag_part2 =
R"(
#if HAS_LOD_PROBE_GRID
layout (std140, binding = BINDING_LOD_PROBE_GRID) uniform ProbeGrid
{
	vec4 uCoverageMin;
	vec4 uCoverageMax;
	ivec4 uBaseDivisions;	
	int uSubDivisionLevel;
	float uDiffuseThresh;
	float uDiffuseHigh;
	float uDiffuseLow;
	float uSpecularThresh;
	float uSpecularHigh;
	float uSpecularLow;
};


layout (std430, binding = BINDING_LOD_PROBES) buffer Probes
{
	vec4 bProbeData[];
};


layout (std430, binding = BINDING_LOD_PROBES) buffer Probes
{
	vec4 bProbeData[];
};

layout (std430, binding = BINDING_LOD_PROBE_INDICES) buffer ProbeIndex
{
	int bIndexData[];
};

int get_probe_lod_i(in ivec3 ipos)
{	
	if (uSubDivisionLevel<1) return 0;

	ivec3 ipos_base = ipos / (1<<uSubDivisionLevel);
	int idx_base = ipos_base.x + (ipos_base.y + ipos_base.z * uBaseDivisions.y) * uBaseDivisions.x;
	int idx_sub = bIndexData[idx_base];
	int base_offset = uBaseDivisions.x * uBaseDivisions.y * uBaseDivisions.z;
	
	int lod = 0;	
	int digit_mask = 1 << (uSubDivisionLevel -1);
	while(lod<uSubDivisionLevel && idx_sub>=0)
	{
		if (lod<uSubDivisionLevel-1)
		{
			int offset = base_offset + idx_sub*8;
			int sub = 0;
			if ((ipos.x & digit_mask) !=0) sub+=1;
			if ((ipos.y & digit_mask) !=0) sub+=2;
			if ((ipos.z & digit_mask) !=0) sub+=4;
			idx_sub = bIndexData[offset + sub];
		}
		else
		{
			idx_sub = -1;
		}
		lod++;
		digit_mask >>=1;		
	}	
	return lod;
}

float get_probe_lod_f(in vec3 pos)
{
	ivec3 divs = uBaseDivisions.xyz * (1<<uSubDivisionLevel);	
	vec3 size_grid = uCoverageMax.xyz - uCoverageMin.xyz;
	vec3 pos_normalized = (pos - uCoverageMin.xyz)/size_grid;	
	vec3 pos_voxel = pos_normalized * vec3(divs) - vec3(0.5);
	pos_voxel = clamp(pos_voxel, vec3(0.0), vec3(divs) - vec3(1.0));
	ivec3 i_voxel = clamp(ivec3(pos_voxel), ivec3(0), ivec3(divs) - ivec3(2));
	vec3 frac_voxel = pos_voxel - vec3(i_voxel);

	float acc_lod = 0.0;
	float acc_weight = 0.0;
	for (int z=0;z<2;z++)
	{
		for (int y=0;y<2;y++)
		{
			for (int x=0;x<2;x++)
			{				
				ivec3 vert = i_voxel + ivec3(x,y,z);
				vec3 w = vec3(1.0) - abs(vec3(x,y,z) - frac_voxel);
				float weight = w.x * w.y * w.z;
				if (weight>0.0)
				{
					acc_lod += float(get_probe_lod_i(vert)) * weight;
					acc_weight += weight;					
				}				
			}
		}
	}
	return acc_lod/acc_weight;
}

int get_probe_idx_lod(in ivec3 ipos, int target_lod)
{
	ivec3 ipos_base = ipos / (1<<target_lod);
	int probe_idx = ipos_base.x + (ipos_base.y + ipos_base.z * uBaseDivisions.y) * uBaseDivisions.x;
	if (uSubDivisionLevel<1 || target_lod < 1) return probe_idx;

	int idx_sub = bIndexData[probe_idx];
	int base_offset = uBaseDivisions.x * uBaseDivisions.y * uBaseDivisions.z;

	int lod = 0;
	int digit_mask = 1 << (uSubDivisionLevel -1);
	while(lod<target_lod && idx_sub>=0)
	{
		int offset = base_offset + idx_sub*8;
		int sub = 0;
		if ((ipos.x & digit_mask) !=0) sub+=1;
		if ((ipos.y & digit_mask) !=0) sub+=2;
		if ((ipos.z & digit_mask) !=0) sub+=4;
		probe_idx = offset + sub;
		
		if (lod < uSubDivisionLevel -1)
		{
			idx_sub = bIndexData[probe_idx];
		}
		else
		{
			idx_sub = -1;
		}
		lod++;
		digit_mask >>=1;
	}
	return probe_idx;
}

void acc_coeffs(inout vec4 coeffs[9], int idx, in float weight)
{
	int offset = idx*10 + 1;
	for (int i=0;i<9;i++)
	{
		coeffs[i]+=bProbeData[offset+i]*weight;
	}
}

vec3 getIrradiance(in vec3 world_pos, in vec3 normal)
{
	float lod = get_probe_lod_f(world_pos);
	int i_lod = int(lod);
	float frac_lod = lod - float(i_lod);

	float sum_weight = 0.0;
	vec4 coeffs[9];
	for (int i=0; i<9; i++) coeffs[i] = vec4(0.0);

	vec3 dx = dFdx(world_pos);
	vec3 dy = dFdy(world_pos);
	vec3 N = normalize(cross(dx, dy));

	vec3 size_grid = uCoverageMax.xyz - uCoverageMin.xyz;
	vec3 pos_normalized = (world_pos - uCoverageMin.xyz)/size_grid;

	// lod 0
	{
		ivec3 divs = uBaseDivisions.xyz * (1<<i_lod);
		vec3 pos_voxel = pos_normalized * vec3(divs) - vec3(0.5);
		pos_voxel = clamp(pos_voxel, vec3(0.0), vec3(divs) - vec3(1.0));
		ivec3 i_voxel = clamp(ivec3(pos_voxel), ivec3(0), ivec3(divs) - ivec3(2));
		vec3 frac_voxel = pos_voxel - vec3(i_voxel);

		for (int z=0;z<2;z++)
		{
			for (int y=0;y<2;y++)
			{
				for (int x=0;x<2;x++)
				{
					ivec3 vert = i_voxel + ivec3(x,y,z);
					vec3 vert_normalized = (vec3(vert) + vec3(0.5))/vec3(divs);
					vec3 vert_world = vert_normalized * size_grid + uCoverageMin.xyz;
					vec3 dir = normalize(vert_world - world_pos);
					
					if (dot(dir, N)>=0.0)
					{					
						vec3 w = vec3(1.0) - abs(vec3(x,y,z) - frac_voxel);
						float weight = w.x * w.y * w.z * (1.0 - frac_lod); 
						if (weight>0.0)
						{
							int idx_probe = get_probe_idx_lod(vert, i_lod);
							sum_weight += weight;
							acc_coeffs(coeffs, idx_probe, weight);
						}
					}
				}
			}
		}
	}
	// lod 1
	if (frac_lod > 0.0)
	{
		i_lod++;
		ivec3 divs = uBaseDivisions.xyz * (1<<i_lod);
		vec3 pos_voxel = pos_normalized * vec3(divs) - vec3(0.5);
		pos_voxel = clamp(pos_voxel, vec3(0.0), vec3(divs) - vec3(1.0));
		ivec3 i_voxel = clamp(ivec3(pos_voxel), ivec3(0), ivec3(divs) - ivec3(2));
		vec3 frac_voxel = pos_voxel - vec3(i_voxel);

		for (int z=0;z<2;z++)
		{
			for (int y=0;y<2;y++)
			{
				for (int x=0;x<2;x++)
				{
					ivec3 vert = i_voxel + ivec3(x,y,z);
					vec3 vert_normalized = (vec3(vert) + vec3(0.5))/vec3(divs);
					vec3 vert_world = vert_normalized * size_grid + uCoverageMin.xyz;
					vec3 dir = normalize(vert_world - world_pos);
					
					if (dot(dir, N)>=0.0)
					{					
						vec3 w = vec3(1.0) - abs(vec3(x,y,z) - frac_voxel);
						float weight = w.x * w.y * w.z * frac_lod; 
						if (weight>0.0)
						{
							int idx_probe = get_probe_idx_lod(vert, i_lod);
							sum_weight += weight;
							acc_coeffs(coeffs, idx_probe, weight);
						}
					}
				}
			}
		}	
	}

	if (sum_weight>0.0)
	{
		for (int i=0; i<9; i++) coeffs[i]/=sum_weight;
		return shGetIrradianceAt(normal, coeffs);
	}

	return vec3(0.0);
}

#if !HAS_REFLECTION_MAP
vec3 getRadiance(in vec3 world_pos, in vec3 reflectVec, float roughness)
{
	return getIrradiance(world_pos, reflectVec) * RECIPROCAL_PI;
}
#endif

#endif

#if HAS_AMBIENT_LIGHT
layout (std140, binding = BINDING_AMBIENT_LIGHT) uniform AmbientLight
{
	vec4 uAmbientColor;
	float uDiffuseThresh;
	float uDiffuseHigh;
	float uDiffuseLow;
	float uSpecularThresh;
	float uSpecularHigh;
	float uSpecularLow;
};

vec3 getIrradiance(in vec3 world_pos, in vec3 normal)
{
	return uAmbientColor.xyz * PI;
}

#if !HAS_REFLECTION_MAP
vec3 getRadiance(in vec3 world_pos, in vec3 reflectVec, float roughness)
{
	return uAmbientColor.xyz;
}
#endif

#endif

#if HAS_HEMISPHERE_LIGHT
layout (std140, binding = BINDING_HEMISPHERE_LIGHT) uniform HemisphereLight
{
	vec4 uHemisphereSkyColor;
	vec4 uHemisphereGroundColor;
	float uDiffuseThresh;
	float uDiffuseHigh;
	float uDiffuseLow;
	float uSpecularThresh;
	float uSpecularHigh;
	float uSpecularLow;
};

vec3 HemisphereColor(in vec3 dir)
{
	float k = dir.y * 0.5 + 0.5;
	return mix( uHemisphereGroundColor.xyz, uHemisphereSkyColor.xyz, k);
}

vec3 getIrradiance(in vec3 world_pos, in vec3 normal)
{
	return HemisphereColor(normal) * PI;
}

#if !HAS_REFLECTION_MAP
vec3 getRadiance(in vec3 world_pos, in vec3 reflectVec, float roughness)
{
	return HemisphereColor(reflectVec);
}
#endif

#endif

#if HAS_FOG
layout (std140, binding = BINDING_FOG) uniform FOG
{
	vec4 fog_rgba;
};
#endif

layout (location = 0) in vec2 vPosProj;
layout (location = 0) out vec4 outColor;

vec3 get_norm(in vec3 pos)
{
	vec3 min_pos = - uSize.xyz*uSpacing.xyz *0.5;
	vec3 max_pos = uSize.xyz*uSpacing.xyz *0.5;

	vec3 delta;
	{
		vec3 pos1 = pos+vec3(uSpacing.x, 0.0, 0.0);
		vec3 coord1 = (pos1 - min_pos)/(max_pos-min_pos);
		vec3 pos2 = pos-vec3(uSpacing.x, 0.0, 0.0);
		vec3 coord2 = (pos2 - min_pos)/(max_pos-min_pos);
		delta.x = texture(uTex, coord1).x -  texture(uTex, coord2).x;
	}
	{
		vec3 pos1 = pos+vec3(0.0, uSpacing.y, 0.0);
		vec3 coord1 = (pos1 - min_pos)/(max_pos-min_pos);
		vec3 pos2 = pos-vec3(0.0, uSpacing.y, 0.0);
		vec3 coord2 = (pos2 - min_pos)/(max_pos-min_pos);
		delta.y = texture(uTex, coord1).x -  texture(uTex, coord2).x;
	}
	{
		vec3 pos1 = pos+vec3(0.0, 0.0, uSpacing.z);
		vec3 coord1 = (pos1 - min_pos)/(max_pos-min_pos);
		vec3 pos2 = pos-vec3(0.0, 0.0, uSpacing.z);
		vec3 coord2 = (pos2 - min_pos)/(max_pos-min_pos);
		delta.z = texture(uTex, coord1).x -  texture(uTex, coord2).x;
	}

	vec3 norm = -delta/uSpacing.xyz;	
	vec4 world_norm = uNormalMat * vec4(norm, 0.0);
	return normalize(world_norm.xyz);
}

vec3 get_shading(in vec3 pos)
{
	vec4 pos_world = uModelMat * vec4(pos, 1.0);
	vec3 viewDir = normalize(uEyePos - pos_world.xyz);
	vec3 norm = get_norm(pos);
	
	PhysicalMaterial material;
	material.diffuseColor = uColor.xyz * ( 1.0 - uMetallicFactor );	
	material.roughness = max( uRoughnessFactor, 0.0525 );	
	material.specularColor = mix( vec3( 0.04 ), uColor.xyz, uMetallicFactor );	

	vec3 dxy = max(abs(dFdx(norm)), abs(dFdy(norm)));
	float geometryRoughness = max(max(dxy.x, dxy.y), dxy.z);	
	material.roughness += geometryRoughness;
	material.roughness = min( material.roughness, 1.0 );
	material.specularF90 = 1.0;

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
			DirectionalShadow shadow = uDirectionalShadows[shadow_id];
			l_shadow = computeShadowCoef(shadow.VPSBMat, uDirectionalShadowTex[shadow_id], pos_world.xyz);
			shadow_id++;
		}
#endif

#if HAS_FOG
		if (l_shadow>0.0)
		{
			float zEye = -dot(pos_world.xyz - light_source.origin.xyz, light_source.direction.xyz);
			if (zEye>0.0)
			{
				float att = pow(1.0 - fog_rgba.w, zEye);
				l_shadow *= att;
			}
		}		
#endif
		IncidentLight directLight = IncidentLight(light_source.color.xyz * l_shadow, light_source.direction.xyz, true);

		float dotNL =  saturate(dot(norm, directLight.direction));
		vec3 irradiance = dotNL * directLight.color;
		diffuse += irradiance * BRDF_Lambert( material.diffuseColor );
		specular += irradiance * BRDF_GGX( directLight.direction, viewDir, norm, material.specularColor, material.specularF90, material.roughness );
	}
#endif

#if HAS_INDIRECT_LIGHT
	{
		vec3 reflectVec = reflect(-viewDir, norm);
		reflectVec = normalize( mix( reflectVec, norm, material.roughness * material.roughness) );

		vec3 irradiance = getIrradiance(pos_world.xyz, norm);
		vec3 radiance = getRadiance(pos_world.xyz, reflectVec, material.roughness);

		diffuse += material.diffuseColor * irradiance * RECIPROCAL_PI;
		specular +=  material.specularColor * radiance;
	}
#endif

	vec3 col = specular + diffuse;
	//col = clamp(col, 0.0, 1.0);				
	return col;
}

void main()
{
	ivec2 coord = ivec2(gl_FragCoord.xy);		
#if MSAA
	float depth = texelFetch(uDepthTex, coord, gl_SampleID).x*2.0-1.0;
#else
	float depth = texelFetch(uDepthTex, coord, 0).x*2.0-1.0;
#endif
	vec4 pos_view = uInvProjMat * vec4(vPosProj, depth, 1.0);
	pos_view *= 1.0/pos_view.w;
	vec4 pos_world = uInvViewMat * pos_view;
	vec3 pos_model = (uInvModelMat * pos_world).xyz;
	vec3 eye_pos = (uInvModelMat * vec4(uEyePos, 1.0)).xyz;
	vec3 dir = normalize(pos_model - eye_pos);	
	float dis = length(pos_model - eye_pos);

	vec3 min_pos = - uSize.xyz*uSpacing.xyz *0.5;
	vec3 max_pos = uSize.xyz*uSpacing.xyz *0.5;
	
	vec3 t_min;
	t_min.x = (min_pos.x - eye_pos.x)/dir.x;
	t_min.y = (min_pos.y - eye_pos.y)/dir.y;
	t_min.z = (min_pos.z - eye_pos.z)/dir.z;

	vec3 t_max;
	t_max.x = (max_pos.x - eye_pos.x)/dir.x;
	t_max.y = (max_pos.y - eye_pos.y)/dir.y;
	t_max.z = (max_pos.z - eye_pos.z)/dir.z;

	vec3 t0_start;
	t0_start.x = min(t_min.x, t_max.x);
	t0_start.y = min(t_min.y, t_max.y);
	t0_start.z = min(t_min.z, t_max.z);
	float t_start = max(max(t0_start.x, t0_start.y), t0_start.z);

	vec3 t1_stop;
	t1_stop.x = max(t_min.x, t_max.x);
	t1_stop.y = max(t_min.y, t_max.y);
	t1_stop.z = max(t_min.z, t_max.z);
	float t_stop = min(min(t1_stop.x, t1_stop.y), t1_stop.z);

	if (t_stop<t_start) discard;
	if (t_stop>dis) t_stop = dis;

	float tMaxX_b,tMaxY_b,tMaxZ_b;
	float tDeltaX_b,tDeltaY_b,tDeltaZ_b;	

	int x_b,y_b,z_b;
	int stepX,stepY,stepZ; 

	float t1 = t_start;
	vec3 pos1 = ((eye_pos + t1*dir) - min_pos) / uSpacing.xyz;

	if (dir.x==0.0)
	{
		x_b = (int(ceil(pos1.x - 0.5)) - 1)/uBsize.x;
		stepX = 0;
		tMaxX_b = t_stop;
	}
	else if (dir.x<0.0)
	{
		x_b = (int(ceil(pos1.x - 0.5)) - 1)/uBsize.x;
		stepX = -1;
		tMaxX_b = ((float(x_b * uBsize.x) + 0.5)*uSpacing.x + min_pos.x - eye_pos.x)/dir.x;
		tDeltaX_b = -float(uBsize.x)*uSpacing.x/dir.x;		
	}
	else
	{
		x_b = int(floor(pos1.x - 0.5))/uBsize.x;
		stepX = 1;
		tMaxX_b = ( (float((x_b + 1) * uBsize.x) + 0.5)*uSpacing.x + min_pos.x - eye_pos.x)/dir.x;
		tDeltaX_b = float(uBsize.x)*uSpacing.x /dir.x;		
	}

	if (dir.y==0.0)
	{
		y_b = (int(ceil(pos1.y - 0.5)) - 1)/uBsize.y;
		stepY = 0;
		tMaxY_b = t_stop;
	}
	else if (dir.y <0.0)
	{
		y_b = (int(ceil(pos1.y -0.5)) - 1)/uBsize.y;
		stepY = -1;
		tMaxY_b = ( (float(y_b * uBsize.y) + 0.5)*uSpacing.y+ min_pos.y - eye_pos.y)/dir.y;
		tDeltaY_b = -float(uBsize.y)*uSpacing.y/dir.y;		
	}
	else
	{
		y_b = int(floor(pos1.y-0.5))/uBsize.y;
		stepY = 1;
		tMaxY_b = ( (float((y_b + 1) * uBsize.y) + 0.5)*uSpacing.y+ min_pos.y - eye_pos.y)/dir.y;
		tDeltaY_b = float(uBsize.y)*uSpacing.y/dir.y;		
	}

	if (dir.z==0.0)
	{
		z_b = (int(ceil(pos1.z - 0.5)) -1)/uBsize.z;
		stepZ = 0;
		tMaxZ_b = t_stop;
	}
	else if (dir.z < 0.0)
	{
		z_b = (int(ceil(pos1.z - 0.5)) - 1)/uBsize.z;
		stepZ = -1;
		tMaxZ_b = ( (float(z_b * uBsize.z) + 0.5)*uSpacing.z+ min_pos.z - eye_pos.z)/dir.z;
		tDeltaZ_b = -float(uBsize.z)*uSpacing.z/dir.z;		
	}
	else
	{
		z_b = int(floor(pos1.z - 0.5))/uBsize.z;
		stepZ = 1;
		tMaxZ_b = ( (float((z_b + 1)*uBsize.z) + 0.5)*uSpacing.z+ min_pos.z - eye_pos.z)/dir.z;
		tDeltaZ_b = float(uBsize.z)*uSpacing.z/dir.z;		
	}

	bool hit = false;
	vec3 pos;

	while(t1<t_stop)
	{	
		float t_b;
		while(t1<t_stop)
		{	
			vec2 MinMax = texelFetch(uGrid, ivec3(x_b,y_b,z_b), 0).xy;					
			if (tMaxX_b<tMaxY_b)
			{
				if (tMaxX_b<tMaxZ_b) 
				{
					t_b = tMaxX_b;
					tMaxX_b+=tDeltaX_b;
					x_b+=stepX;
				}
				else
				{
					t_b = tMaxZ_b;
					tMaxZ_b+=tDeltaZ_b;
					z_b+=stepZ;
				}			
			}
			else
			{
				if (tMaxY_b<tMaxZ_b) 
				{
					t_b = tMaxY_b;
					tMaxY_b+=tDeltaY_b;
					y_b+=stepY;
				}
				else 
				{
					t_b = tMaxZ_b;
					tMaxZ_b+=tDeltaZ_b;
					z_b+=stepZ;
				}
			}
			
			if ( MinMax.y>=uIsovalue && MinMax.x<=uIsovalue) break;			
			
			t1=t_b;
		}

		if (t1>=t_stop) break;		
		if (t_b>t_stop) t_b=t_stop;
		
		float t = t1;
		pos = eye_pos + t*dir;
		vec3 coord = (pos - min_pos)/(max_pos-min_pos);
		float v0 = texture(uTex, coord).x;	

		while(true)
		{
			t+=uStep;

			pos = eye_pos + t*dir;
			coord = (pos - min_pos)/(max_pos-min_pos);
			float v1 = texture(uTex, coord).x;
			if ((v0<=uIsovalue && v1>=uIsovalue) || (v0>=uIsovalue && v1<=uIsovalue))
			{
				float k = (uIsovalue - v1)/(v1-v0);
				t += k*uStep;
				pos = eye_pos + t*dir;
				hit = true;
				break;					
			}

			v0 = v1;
			if (t>=t_b) break;	
		}

		if (hit) break;	

		t1=t_b;
		
	}

	if (hit)
	{
		vec3 col = get_shading(pos);
		outColor = vec4(col, 1.0);

		pos_world = uModelMat * vec4(pos, 1.0);
		pos_view = uViewMat * pos_world;
		vec4 pos_proj = uProjMat * pos_view;
		pos_proj*= 1.0/pos_proj.w;				
		gl_FragDepth = pos_proj.z * 0.5 + 0.5;
		return;
	}
	
	discard;
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

DrawIsosurface::DrawIsosurface(const Options& options) : m_options(options)
{
	std::string defines = "";
	if (options.msaa)
	{
		defines += "#define MSAA 1\n";
	}
	else
	{
		defines += "#define MSAA 0\n";
	}

	{
		char line[64];
		sprintf(line, "#define NUM_DIRECTIONAL_LIGHTS %d\n", options.num_directional_lights);
		defines += line;
	}

	if (options.num_directional_lights > 0)
	{
		m_bindings.binding_directional_lights = 2;
		{
			char line[64];
			sprintf(line, "#define BINDING_DIRECTIONAL_LIGHTS %d\n", m_bindings.binding_directional_lights);
			defines += line;
		}
	}
	else
	{
		m_bindings.binding_directional_lights = 1;
	}


	{
		char line[64];
		sprintf(line, "#define NUM_DIRECTIONAL_SHADOWS %d\n", options.num_directional_shadows);
		defines += line;
	}

	m_bindings.location_tex_directional_shadow = 2 + options.num_directional_shadows;

	if (options.num_directional_shadows > 0)
	{
		m_bindings.binding_directional_shadows = m_bindings.binding_directional_lights + 1;
		{
			char line[64];
			sprintf(line, "#define BINDING_DIRECTIONAL_SHADOWS %d\n", m_bindings.binding_directional_shadows);
			defines += line;
		}
		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_DIRECTIONAL_SHADOW %d\n", m_bindings.location_tex_directional_shadow - options.num_directional_shadows + 1);
			defines += line;
		}
	}
	else
	{
		m_bindings.binding_directional_shadows = m_bindings.binding_directional_lights;
	}

	bool has_indirect_light = options.has_environment_map || options.has_probe_grid || options.has_lod_probe_grid || options.has_ambient_light || options.has_hemisphere_light;
	if (has_indirect_light)
	{
		defines += "#define HAS_INDIRECT_LIGHT 1\n";
	}
	else
	{
		defines += "#define HAS_INDIRECT_LIGHT 0\n";
	}

	if (options.has_reflection_map)
	{
		defines += "#define HAS_REFLECTION_MAP 1\n";
		m_bindings.location_tex_reflection_map = m_bindings.location_tex_directional_shadow + 1;
		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_REFLECTION_MAP %d\n", m_bindings.location_tex_reflection_map);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_REFLECTION_MAP 0\n";
		m_bindings.location_tex_reflection_map = m_bindings.location_tex_directional_shadow;
	}

	if (options.has_environment_map)
	{
		defines += "#define HAS_ENVIRONMENT_MAP 1\n";
		m_bindings.binding_environment_map = m_bindings.binding_directional_shadows + 1;

		{
			char line[64];
			sprintf(line, "#define BINDING_ENVIRONMEN_MAP %d\n", m_bindings.binding_environment_map);
			defines += line;
		}

	}
	else
	{
		defines += "#define HAS_ENVIRONMENT_MAP 0\n";
		m_bindings.binding_environment_map = m_bindings.binding_directional_shadows;
	}

	if (options.has_probe_grid)
	{
		defines += "#define HAS_PROBE_GRID 1\n";
		m_bindings.binding_probe_grid = m_bindings.binding_environment_map + 1;
		m_bindings.binding_probes = m_bindings.binding_probe_grid + 1;
		m_bindings.binding_probe_visibility = m_bindings.binding_probes + 1;

		{
			char line[64];
			sprintf(line, "#define BINDING_PROBE_GRID %d\n", m_bindings.binding_probe_grid);
			defines += line;
		}

		{
			char line[64];
			sprintf(line, "#define BINDING_PROBES %d\n", m_bindings.binding_probes);
			defines += line;
		}

		{
			char line[64];
			sprintf(line, "#define BINDING_PROBE_VISIBILITY %d\n", m_bindings.binding_probe_visibility);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_PROBE_GRID 0\n";
		m_bindings.binding_probe_grid = m_bindings.binding_environment_map;
		m_bindings.binding_probes = m_bindings.binding_environment_map;
		m_bindings.binding_probe_visibility = m_bindings.binding_environment_map;
	}

	if (options.probe_reference_recorded)
	{
		defines += "#define PROBE_REFERENCE_RECORDED 1\n";
		m_bindings.binding_probe_references = m_bindings.binding_probe_visibility + 1;

		{
			char line[64];
			sprintf(line, "#define BINDING_PROBE_REFERENCES %d\n", m_bindings.binding_probe_references);
			defines += line;
		}
	}
	else
	{
		defines += "#define PROBE_REFERENCE_RECORDED 0\n";
		m_bindings.binding_probe_references = m_bindings.binding_probe_visibility;
	}

	if (options.has_lod_probe_grid)
	{
		defines += "#define HAS_LOD_PROBE_GRID 1\n";
		m_bindings.binding_lod_probe_grid = m_bindings.binding_probe_references + 1;
		m_bindings.binding_lod_probes = m_bindings.binding_lod_probe_grid + 1;
		m_bindings.binding_lod_probe_indices = m_bindings.binding_lod_probes + 1;

		{
			char line[64];
			sprintf(line, "#define BINDING_LOD_PROBE_GRID %d\n", m_bindings.binding_lod_probe_grid);
			defines += line;
		}

		{
			char line[64];
			sprintf(line, "#define BINDING_LOD_PROBES %d\n", m_bindings.binding_lod_probes);
			defines += line;
		}

		{
			char line[64];
			sprintf(line, "#define BINDING_LOD_PROBE_INDICES %d\n", m_bindings.binding_lod_probe_indices);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_LOD_PROBE_GRID 0\n";
		m_bindings.binding_lod_probe_grid = m_bindings.binding_probe_references;
		m_bindings.binding_lod_probes = m_bindings.binding_probe_references;
		m_bindings.binding_lod_probe_indices = m_bindings.binding_probe_references;
	}

	if (options.has_ambient_light)
	{
		defines += "#define HAS_AMBIENT_LIGHT 1\n";
		m_bindings.binding_ambient_light = m_bindings.binding_lod_probe_indices + 1;
		{
			char line[64];
			sprintf(line, "#define BINDING_AMBIENT_LIGHT %d\n", m_bindings.binding_ambient_light);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_AMBIENT_LIGHT 0\n";
		m_bindings.binding_ambient_light = m_bindings.binding_lod_probe_indices;
	}

	if (options.has_hemisphere_light)
	{
		defines += "#define HAS_HEMISPHERE_LIGHT 1\n";
		m_bindings.binding_hemisphere_light = m_bindings.binding_ambient_light + 1;
		{
			char line[64];
			sprintf(line, "#define BINDING_HEMISPHERE_LIGHT %d\n", m_bindings.binding_hemisphere_light);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_HEMISPHERE_LIGHT 0\n";
		m_bindings.binding_hemisphere_light = m_bindings.binding_ambient_light;
	}

	if (options.has_fog)
	{
		defines += "#define HAS_FOG 1\n";
		m_bindings.binding_fog = m_bindings.binding_hemisphere_light + 1;
		{
			char line[64];
			sprintf(line, "#define BINDING_FOG %d\n", m_bindings.binding_fog);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_FOG 0\n";
		m_bindings.binding_fog = m_bindings.binding_hemisphere_light;
	}

	std::string s_frag = g_frag_part0 + g_frag_part1+ g_frag_part2;
	replace(s_frag, "#DEFINES#", defines.c_str());

    GLShader vert_shader(GL_VERTEX_SHADER, g_vertex.c_str());
    GLShader frag_shader(GL_FRAGMENT_SHADER, s_frag.c_str());
    m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(vert_shader, frag_shader));
}


inline void toViewAABB(const glm::mat4& MV, const glm::vec3& min_pos, const glm::vec3& max_pos, glm::vec3& min_pos_out, glm::vec3& max_pos_out)
{
	glm::vec4 view_pos[8];
	view_pos[0] = MV * glm::vec4(min_pos.x, min_pos.y, min_pos.z, 1.0f);
	view_pos[1] = MV * glm::vec4(max_pos.x, min_pos.y, min_pos.z, 1.0f);
	view_pos[2] = MV * glm::vec4(min_pos.x, max_pos.y, min_pos.z, 1.0f);
	view_pos[3] = MV * glm::vec4(max_pos.x, max_pos.y, min_pos.z, 1.0f);
	view_pos[4] = MV * glm::vec4(min_pos.x, min_pos.y, max_pos.z, 1.0f);
	view_pos[5] = MV * glm::vec4(max_pos.x, min_pos.y, max_pos.z, 1.0f);
	view_pos[6] = MV * glm::vec4(min_pos.x, max_pos.y, max_pos.z, 1.0f);
	view_pos[7] = MV * glm::vec4(max_pos.x, max_pos.y, max_pos.z, 1.0f);

	min_pos_out = { FLT_MAX, FLT_MAX, FLT_MAX };
	max_pos_out = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

	for (int k = 0; k < 8; k++)
	{
		glm::vec4 pos = view_pos[k];
		if (pos.x < min_pos_out.x) min_pos_out.x = pos.x;
		if (pos.x > max_pos_out.x) max_pos_out.x = pos.x;
		if (pos.y < min_pos_out.y) min_pos_out.y = pos.y;
		if (pos.y > max_pos_out.y) max_pos_out.y = pos.y;
		if (pos.z < min_pos_out.z) min_pos_out.z = pos.z;
		if (pos.z > max_pos_out.z) max_pos_out.z = pos.z;
	}
}

inline void calc_scissor(const Camera* camera, const VolumeIsosurfaceModel* model, float width, float height, glm::ivec2& origin, glm::ivec2& size)
{
	origin = { 0,0 };
	size = { 0,0 };

	glm::vec3 min_pos, max_pos;
	model->m_data->GetMinMax(min_pos, max_pos);

	glm::mat4 MV = camera->matrixWorldInverse * model->matrixWorld;
	glm::vec3 min_pos_view, max_pos_view;
	toViewAABB(MV, min_pos, max_pos, min_pos_view, max_pos_view);

	glm::mat4 invP = camera->projectionMatrixInverse;
	glm::vec4 view_far = invP * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	view_far /= view_far.w;
	glm::vec4 view_near = invP * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
	view_near /= view_near.w;

	if (min_pos_view.z < view_far.z)
	{
		min_pos_view.z = view_far.z;
	}

	if (max_pos_view.z > view_near.z)
	{
		max_pos_view.z = view_near.z;
	}

	if (min_pos_view.z > max_pos_view.z) return;

	glm::mat4 P = camera->projectionMatrix;

	glm::vec4 min_pos_proj = P * glm::vec4(min_pos_view.x, min_pos_view.y, min_pos_view.z, 1.0f);
	min_pos_proj /= min_pos_proj.w;

	glm::vec4 max_pos_proj = P * glm::vec4(max_pos_view.x, max_pos_view.y, min_pos_view.z, 1.0f);
	max_pos_proj /= max_pos_proj.w;

	glm::vec4 min_pos_proj2 = P * glm::vec4(min_pos_view.x, min_pos_view.y, max_pos_view.z, 1.0f);
	min_pos_proj2 /= min_pos_proj2.w;

	glm::vec4 max_pos_proj2 = P * glm::vec4(max_pos_view.x, max_pos_view.y, max_pos_view.z, 1.0f);
	max_pos_proj2 /= max_pos_proj2.w;

	glm::vec2 min_proj = glm::min(min_pos_proj, min_pos_proj2);
	glm::vec2 max_proj = glm::max(max_pos_proj, max_pos_proj2);

	if (min_proj.x < -1.0f) min_proj.x = -1.0f;
	if (min_proj.y < -1.0f) min_proj.y = -1.0f;
	if (max_proj.x > 1.0f) max_proj.x = 1.0f;
	if (max_proj.y > 1.0f) max_proj.y = 1.0f;

	if (min_proj.x > max_proj.x || min_proj.y > max_proj.y) return;

	glm::vec2 min_screen = (glm::vec2(min_proj) + 1.0f) * 0.5f * glm::vec2(width, height);
	glm::vec2 max_screen = (glm::vec2(max_proj) + 1.0f) * 0.5f * glm::vec2(width, height);
	
	origin.x = (int)(min_screen.x + 0.5f);
	origin.y = (int)(min_screen.y + 0.5f);

	size.x = (int)(max_screen.x + 0.5f) - origin.x;
	size.y = (int)(max_screen.y + 0.5f) - origin.y;

}

void DrawIsosurface::render(const RenderParams& params)
{
	GLint i_viewport[4];
	glGetIntegerv(GL_VIEWPORT, i_viewport);	

	glm::ivec2 origin, size;
	calc_scissor(params.camera, params.model, (float)i_viewport[2], (float)i_viewport[3], origin, size);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	if (m_options.msaa)
	{
		glEnable(GL_SAMPLE_SHADING);
	}

	glEnable(GL_SCISSOR_TEST);
	glScissor(origin.x, origin.y, size.x, size.y);

	glUseProgram(m_prog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.camera->m_constant.m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.model->m_constant.m_id);

	if (m_options.num_directional_lights > 0)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_directional_lights, params.lights->constant_directional_lights->m_id);
	}

	if (m_options.num_directional_shadows > 0)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_directional_shadows, params.lights->constant_directional_shadows->m_id);
	}

	if (m_options.has_environment_map)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_environment_map, params.lights->environment_map->m_constant.m_id);
	}

	if (m_options.has_ambient_light)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_ambient_light, params.lights->ambient_light->m_constant.m_id);
	}

	if (m_options.has_probe_grid)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_probe_grid, params.lights->probe_grid->m_constant.m_id);
		if (params.lights->probe_grid->m_probe_buf != nullptr)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindings.binding_probes, params.lights->probe_grid->m_probe_buf->m_id);
		}
		if (params.lights->probe_grid->m_visibility_buf != nullptr)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindings.binding_probe_visibility, params.lights->probe_grid->m_visibility_buf->m_id);
		}
		if (m_options.probe_reference_recorded)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindings.binding_probe_references, params.lights->probe_grid->m_ref_buf->m_id);
		}
	}

	if (m_options.has_lod_probe_grid)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_lod_probe_grid, params.lights->lod_probe_grid->m_constant.m_id);
		if (params.lights->lod_probe_grid->m_probe_buf != nullptr)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindings.binding_lod_probes, params.lights->lod_probe_grid->m_probe_buf->m_id);
		}
		if (params.lights->lod_probe_grid->m_sub_index_buf != nullptr)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindings.binding_lod_probe_indices, params.lights->lod_probe_grid->m_sub_index_buf->m_id);
		}
	}


	if (m_options.has_hemisphere_light)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_hemisphere_light, params.lights->hemisphere_light->m_constant.m_id);
	}

	if (m_options.has_fog)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_fog, params.constant_fog->m_id);
	}

	const GLTexture3D& tex = params.model->m_data->texture;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, tex.tex_id);
	glUniform1i(0, 0);

	const GLTexture3D& grid = params.model->m_partition->minmax_texture;
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, grid.tex_id);
	glUniform1i(1, 1);

	glActiveTexture(GL_TEXTURE2);
	if (m_options.msaa)
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, params.tex_depth->tex_id);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, params.tex_depth->tex_id);
	}
	glUniform1i(2, 2);

	if (m_options.num_directional_shadows > 0)
	{
		int start_idx = m_bindings.location_tex_directional_shadow - m_options.num_directional_shadows + 1;
		std::vector<int> values(m_options.num_directional_shadows);
		for (int i = 0; i < m_options.num_directional_shadows; i++)
		{
			glActiveTexture(GL_TEXTURE0 + start_idx + i);
			glBindTexture(GL_TEXTURE_2D, params.lights->directional_shadow_texs[i]);
			values[i] = start_idx + i;
		}
		glUniform1iv(start_idx, m_options.num_directional_shadows, values.data());
	}

	if (m_options.has_reflection_map)
	{
		glActiveTexture(GL_TEXTURE0 + m_bindings.location_tex_reflection_map);
		glBindTexture(GL_TEXTURE_CUBE_MAP, params.lights->reflection_map->tex_id);
		glUniform1i(m_bindings.location_tex_reflection_map, m_bindings.location_tex_reflection_map);
	}

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glUseProgram(0);

	glDisable(GL_SCISSOR_TEST);

	if (m_options.msaa)
	{
		glDisable(GL_SAMPLE_SHADING);
	}

}

