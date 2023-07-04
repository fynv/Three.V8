#include <GL/glew.h>
#include "models/ModelComponents.h"
#include "lights/DirectionalLight.h"
#include "StandardRoutine.h"
#include "cameras/Camera.h"
#include "cameras/Reflector.h"

static std::string g_vertex =
R"(#version 430

#DEFINES#

layout (location = LOCATION_ATTRIB_POS) in vec3 aPos;
layout (location = LOCATION_ATTRIB_NORM) in vec3 aNorm;

layout (std140, binding = BINDING_CAMERA) uniform Camera
{
	mat4 uProjMat;
	mat4 uViewMat;	
	mat4 uInvProjMat;
	mat4 uInvViewMat;	
	vec3 uEyePos;
};

layout (std140, binding = BINDING_MODEL) uniform Model
{
	mat4 uModelMat;
	mat4 uNormalMat;
};

layout (location = LOCATION_VARYING_VIEWDIR) out vec3 vViewDir;
layout (location = LOCATION_VARYING_NORM) out vec3 vNorm;

#if HAS_COLOR
layout (location = LOCATION_ATTRIB_COLOR) in vec4 aColor;
layout (location = LOCATION_VARYING_COLOR) out vec4 vColor;
#endif

#if HAS_UV
layout (location = LOCATION_ATTRIB_UV) in vec2 aUV;
layout (location = LOCATION_VARYING_UV) out vec2 vUV;
#endif

#if HAS_LIGHTMAP
layout (location = LOCATION_ATTRIB_ATLAS_UV) in vec2 aAtlasUV;
layout (location = LOCATION_VARYING_ATLAS_UV) out vec2 vAtlasUV;
#endif

#if HAS_NORMAL_MAP
layout (location = LOCATION_ATTRIB_TANGENT) in vec3 aTangent;
layout (location = LOCATION_VARYING_TANGENT) out vec3 vTangent;
layout (location = LOCATION_ATTRIB_BITANGENT) in vec3 aBitangent;
layout (location = LOCATION_VARYING_BITANGENT) out vec3 vBitangent;
#endif

layout (location = LOCATION_VARYING_WORLD_POS) out vec3 vWorldPos;

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

#if HAS_LIGHTMAP
	vAtlasUV = aAtlasUV;
#endif

#if HAS_NORMAL_MAP
	vec4 world_tangent = uModelMat * vec4(aTangent, 0.0);
	vTangent =  world_tangent.xyz;

	vec4 world_bitangent = uModelMat * vec4(aBitangent, 0.0);
	vBitangent =  world_bitangent.xyz;
#endif
}
)";

static std::string g_frag_part0 =
R"(#version 430

#DEFINES#

#if IS_REFLECT
layout (std140, binding = BINDING_CAMERA) uniform Camera
{
	mat4 uProjMat;
	mat4 uViewMat;	
	mat4 uInvProjMat;
	mat4 uInvViewMat;	
	vec3 uEyePos;
};

layout (std140, binding = BINDING_MATRIX_REFLECTOR) uniform MatrixReflector
{
	mat4 uMatrixReflector;
};
#endif

#define EPSILON 1e-6
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

uint seed;
float jitter;

vec2 vogel_sample(float j, float N)
{
	float r = sqrt((j+0.5)/N);
	float theta = j * 2.4 + jitter * 2.0 * PI;
	return vec2(r * cos(theta), r * sin(theta));
}

vec3 N;

layout (location = LOCATION_VARYING_VIEWDIR) in vec3 vViewDir;
layout (location = LOCATION_VARYING_NORM) in vec3 vNorm;

layout (std140, binding = BINDING_MATERIAL) uniform Material
{
	vec4 uColor;
	vec4 uEmissive;
	vec4 uSpecularGlossiness;
	vec2 uNormalScale;
	float uMetallicFactor;
	float uRoughnessFactor;
	float uAlphaCutoff;
	int uDoubleSided;
};

#if HAS_COLOR
layout (location = LOCATION_VARYING_COLOR) in vec4 vColor;
#endif

#if HAS_UV
layout (location = LOCATION_VARYING_UV) in vec2 vUV;
#endif

#if HAS_LIGHTMAP
layout (location = LOCATION_VARYING_ATLAS_UV) in vec2 vAtlasUV;
#endif

#if HAS_COLOR_TEX
layout (location = LOCATION_TEX_COLOR) uniform sampler2D uTexColor;
#endif

#if HAS_METALNESS_MAP
layout (location = LOCATION_TEX_METALNESS) uniform sampler2D uTexMetalness;
#endif

#if HAS_ROUGHNESS_MAP
layout (location = LOCATION_TEX_ROUGHNESS) uniform sampler2D uTexRoughness;
#endif

#if HAS_NORMAL_MAP
layout (location = LOCATION_TEX_NORMAL) uniform sampler2D uTexNormal;
layout (location = LOCATION_VARYING_TANGENT) in vec3 vTangent;
layout (location = LOCATION_VARYING_BITANGENT) in vec3 vBitangent;
#endif

layout (location = LOCATION_VARYING_WORLD_POS) in vec3 vWorldPos;

#if HAS_EMISSIVE_MAP
layout (location = LOCATION_TEX_EMISSIVE) uniform sampler2D uTexEmissive;
#endif

#if HAS_SPECULAR_MAP
layout (location = LOCATION_TEX_SPECULAR) uniform sampler2D uTexSpecular;
#endif

#if HAS_GLOSSINESS_MAP
layout (location = LOCATION_TEX_GLOSSINESS) uniform sampler2D uTexGlossiness;
#endif

struct IncidentLight {
	vec3 color;
	vec3 direction;
	bool visible;
};

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
)";

static std::string g_frag_part1 =
R"(
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
layout (location = LOCATION_TEX_DIRECTIONAL_SHADOW_DEPTH) uniform sampler2D uDirectionalShadowTexDepth[NUM_DIRECTIONAL_SHADOWS];

vec3 computeShadowCoords(in mat4 VPSB)
{
	vec4 shadowCoords = VPSB * vec4(vWorldPos, 1.0);
	return shadowCoords.xyz;
}

float borderDepthTexture(sampler2D shadowTex, vec2 uv)
{
	return ((uv.x <= 1.0) && (uv.y <= 1.0) &&
	 (uv.x >= 0.0) && (uv.y >= 0.0)) ? textureLod(shadowTex, uv, 0.0).x : 1.0;
}

float borderPCFTexture(sampler2DShadow shadowTex, vec3 uvz)
{
	return ((uvz.x <= 1.0) && (uvz.y <= 1.0) &&
	 (uvz.x >= 0.0) && (uvz.y >= 0.0)) ? texture(shadowTex, uvz) : 
	 ((uvz.z <= 1.0) ? 1.0 : 0.0);
}


float computeShadowCoef(in mat4 VPSB, sampler2DShadow shadowTex)
{
	vec3 shadowCoords;
	shadowCoords = computeShadowCoords(VPSB);
	return borderPCFTexture(shadowTex, shadowCoords);
}

// Returns average blocker depth in the search region, as well as the number of found blockers.
// Blockers are defined as shadow-map samples between the surface point and the light.
void findBlocker(
	sampler2D shadowTex,
    out float accumBlockerDepth, 
    out float numBlockers,
    out float maxBlockers,
    vec2 uv,
    float z,
    vec2 searchRegionRadiusUV)
{
    accumBlockerDepth = 0.0;
    numBlockers = 0.0;
	maxBlockers = 300.0;

    {
        maxBlockers = 32.0;
        for (int i = 0; i < 32; ++i)
        {
            vec2 offset = vogel_sample(float(i), 32.0) * searchRegionRadiusUV;
            float shadowMapDepth = borderDepthTexture(shadowTex, uv + offset);
            if (shadowMapDepth < z)
            {
                accumBlockerDepth += shadowMapDepth;
                numBlockers++;
            }
        }
    }

}

float zClipToEye(in DirectionalShadow shadow, float z)
{
    return (shadow.nearFar.x + (shadow.nearFar.y - shadow.nearFar.x) * z);
}

// Using similar triangles between the area light, the blocking plane and the surface point
vec2 penumbraRadiusUV(vec2 light_radius_uv, float zReceiver, float zBlocker)
{
    return light_radius_uv * (zReceiver - zBlocker);
}


// Performs PCF filtering on the shadow map using multiple taps in the filter region.
float pcfFilter(sampler2DShadow shadowTex, vec2 uv, float z, vec2 filterRadiusUV)
{
    float sum = 0.0;
    {
        for (int i = 0; i < 64; ++i)
        {
            vec2 offset = vogel_sample(float(i), 64.0)  * filterRadiusUV;        
            sum += borderPCFTexture(shadowTex, vec3(uv + offset, z));
        }
        return sum / 64.0;
    }
}

float pcssShadow( sampler2DShadow shadowTex, sampler2D shadowTexDepth, in DirectionalShadow shadow, vec2 uv, float z, float zEye)
{
    // ------------------------
    // STEP 1: blocker search
    // ------------------------
    float accumBlockerDepth, numBlockers, maxBlockers;
    
    vec2 frustum_size = vec2(shadow.leftRight.y - shadow.leftRight.x, shadow.bottomTop.y - shadow.bottomTop.x);
    vec2 light_radius_uv = vec2(shadow.lightRadius) / frustum_size;
    vec2 searchRegionRadiusUV = light_radius_uv* (zEye - shadow.nearFar.x);
	findBlocker(shadowTexDepth, accumBlockerDepth, numBlockers, maxBlockers, uv, z, searchRegionRadiusUV);

    // Early out if not in the penumbra
    if (numBlockers == 0.0)
        return 1.0;

    // ------------------------
    // STEP 2: penumbra size
    // ------------------------
    float avgBlockerDepth = accumBlockerDepth / numBlockers;
    float avgBlockerDepthWorld = zClipToEye(shadow, avgBlockerDepth);
    
    vec2 penumbraRadius = penumbraRadiusUV(light_radius_uv, zEye, avgBlockerDepthWorld);
    
	return pcfFilter(shadowTex, uv, z, penumbraRadius);
}


float computePCSSShadowCoef(in DirectionalShadow shadow, sampler2DShadow shadowTex, sampler2D shadowTexDepth)
{	
	vec3 uvz = computeShadowCoords(shadow.VPSBMat);	
	float zEye = -(shadow.viewMat * vec4(vWorldPos, 1.0)).z;
	return pcssShadow(shadowTex, shadowTexDepth, shadow, uvz.xy, uvz.z, zEye);
}

#endif
)";

static std::string g_frag_part2 =
R"(
#if HAS_LIGHTMAP
layout (location = LOCATION_TEX_LIGHTMAP) uniform sampler2D uTexLightmap;
#endif 

#if HAS_REFLECTOR
layout (std140, binding = BINDING_REFLECTOR_CAMERA) uniform ReflectorCamera
{
	mat4 uReflProjMat;
	mat4 uReflViewMat;	
	mat4 uReflInvProjMat;
	mat4 uReflInvViewMat;	
	vec3 uReflEyePos;
};

layout (location = LOCATION_TEX_REFLECTOR) uniform sampler2D uTexReflector;
layout (location = LOCATION_TEX_REFLECTOR_DEPTH) uniform sampler2D uTexReflectorDepth;

vec3 getRadiance(in vec3 worldPos, in vec3 reflectVec, float roughness, in vec3 irradiance)
{
	vec4 dir_view = uReflViewMat * vec4(reflectVec, 0.0);
	vec4 proj = uReflProjMat * vec4(dir_view.xyz, 1.0);
	proj*= 1.0/proj.w;
	vec2 uv = vec2((proj.x + 1.0)*0.5, (proj.y + 1.0)*0.5);
	vec3 rad = textureLod(uTexReflector, uv, 0.0).xyz;

	if (roughness > 0.053)
	{
		vec3 rad2 = irradiance * RECIPROCAL_PI;
		float lum1 = luminance(rad);
		if (lum1>0.0)
		{
			float lum2 = luminance(rad2);			
			float r2 = roughness*roughness;
			float r4 = r2*r2;
			float gloss = log(2.0/r4 - 1.0)/log(2.0)/18.0;
			rad *= gloss + lum2/lum1 * (1.0-gloss);
		}
	}
	return rad;
}
#endif

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
#if HAS_REFLECTION_MAP && !HAS_REFLECTOR

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

vec3 getReflRadiance(in vec3 reflectVec, float roughness)
{
	return GetReflectionAt(reflectVec, uReflectionMap, roughness);
}

#if HAS_REFLECTION_DISTANCE

layout (location = LOCATION_TEX_REFLECTION_DISTANCE) uniform samplerCube uReflectionDis;

vec3 getReflectionDir(in vec3 dir, in vec3 norm)
{
	vec3 refl_dir = reflect(dir, norm);	
	float proj = dot(refl_dir, dir);
	if (proj >= 1.0) return dir;
	else if (proj <= -1.0) return -dir;
	
	vec3 dir_y = normalize(refl_dir - proj * dir);

	float max_alpha = acos(proj);
	float sin_max_alpha = sin(max_alpha);
	float dis0 = textureLod(uReflectionDis, dir, 0.0).x;
	
	float delta0 = 0.0;
	vec3 v0 = dis0 * dir;	

	float step = max(PI / 100.0, max_alpha/30.0);
	float start = RandomFloat(seed) * step;

	for (float alpha = start; alpha < max_alpha - 0.0001; alpha += step)
	{
		float ray_dis = sin_max_alpha * dis0 / sin(max_alpha -alpha);
		vec3 sample_dir = cos(alpha)*dir + sin(alpha)*dir_y;
		float sample_dis = textureLod(uReflectionDis, sample_dir, 0.0).x;
		float delta = ray_dis - sample_dis;
		vec3 v = ray_dis * sample_dir;
		if (delta>=0.0) 
		{
			float k = -delta0/(delta - delta0);			
			return normalize((1.0 - k) * v0 + k *v);
		}
		delta0 = delta;
		v0 = v;
		
	}

	return refl_dir;
}

#endif

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

vec3 getIrradiance(in vec3 normal)
{
	return shGetIrradianceAt(normal, uSHCoefficients);
}

#endif

#if HAS_PROBE_GRID
layout (std140, binding = BINDING_PROBE_GRID) uniform ProbeGrid
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

#if PROBE_REFERENCE_RECORDED
layout (std430, binding = BINDING_PROBE_REFERENCES) buffer ProbeReferences
{
	uint bReferenced[];
};
#endif

#endif

#if HAS_LOD_PROBE_GRID
layout (std140, binding = BINDING_LOD_PROBE_GRID) uniform ProbeGrid
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


layout (std430, binding = BINDING_LOD_PROBES) buffer Probes
{
	vec4 bProbeData[];
};

layout (std430, binding = BINDING_LOD_PROBE_INDICES) buffer ProbeIndex
{
	int bIndexData[];
};

#endif


#if HAS_PROBE_GRID || HAS_LOD_PROBE_GRID

layout (location = LOCATION_TEX_IRRADIANCE) uniform sampler2D uTexIrr;
layout (location = LOCATION_TEX_VISIBILITY) uniform sampler2D uTexVis;

vec2 signNotZero(in vec2 v)
{
	return vec2((v.x >= 0.0) ? 1.0 : -1.0, (v.y >= 0.0)? 1.0: -1.0);
}

vec2 vec3_to_oct(in vec3 v)
{
	vec2 p = v.xy * (1.0/ (abs(v.x) + abs(v.y) + abs(v.z)));
	return (v.z <= 0.0) ? ((1.0 - abs(p.yx)) * signNotZero(p)) : p;
}

vec3 get_irradiance_common(int idx, in vec3 dir)
{
	vec2 probe_uv = vec3_to_oct(dir)*0.5 + 0.5;
	int pack_x = idx % uPackSize;
	int pack_y = idx / uPackSize;
	vec2 uv = (vec2(pack_x, pack_y) * float(uIrrRes + 2) + (probe_uv * float(uIrrRes) + 1.0))/float(uIrrPackRes);
	return texture(uTexIrr, uv).xyz;
}

vec2 get_mean_dis_common(in vec3 dir, int idx)
{
	vec2 probe_uv = vec3_to_oct(dir)*0.5 + 0.5;
	int pack_x = idx % uPackSize;
	int pack_y = idx / uPackSize;
	vec2 uv = (vec2(pack_x, pack_y) * float(uVisRes + 2) + (probe_uv * float(uVisRes) + 1.0))/float(uPackRes);
	return texture(uTexVis, uv).xy;
}

float get_visibility_common(in vec3 wpos, int idx, in vec3 vert_world, float scale)
{		
	vec3 dir = wpos - vert_world;
	float dis = length(dir);
	dir = normalize(dir);	
	
	vec2 mean_dis_var = get_mean_dis_common(dir, idx);
	float mean_dis = mean_dis_var.x;
	float mean_var = mean_dis_var.y;
	float mean_var2 = mean_var * mean_var;
	float delta = max(dis - mean_dis, 0.0);
	float delta2 = delta*delta * scale * scale;	
	return mean_var2/(mean_var2 + delta2);
}
#endif

#if HAS_PROBE_GRID

float get_visibility(in vec3 wpos, in ivec3 vert, in vec3 vert_world)
{
	int idx = vert.x + (vert.y + vert.z*uDivisions.y)*uDivisions.x;
	return get_visibility_common(wpos, idx, vert_world, 1.0);
}

vec3 get_irradiance(in ivec3 vert, in vec3 dir)
{
	int idx = vert.x + (vert.y + vert.z*uDivisions.y)*uDivisions.x;
#if PROBE_REFERENCE_RECORDED
	bReferenced[idx] = 1;
#endif
	return get_irradiance_common(idx, dir);
}

vec3 getIrradiance(in vec3 normal)
{	
	vec3 viewDir = normalize(vViewDir);
	vec3 wpos = vWorldPos + (N + 3.0 * viewDir) * uNormalBias;
	
	vec3 size_grid = uCoverageMax.xyz - uCoverageMin.xyz;
	vec3 pos_normalized = (wpos - uCoverageMin.xyz)/size_grid;
	pos_normalized.y = pow(pos_normalized.y, 1.0/uYpower);	
	vec3 pos_voxel = pos_normalized * vec3(uDivisions) - vec3(0.5);
	pos_voxel = clamp(pos_voxel, vec3(0.0), vec3(uDivisions) - vec3(1.0));
	
	ivec3 i_voxel = clamp(ivec3(pos_voxel), ivec3(0), ivec3(uDivisions) - ivec3(2));
	vec3 frac_voxel = pos_voxel - vec3(i_voxel);

	float sum_weight = 0.0;
	vec3 irr = vec3(0.0);

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
				vec3 dir = normalize(vert_world - vWorldPos);
				float dotDirN = dot(dir, N);
				float k = 0.9;
				dotDirN = (k*dotDirN + sqrt(1.0 - (1.0-dotDirN*dotDirN)*k*k))/(k+1.0);
				float weight = dotDirN * get_visibility(wpos, vert,vert_world);	

				const float crushThreshold = 0.2;
				if (weight < crushThreshold) {
					weight *= weight * weight / (crushThreshold*crushThreshold); 
				}

				vec3 w = vec3(1.0) - abs(vec3(x,y,z) - frac_voxel);
				weight *= w.x * w.y * w.z;		
				if (weight> 0.0)
				{					
					sum_weight += weight;
					irr += get_irradiance(vert, normal) * weight;			
				}
			}
		}
	}	

	if (sum_weight>0.0)
	{
		return irr/sum_weight;
	}	

	return vec3(0.0);
}

#endif

)";

static std::string g_frag_part3 =
R"(
#if HAS_LOD_PROBE_GRID


float get_visibility(in vec3 wpos, int idx, int lod, in vec3 vert_world)
{	
	float scale = float(1<<(uSubDivisionLevel -lod));
	return get_visibility_common(wpos, idx, vert_world, scale);
}

int get_probe_idx(in ivec3 ipos)
{
	int base_offset = uBaseDivisions.x * uBaseDivisions.y * uBaseDivisions.z;

	ivec3 ipos_base = ipos / (1<<uSubDivisionLevel);
	int node_idx = ipos_base.x + (ipos_base.y + ipos_base.z * uBaseDivisions.y) * uBaseDivisions.x;
	int probe_idx = bIndexData[node_idx];

	int lod = 0;
	int digit_mask = 1 << (uSubDivisionLevel -1);
	while(lod<uSubDivisionLevel && probe_idx>=uNumProbes)
	{		
		int offset = base_offset + (probe_idx - uNumProbes)*8;
		int sub = 0;
		if ((ipos.x & digit_mask) !=0) sub+=1;
		if ((ipos.y & digit_mask) !=0) sub+=2;
		if ((ipos.z & digit_mask) !=0) sub+=4;
		node_idx = offset + sub;
		probe_idx = bIndexData[node_idx];

		lod++;
		digit_mask >>=1;	
	}

	return probe_idx;
}

vec3 getIrradiance(in vec3 normal)
{
	vec3 viewDir = normalize(vViewDir);		
	vec3 wpos = vWorldPos + (N + 3.0 * viewDir) * uNormalBias;

	ivec3 divs = uBaseDivisions.xyz * (1<<uSubDivisionLevel);

	vec3 size_grid = uCoverageMax.xyz - uCoverageMin.xyz;
	vec3 pos_normalized = (wpos - uCoverageMin.xyz)/size_grid;	
	vec3 pos_voxel = pos_normalized * vec3(divs) - vec3(0.5);
	pos_voxel = clamp(pos_voxel, vec3(0.0), vec3(divs) - vec3(1.0));

	ivec3 i_voxel = clamp(ivec3(pos_voxel), ivec3(0), ivec3(divs) - ivec3(2));
	vec3 frac_voxel = pos_voxel - vec3(i_voxel);
	
	vec3 irr = vec3(0.0);
	float sum_weight = 0.0;
	
	for (int z=0;z<2;z++)
	{
		for (int y=0;y<2;y++)
		{
			for (int x=0;x<2;x++)
			{				
				ivec3 vert = i_voxel + ivec3(x,y,z);
				int idx_probe = get_probe_idx(vert);
				vec4 pos_lod = bProbeData[idx_probe];
				vec3 probe_world = pos_lod.xyz;
				vec3 dir = normalize(probe_world - vWorldPos);					
				float dotDirN = dot(dir, N);
				float k = 0.9;
				dotDirN = (k*dotDirN + sqrt(1.0 - (1.0-dotDirN*dotDirN)*k*k))/(k+1.0);				
				float weight = dotDirN * get_visibility(wpos, idx_probe, int(pos_lod.w), probe_world);

				const float crushThreshold = 0.2;
				if (weight < crushThreshold) {
					weight *= weight * weight / (crushThreshold*crushThreshold); 
				}

				vec3 w = vec3(1.0) - abs(vec3(x,y,z) - frac_voxel);
				weight *= w.x * w.y * w.z;
				if (weight>0.0)
				{					
					vec3 sample_dir = normal;										

					// parallax correction
					float distance = get_mean_dis_common(sample_dir, idx_probe).x;					
					vec3 pos_to = wpos + distance * sample_dir;
					sample_dir = normalize(pos_to - probe_world);
					
					sum_weight += weight;
					irr += get_irradiance_common(idx_probe, sample_dir) * weight;
				}
			}
		}
	}
	
	if (sum_weight > 0.0)	
	{
		return irr/sum_weight;
	}
	return vec3(0.0);
}

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

vec3 getIrradiance(in vec3 normal)
{
	return uAmbientColor.xyz * PI;
}
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

vec3 getIrradiance(in vec3 normal)
{
	return HemisphereColor(normal) * PI;
}
#endif


#if HAS_REFLECTION_MAP && !HAS_REFLECTOR
vec3 getRadiance(in vec3 worldPos, in vec3 reflectVec, float roughness, in vec3 irradiance)
{
	vec3 rad = getReflRadiance(reflectVec, roughness);

	if (roughness > 0.053)
	{
		vec3 rad2 = irradiance * RECIPROCAL_PI;
		float lum1 = luminance(rad);
		if (lum1>0.0)
		{
			float lum2 = luminance(rad2);			
			float r2 = roughness*roughness;
			float r4 = r2*r2;
			float gloss = log(2.0/r4 - 1.0)/log(2.0)/18.0;
			rad *= gloss + lum2/lum1 * (1.0-gloss);
		}
	}
	return rad;
}
#endif 

#if HAS_FOG
layout (std140, binding = BINDING_FOG) uniform FOG
{
	vec4 fog_rgba;
};
#endif

#if USE_SSAO
layout (location = LOCATION_TEX_SSAO) uniform sampler2D uTexSSAO;
#endif 

layout (location = 0) out vec4 out0;

#if ALPHA_BLEND
layout (location = 1) out float out1;
#endif

void main()
{	
	seed = InitRandomSeed(uint(gl_FragCoord.x), uint(gl_FragCoord.y)); 
	jitter = RandomFloat(seed);

	vec4 base_color = uColor;
#if HAS_COLOR
	base_color *= vColor;
#endif

	float tex_alpha = 1.0;

#if HAS_COLOR_TEX
	vec4 tex_color = texture(uTexColor, vUV);
	tex_alpha = tex_color.w;
	base_color *= tex_color;
#endif

#if ALPHA_MASK
	base_color.w = base_color.w > uAlphaCutoff ? 1.0 : 0.0;
#endif

#if SPECULAR_GLOSSINESS

	vec3 specularFactor = uSpecularGlossiness.xyz;
#if HAS_SPECULAR_MAP	
	specularFactor *= texture( uTexSpecular, vUV ).xyz;
#endif
	float glossinessFactor = uSpecularGlossiness.w;
#if HAS_GLOSSINESS_MAP
	glossinessFactor *= texture( uTexGlossiness, vUV ).w;
#endif

#else

	float metallicFactor = uMetallicFactor;
	float roughnessFactor = uRoughnessFactor;

#if HAS_METALNESS_MAP
	metallicFactor *= texture(uTexMetalness, vUV).z;
#endif

#if HAS_ROUGHNESS_MAP
	roughnessFactor *= texture(uTexRoughness, vUV).y;
#endif

#endif

	vec3 viewDir = normalize(vViewDir);
	vec3 norm = normalize(vNorm);	

#if HAS_NORMAL_MAP
	if (length(vTangent)>0.0 && length(vBitangent)>0.0)
	{
		vec3 T = normalize(vTangent);
		vec3 B = normalize(vBitangent);
		vec3 bump =  texture(uTexNormal, vUV).xyz;
		bump = (2.0 * bump - 1.0) * vec3(uNormalScale.x, uNormalScale.y, 1.0);
		norm = normalize(bump.x*T + bump.y*B + bump.z*norm);
	}
#endif	

	if (uDoubleSided!=0 && !gl_FrontFacing)
	{		
		norm = -norm;
	}
	
	vec3 dxy = max(abs(dFdx(norm)), abs(dFdy(norm)));
	float geometryRoughness = max(max(dxy.x, dxy.y), dxy.z);	
	
	vec3 dx = dFdx(vWorldPos);
	vec3 dy = dFdy(vWorldPos);
	N = normalize(cross(dx, dy));

#if ALPHA_MASK || ALPHA_BLEND
	if (base_color.w == 0.0) discard;
#endif

#if IS_REFLECT
	vec4 pos_refl_eye = uMatrixReflector * vec4(uEyePos, 1.0);
	vec4 pos_refl_frag = uMatrixReflector * vec4(vWorldPos, 1.0);
	if (pos_refl_eye.z * pos_refl_frag.z > 0.0) discard;    
#endif

	PhysicalMaterial material;

#if SPECULAR_GLOSSINESS
	material.diffuseColor = base_color.xyz * ( 1.0 -
                          max( max( specularFactor.r, specularFactor.g ), specularFactor.b ) );
	material.roughness = max( 1.0 - glossinessFactor, 0.0525 );	
	material.specularColor = specularFactor.rgb;
#else
	material.diffuseColor = base_color.xyz * ( 1.0 - metallicFactor );	
	material.roughness = max( roughnessFactor, 0.0525 );	
	material.specularColor = mix( vec3( 0.04 ), base_color.xyz, metallicFactor );	
#endif

	bool has_specular = length(material.specularColor)>0.0;	
	material.roughness += geometryRoughness;
	material.roughness = min( material.roughness, 1.0 );
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
			DirectionalShadow shadow = uDirectionalShadows[shadow_id];
			if (shadow.lightRadius>0.0)
			{
				l_shadow = computePCSSShadowCoef(shadow, uDirectionalShadowTex[shadow_id], uDirectionalShadowTexDepth[shadow_id]);
			}
			else
			{
				l_shadow = computeShadowCoef(shadow.VPSBMat, uDirectionalShadowTex[shadow_id]);
			}
			shadow_id++;
		}
#endif

#if HAS_FOG
		if (l_shadow>0.0)
		{
			float zEye = -dot(vWorldPos - light_source.origin.xyz, light_source.direction.xyz);
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

#if (TONE_SHADING & 1) == 0
		diffuse += irradiance * BRDF_Lambert( material.diffuseColor );
#else
		float diffuse_thresh = light_source.diffuse_thresh;
		float diffuse_high = light_source.diffuse_high;
		float diffuse_low = light_source.diffuse_low;

		vec3 diffuse_light = irradiance * BRDF_Lambert_Toon();
		float lum_diffuse = luminance(diffuse_light);
		if (lum_diffuse > diffuse_thresh)
		{
			diffuse_light *= diffuse_high/lum_diffuse;			
		}
		else if (lum_diffuse>0.0)
		{
			diffuse_light *= diffuse_low/lum_diffuse;
		}
		else
		{
			diffuse_light = vec3(diffuse_low);
		}

		diffuse += diffuse_light* material.diffuseColor;
#endif

		if (has_specular)
		{
#if (TONE_SHADING & 2) == 0
			specular += irradiance * BRDF_GGX( directLight.direction, viewDir, norm, material.specularColor, material.specularF90, material.roughness );
#else
			float specular_thresh = light_source.specular_thresh;
			float specular_high =  light_source.specular_high;
			float specular_low =  light_source.specular_low;

			vec3 specular_light = irradiance * BRDF_GGX_Toon(directLight.direction, viewDir, norm, material.roughness);
			float lum_specular = luminance(specular_light);
			if (lum_specular > specular_thresh)
			{
				specular_light *= specular_high/lum_specular;
			}
			else if (lum_specular>0.0)
			{
				specular_light *= specular_low/lum_specular;
			}
			else
			{
				specular_light = vec3(specular_low);
			}
			specular += specular_light* material.specularColor;
#endif
		}
	}
#endif

#if HAS_LIGHTMAP	
	vec4 lm = texture(uTexLightmap, vAtlasUV);
	vec3 light_color = lm.w>0.0 ? lm.xyz/lm.w : vec3(0.0);
	diffuse += material.diffuseColor * light_color;

	if (has_specular)
	{
#if HAS_REFLECTION_MAP	|| HAS_REFLECTOR
#if HAS_REFLECTION_DISTANCE && !HAS_REFLECTOR
		vec3 reflectVec = getReflectionDir(-viewDir, norm);
#else	
		vec3 reflectVec = reflect(-viewDir, norm);	
#endif // HAS_REFLECTION_DISTANCE
		reflectVec = normalize( mix( reflectVec, norm, material.roughness * material.roughness) );			
		vec3 radiance = getRadiance(vWorldPos, reflectVec, material.roughness, light_color * PI);
		specular +=  material.specularColor * radiance;
#else
		specular += material.specularColor * light_color;
#endif // HAS_REFLECTION_MAP
	}
	
#elif HAS_INDIRECT_LIGHT
	{
		vec3 irradiance = getIrradiance(norm);
		vec3 radiance = vec3(0.0);

		if (has_specular)
		{
#if HAS_REFLECTION_MAP || HAS_REFLECTOR

#if HAS_REFLECTION_DISTANCE && !HAS_REFLECTOR
			vec3 reflectVec = getReflectionDir(-viewDir, norm);
#else
			vec3 reflectVec = reflect(-viewDir, norm);		
#endif // HAS_REFLECTION_DISTANCE
			reflectVec = normalize( mix( reflectVec, norm, material.roughness * material.roughness) );		
			radiance = getRadiance(vWorldPos, reflectVec, material.roughness, irradiance);

#else
			radiance = irradiance * RECIPROCAL_PI;
#endif  // HAS_REFLECTION_MAP
		}

#if USE_SSAO && IS_OPAQUE
		{
			ivec2 coord = ivec2(gl_FragCoord.xy);	
			float ao = texelFetch(uTexSSAO, coord, 0).x;
			ao = pow(ao,2.0);
			irradiance *= ao;
			radiance *= ao;
		}
#endif

#if (TONE_SHADING & 4) == 0
		diffuse += material.diffuseColor * irradiance * RECIPROCAL_PI;
#else
		float diffuse_thresh = uDiffuseThresh;
		float diffuse_high = uDiffuseHigh;
		float diffuse_low = uDiffuseLow;
		
		vec3 diffuse_light =  irradiance * BRDF_Lambert_Toon();
		float lum_diffuse = luminance(diffuse_light);
		if (lum_diffuse > diffuse_thresh)
		{
			diffuse_light *= diffuse_high/lum_diffuse;			
		}
		else if (lum_diffuse>0.0)
		{
			diffuse_light *= diffuse_low/lum_diffuse;
		}
		else
		{
			diffuse_light = vec3(diffuse_low);
		}
		
		diffuse += diffuse_light* material.diffuseColor;
#endif

		if (has_specular)
		{
#if (TONE_SHADING & 8) == 0
			specular +=  material.specularColor * radiance;
#else		
			float specular_thresh = uSpecularThresh;
			float specular_high = uSpecularHigh;
			float specular_low = uSpecularLow;

			vec3 specular_light = radiance;
			float lum_specular = luminance(specular_light);
			if (lum_specular > specular_thresh)
			{
				specular_light *= specular_high/lum_specular;
			}
			else if (lum_specular>0.0)
			{
				specular_light *= specular_low/lum_specular;
			}
			else
			{
				specular_light = vec3(specular_low);
			}
			specular += specular_light* material.specularColor;
#endif
		}
	}
#endif

	vec3 col = emissive + specular;
#if !IS_HIGHTLIGHT
	col += diffuse;
#endif
//	col = clamp(col, 0.0, 1.0);	

#if ALPHA_BLEND
	float alpha = base_color.w;
	float a = min(1.0, alpha) * 8.0 + 0.01;
	float b = -gl_FragCoord.z * 0.95 + 1.0;
	float weight = clamp(a * a * a * 1e8 * b * b * b, 1e-2, 3e2);
	out0 = vec4(col * alpha, alpha) * weight;
	out1 = alpha;
#elif IS_HIGHTLIGHT
	out0 = vec4(col*tex_alpha, 0.0);
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

void StandardRoutine::s_generate_shaders(const Options& options, Bindings& bindings, std::string& s_vertex, std::string& s_frag)
{
	s_vertex = g_vertex;
	s_frag = g_frag_part0 + g_frag_part1 + g_frag_part2 + g_frag_part3;

	std::string defines = "";

	if (options.has_lightmap)
	{
		defines += "#define HAS_LIGHTMAP 1\n";
	}
	else
	{
		defines += "#define HAS_LIGHTMAP 0\n";
	}

	{
		bindings.location_attrib_pos = 0;
		{
			char line[64];
			sprintf(line, "#define LOCATION_ATTRIB_POS %d\n", bindings.location_attrib_pos);
			defines += line;
		}
	}

	{
		bindings.location_attrib_norm = bindings.location_attrib_pos+1;
		{
			char line[64];
			sprintf(line, "#define LOCATION_ATTRIB_NORM %d\n", bindings.location_attrib_norm);
			defines += line;
		}
	}

	{
		bindings.binding_camera = 0;
		{
			char line[64];
			sprintf(line, "#define BINDING_CAMERA %d\n", bindings.binding_camera);
			defines += line;
		}
	}

	if (options.is_reflect)
	{
		defines += "#define IS_REFLECT 1\n";
		bindings.binding_matrix_reflector = bindings.binding_camera + 1;
		{
			char line[64];
			sprintf(line, "#define BINDING_MATRIX_REFLECTOR %d\n", bindings.binding_matrix_reflector);
			defines += line;
		}

	}
	else
	{
		defines += "#define IS_REFLECT 0\n";
		bindings.binding_matrix_reflector = bindings.binding_camera;
	}

	{
		bindings.binding_model = bindings.binding_matrix_reflector + 1;
		{
			char line[64];
			sprintf(line, "#define BINDING_MODEL %d\n", bindings.binding_model);
			defines += line;
		}
	}

	{
		bindings.location_varying_viewdir = 0;
		{
			char line[64];
			sprintf(line, "#define LOCATION_VARYING_VIEWDIR %d\n", bindings.location_varying_viewdir);
			defines += line;
		}
	}

	{
		bindings.location_varying_norm = bindings.location_varying_viewdir + 1;
		{
			char line[64];
			sprintf(line, "#define LOCATION_VARYING_NORM %d\n", bindings.location_varying_norm);
			defines += line;
		}
	}

	if (options.alpha_mode == AlphaMode::Opaque)
	{
		defines += "#define IS_OPAQUE 1\n";
	}
	else
	{
		defines += "#define IS_OPAQUE 0\n";
	}

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

	if (options.specular_glossiness)
	{
		defines += "#define SPECULAR_GLOSSINESS 1\n";
	}
	else
	{
		defines += "#define SPECULAR_GLOSSINESS 0\n";
	}

	{
		bindings.binding_material = bindings.binding_model + 1;
		{
			char line[64];
			sprintf(line, "#define BINDING_MATERIAL %d\n", bindings.binding_material);
			defines += line;
		}
	}

	if (options.has_color)
	{
		defines += "#define HAS_COLOR 1\n";
		bindings.location_attrib_color = bindings.location_attrib_norm + 1;
		bindings.location_varying_color = bindings.location_varying_norm + 1;

		{
			char line[64];
			sprintf(line, "#define LOCATION_ATTRIB_COLOR %d\n", bindings.location_attrib_color);
			defines += line;
		}

		{
			char line[64];
			sprintf(line, "#define LOCATION_VARYING_COLOR %d\n", bindings.location_varying_color);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_COLOR 0\n";
		bindings.location_attrib_color = bindings.location_attrib_norm;
		bindings.location_varying_color = bindings.location_varying_norm;
	}

	bool has_uv = options.has_color_texture || options.has_metalness_map || options.has_roughness_map 
		|| options.has_normal_map || options.has_emissive_map || options.has_specular_map || options.has_glossiness_map;

	if (has_uv)
	{
		defines += "#define HAS_UV 1\n";
		bindings.location_attrib_uv = bindings.location_attrib_color + 1;
		bindings.location_varying_uv = bindings.location_varying_color + 1;
		{
			char line[64];
			sprintf(line, "#define LOCATION_ATTRIB_UV %d\n", bindings.location_attrib_uv);
			defines += line;
		}
		{
			char line[64];
			sprintf(line, "#define LOCATION_VARYING_UV %d\n", bindings.location_varying_uv);
			defines += line;
		}		
	}
	else
	{
		defines += "#define HAS_UV 0\n";
		bindings.location_attrib_uv = bindings.location_attrib_color;
		bindings.location_varying_uv = bindings.location_varying_color;
	}

	if (options.has_lightmap)
	{
		bindings.location_attrib_atlas_uv = bindings.location_attrib_uv + 1;
		bindings.location_varying_atlas_uv = bindings.location_varying_uv + 1;
		{
			char line[64];
			sprintf(line, "#define LOCATION_ATTRIB_ATLAS_UV %d\n", bindings.location_attrib_atlas_uv);
			defines += line;
		}
		{
			char line[64];
			sprintf(line, "#define LOCATION_VARYING_ATLAS_UV %d\n", bindings.location_varying_atlas_uv);
			defines += line;
		}
	}
	else
	{
		bindings.location_attrib_atlas_uv = bindings.location_attrib_uv;
		bindings.location_varying_atlas_uv = bindings.location_varying_uv;
	}
	
	if (options.has_color_texture)
	{
		defines += "#define HAS_COLOR_TEX 1\n";

		bindings.location_tex_color = 0;
		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_COLOR %d\n", bindings.location_tex_color);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_COLOR_TEX 0\n";
		bindings.location_tex_color = -1;
	}

	if (options.has_metalness_map)
	{
		defines += "#define HAS_METALNESS_MAP 1\n";

		bindings.location_tex_metalness = bindings.location_tex_color + 1;
		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_METALNESS %d\n", bindings.location_tex_metalness);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_METALNESS_MAP 0\n";
		bindings.location_tex_metalness = bindings.location_tex_color;
	}

	if (options.has_roughness_map)
	{
		defines += "#define HAS_ROUGHNESS_MAP 1\n";
		bindings.location_tex_roughness = bindings.location_tex_metalness + 1;
		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_ROUGHNESS %d\n", bindings.location_tex_roughness);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_ROUGHNESS_MAP 0\n";
		bindings.location_tex_roughness = bindings.location_tex_metalness;
	}

	if (options.has_normal_map)
	{
		defines += "#define HAS_NORMAL_MAP 1\n";
		bindings.location_tex_normal = bindings.location_tex_roughness + 1;
		bindings.location_attrib_tangent = bindings.location_attrib_atlas_uv + 1;
		bindings.location_varying_tangent = bindings.location_varying_atlas_uv + 1;
		bindings.location_attrib_bitangent = bindings.location_attrib_tangent + 1;
		bindings.location_varying_bitangent = bindings.location_varying_tangent + 1;

		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_NORMAL %d\n", bindings.location_tex_normal);
			defines += line;
		}

		{
			char line[64];
			sprintf(line, "#define LOCATION_ATTRIB_TANGENT %d\n", bindings.location_attrib_tangent);
			defines += line;
		}
		{
			char line[64];
			sprintf(line, "#define LOCATION_VARYING_TANGENT %d\n", bindings.location_varying_tangent);
			defines += line;
		}
		{
			char line[64];
			sprintf(line, "#define LOCATION_ATTRIB_BITANGENT %d\n", bindings.location_attrib_bitangent);
			defines += line;
		}
		{
			char line[64];
			sprintf(line, "#define LOCATION_VARYING_BITANGENT %d\n", bindings.location_varying_bitangent);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_NORMAL_MAP 0\n";
		bindings.location_tex_normal = bindings.location_tex_roughness;
		bindings.location_attrib_tangent = bindings.location_attrib_atlas_uv;
		bindings.location_varying_tangent = bindings.location_varying_atlas_uv;
		bindings.location_attrib_bitangent = bindings.location_attrib_tangent;
		bindings.location_varying_bitangent = bindings.location_varying_tangent;
	}

	{
		bindings.location_varying_world_pos = bindings.location_varying_bitangent + 1;
		{
			char line[64];
			sprintf(line, "#define LOCATION_VARYING_WORLD_POS %d\n", bindings.location_varying_world_pos);
			defines += line;
		}
	}

	if (options.has_emissive_map)
	{
		defines += "#define HAS_EMISSIVE_MAP 1\n";
		bindings.location_tex_emissive = bindings.location_tex_normal + 1;
		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_EMISSIVE %d\n", bindings.location_tex_emissive);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_EMISSIVE_MAP 0\n";
		bindings.location_tex_emissive = bindings.location_tex_normal;
	}

	if (options.has_specular_map)
	{
		defines += "#define HAS_SPECULAR_MAP 1\n";
		bindings.location_tex_specular = bindings.location_tex_emissive + 1;
		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_SPECULAR %d\n", bindings.location_tex_specular);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_SPECULAR_MAP 0\n";
		bindings.location_tex_specular = bindings.location_tex_emissive;
	}

	if (options.has_glossiness_map)
	{
		defines += "#define HAS_GLOSSINESS_MAP 1\n";
		bindings.location_tex_glossiness = bindings.location_tex_specular + 1;
		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_GLOSSINESS %d\n", bindings.location_tex_glossiness);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_GLOSSINESS_MAP 0\n";
		bindings.location_tex_glossiness = bindings.location_tex_specular;
	}

	{
		char line[64];
		sprintf(line, "#define NUM_DIRECTIONAL_LIGHTS %d\n", options.num_directional_lights);
		defines += line;
	}

	if (options.num_directional_lights > 0)
	{
		bindings.binding_directional_lights = bindings.binding_material + 1;
		{
			char line[64];
			sprintf(line, "#define BINDING_DIRECTIONAL_LIGHTS %d\n", bindings.binding_directional_lights);
			defines += line;
		}
	}
	else
	{
		bindings.binding_directional_lights = bindings.binding_material;
	}

	{
		char line[64];
		sprintf(line, "#define NUM_DIRECTIONAL_SHADOWS %d\n", options.num_directional_shadows);
		defines += line;
	}

	bindings.location_tex_directional_shadow = bindings.location_tex_glossiness + options.num_directional_shadows;
	bindings.location_tex_directional_shadow_depth = bindings.location_tex_directional_shadow + options.num_directional_shadows;

	if (options.num_directional_shadows > 0)
	{	
		bindings.binding_directional_shadows = bindings.binding_directional_lights + 1;
		{
			char line[64];
			sprintf(line, "#define BINDING_DIRECTIONAL_SHADOWS %d\n", bindings.binding_directional_shadows);
			defines += line;
		}
		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_DIRECTIONAL_SHADOW %d\n", bindings.location_tex_directional_shadow - options.num_directional_shadows + 1);
			defines += line;
		}
		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_DIRECTIONAL_SHADOW_DEPTH %d\n", bindings.location_tex_directional_shadow_depth - options.num_directional_shadows + 1);
			defines += line;
		}
	}
	else
	{
		bindings.binding_directional_shadows = bindings.binding_directional_lights;
	}

	bool has_indirect_light = options.has_environment_map || options.has_probe_grid || options.has_lod_probe_grid ||  options.has_ambient_light || options.has_hemisphere_light;
	if (has_indirect_light)
	{
		defines += "#define HAS_INDIRECT_LIGHT 1\n";
	}
	else
	{
		defines += "#define HAS_INDIRECT_LIGHT 0\n";
	}

	if (options.has_lightmap)
	{
		bindings.location_tex_lightmap = bindings.location_tex_directional_shadow_depth + 1;
		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_LIGHTMAP %d\n", bindings.location_tex_lightmap);
			defines += line;
		}
	}
	else
	{
		bindings.location_tex_lightmap = bindings.location_tex_directional_shadow_depth;
	}

	if (options.has_reflection_map)
	{
		defines += "#define HAS_REFLECTION_MAP 1\n";
		bindings.location_tex_reflection_map = bindings.location_tex_lightmap + 1;
		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_REFLECTION_MAP %d\n", bindings.location_tex_reflection_map);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_REFLECTION_MAP 0\n";
		bindings.location_tex_reflection_map = bindings.location_tex_lightmap;
	}

	if (options.has_reflection_distance)
	{
		defines += "#define HAS_REFLECTION_DISTANCE 1\n";
		bindings.location_tex_reflection_distance = bindings.location_tex_reflection_map + 1;
		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_REFLECTION_DISTANCE %d\n", bindings.location_tex_reflection_distance);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_REFLECTION_DISTANCE 0\n";
		bindings.location_tex_reflection_distance = bindings.location_tex_reflection_map;
	}

	if (options.has_reflector)
	{
		defines += "#define HAS_REFLECTOR 1\n";
		bindings.binding_reflector_camera = bindings.binding_directional_shadows + 1;
		bindings.location_tex_reflector = bindings.location_tex_reflection_distance + 1;
		bindings.location_tex_reflector_depth = bindings.location_tex_reflection_distance + 2;

		{
			char line[64];
			sprintf(line, "#define BINDING_REFLECTOR_CAMERA %d\n", bindings.binding_reflector_camera);
			defines += line;
		}
		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_REFLECTOR %d\n", bindings.location_tex_reflector);
			defines += line;
		}
		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_REFLECTOR_DEPTH %d\n", bindings.location_tex_reflector_depth);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_REFLECTOR 0\n";
		bindings.binding_reflector_camera = bindings.binding_directional_shadows;
		bindings.location_tex_reflector = bindings.location_tex_reflection_distance;
		bindings.location_tex_reflector_depth = bindings.location_tex_reflection_distance;
	}

	if (options.has_environment_map)
	{
		defines += "#define HAS_ENVIRONMENT_MAP 1\n";
		bindings.binding_environment_map = bindings.binding_reflector_camera + 1;
		
		{
			char line[64];
			sprintf(line, "#define BINDING_ENVIRONMEN_MAP %d\n", bindings.binding_environment_map);
			defines += line;
		}
		
	}
	else
	{
		defines += "#define HAS_ENVIRONMENT_MAP 0\n";
		bindings.binding_environment_map = bindings.binding_reflector_camera;
	}

	if (options.has_probe_grid)
	{
		defines += "#define HAS_PROBE_GRID 1\n";
		bindings.binding_probe_grid = bindings.binding_environment_map + 1;		

		{
			char line[64];
			sprintf(line, "#define BINDING_PROBE_GRID %d\n", bindings.binding_probe_grid);
			defines += line;
		}
		
	}
	else
	{
		defines += "#define HAS_PROBE_GRID 0\n";
		bindings.binding_probe_grid = bindings.binding_environment_map;
	}

	if (options.probe_reference_recorded)
	{
		defines += "#define PROBE_REFERENCE_RECORDED 1\n";
		bindings.binding_probe_references = bindings.binding_probe_grid + 1;

		{
			char line[64];
			sprintf(line, "#define BINDING_PROBE_REFERENCES %d\n", bindings.binding_probe_references);
			defines += line;
		}
	}
	else
	{
		defines += "#define PROBE_REFERENCE_RECORDED 0\n";
		bindings.binding_probe_references = bindings.binding_probe_grid;
	}
	
	if (options.has_lod_probe_grid)
	{
		defines += "#define HAS_LOD_PROBE_GRID 1\n";
		bindings.binding_lod_probe_grid = bindings.binding_probe_references + 1;
		bindings.binding_lod_probes = bindings.binding_lod_probe_grid + 1;
		bindings.binding_lod_probe_indices = bindings.binding_lod_probes + 1;

		{
			char line[64];
			sprintf(line, "#define BINDING_LOD_PROBE_GRID %d\n", bindings.binding_lod_probe_grid);
			defines += line;
		}

		{
			char line[64];
			sprintf(line, "#define BINDING_LOD_PROBES %d\n", bindings.binding_lod_probes);
			defines += line;
		}

		{
			char line[64];
			sprintf(line, "#define BINDING_LOD_PROBE_INDICES %d\n", bindings.binding_lod_probe_indices);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_LOD_PROBE_GRID 0\n";
		bindings.binding_lod_probe_grid = bindings.binding_probe_references;
		bindings.binding_lod_probes = bindings.binding_probe_references;
		bindings.binding_lod_probe_indices = bindings.binding_probe_references;
	}

	if (options.has_probe_grid || options.has_lod_probe_grid)
	{
		bindings.location_tex_irradiance = bindings.location_tex_reflector_depth + 1;
		bindings.location_tex_visibility = bindings.location_tex_irradiance + 1;
		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_IRRADIANCE %d\n", bindings.location_tex_irradiance);
			defines += line;
		}
		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_VISIBILITY %d\n", bindings.location_tex_visibility);
			defines += line;
		}
	}
	else
	{
		bindings.location_tex_irradiance = bindings.location_tex_reflector_depth;
		bindings.location_tex_visibility = bindings.location_tex_reflector_depth;
	}

	if (options.has_ambient_light)
	{
		defines += "#define HAS_AMBIENT_LIGHT 1\n";
		bindings.binding_ambient_light = bindings.binding_lod_probe_indices + 1;
		{
			char line[64];
			sprintf(line, "#define BINDING_AMBIENT_LIGHT %d\n", bindings.binding_ambient_light);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_AMBIENT_LIGHT 0\n";
		bindings.binding_ambient_light = bindings.binding_lod_probe_indices;
	}

	if (options.has_hemisphere_light)
	{
		defines += "#define HAS_HEMISPHERE_LIGHT 1\n";
		bindings.binding_hemisphere_light = bindings.binding_ambient_light + 1;
		{
			char line[64];
			sprintf(line, "#define BINDING_HEMISPHERE_LIGHT %d\n", bindings.binding_hemisphere_light);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_HEMISPHERE_LIGHT 0\n";
		bindings.binding_hemisphere_light = bindings.binding_ambient_light;
	}

	{
		char line[64];
		sprintf(line, "#define TONE_SHADING %d\n", options.tone_shading);
		defines += line;
	}

	if (options.has_fog)
	{
		defines += "#define HAS_FOG 1\n";
		bindings.binding_fog = bindings.binding_hemisphere_light + 1;
		{
			char line[64];
			sprintf(line, "#define BINDING_FOG %d\n", bindings.binding_fog);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_FOG 0\n";
		bindings.binding_fog = bindings.binding_hemisphere_light;
	}	

	if (options.use_ssao && options.alpha_mode == AlphaMode::Opaque)
	{
		defines += "#define USE_SSAO 1\n";
		bindings.location_tex_ssao = bindings.location_tex_visibility + 1;
		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_SSAO %d\n", bindings.location_tex_ssao);
			defines += line;
		}
	}
	else
	{
		defines += "#define USE_SSAO 0\n";
		bindings.location_tex_ssao = bindings.location_tex_visibility;
	}

	replace(s_vertex, "#DEFINES#", defines.c_str());
	replace(s_frag, "#DEFINES#", defines.c_str());
}

StandardRoutine::StandardRoutine(const Options& options) : m_options(options)
{
	std::string s_vertex, s_frag;
	s_generate_shaders(options, m_bindings, s_vertex, s_frag);
	
	GLShader vert_shader(GL_VERTEX_SHADER, s_vertex.c_str());
	GLShader frag_shader(GL_FRAGMENT_SHADER, s_frag.c_str());
	m_prog = (std::unique_ptr<GLProgram>)(new GLProgram(vert_shader, frag_shader));
}

void StandardRoutine::_render_common(const RenderParams& params)
{
	const MeshStandardMaterial& material = *(MeshStandardMaterial*)params.material_list[params.primitive->material_idx];
	const GeometrySet& geo = params.primitive->geometry[params.primitive->geometry.size() - 1];

	glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_camera, params.camera->m_constant.m_id);
	if (m_options.is_reflect)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_matrix_reflector, params.camera->reflector->m_constant.m_id);
	}
	glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_model, params.constant_model->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_material, material.constant_material.m_id);

	if (m_options.num_directional_lights > 0)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_directional_lights, params.lights->constant_directional_lights->m_id);
	}

	if (m_options.num_directional_shadows > 0)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_directional_shadows, params.lights->constant_directional_shadows->m_id);
	}

	if (m_options.has_reflector)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_reflector_camera, params.reflector->m_camera.m_constant.m_id);
	}

	if (m_options.has_environment_map)
	{
		if (params.lights->environment_map != nullptr)
		{
			glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_environment_map, params.lights->environment_map->m_constant.m_id);
		}
		else if (params.primitive->envMap != nullptr)
		{
			glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_environment_map, params.primitive->envMap->m_constant.m_id);
		}
	}

	if (m_options.has_probe_grid)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_probe_grid, params.lights->probe_grid->m_constant.m_id);
		if (m_options.probe_reference_recorded)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindings.binding_probe_references, params.lights->probe_grid->m_ref_buf->m_id);
		}
	}

	if (m_options.has_lod_probe_grid)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_lod_probe_grid, params.lights->lod_probe_grid->m_constant.m_id);
		if (params.lights->lod_probe_grid->m_probe_bufs[0] != nullptr)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindings.binding_lod_probes, params.lights->lod_probe_grid->m_probe_bufs[0]->m_id);
		}
		if (params.lights->lod_probe_grid->m_sub_index_buf != nullptr)
		{
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindings.binding_lod_probe_indices, params.lights->lod_probe_grid->m_sub_index_buf->m_id);
		}
	}

	if (m_options.has_ambient_light)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_ambient_light, params.lights->ambient_light->m_constant.m_id);
	}

	if (m_options.has_hemisphere_light)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_hemisphere_light, params.lights->hemisphere_light->m_constant.m_id);
	}

	if (m_options.has_fog)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_fog, params.constant_fog->m_id);
	}

	glBindBuffer(GL_ARRAY_BUFFER, geo.pos_buf->m_id);
	glVertexAttribPointer(m_bindings.location_attrib_pos, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(m_bindings.location_attrib_pos);

	glBindBuffer(GL_ARRAY_BUFFER, geo.normal_buf->m_id);
	glVertexAttribPointer(m_bindings.location_attrib_norm, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(m_bindings.location_attrib_norm);

	if (m_options.has_color)
	{
		glBindBuffer(GL_ARRAY_BUFFER, params.primitive->color_buf->m_id);
		glVertexAttribPointer(m_bindings.location_attrib_color, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(m_bindings.location_attrib_color);
	}

	bool has_uv = m_options.has_color_texture || m_options.has_metalness_map || m_options.has_roughness_map
		|| m_options.has_normal_map || m_options.has_emissive_map || m_options.has_specular_map || m_options.has_glossiness_map;
	if (has_uv)
	{
		glBindBuffer(GL_ARRAY_BUFFER, params.primitive->uv_buf->m_id);
		glVertexAttribPointer(m_bindings.location_attrib_uv, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(m_bindings.location_attrib_uv);
	}

	if (m_options.has_lightmap)
	{
		glBindBuffer(GL_ARRAY_BUFFER, params.primitive->lightmap_uv_buf->m_id);
		glVertexAttribPointer(m_bindings.location_attrib_atlas_uv, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(m_bindings.location_attrib_atlas_uv);
	}

	if (m_options.has_normal_map)
	{
		glBindBuffer(GL_ARRAY_BUFFER, geo.tangent_buf->m_id);
		glVertexAttribPointer(m_bindings.location_attrib_tangent, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(m_bindings.location_attrib_tangent);

		glBindBuffer(GL_ARRAY_BUFFER, geo.bitangent_buf->m_id);
		glVertexAttribPointer(m_bindings.location_attrib_bitangent, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(m_bindings.location_attrib_bitangent);
	}

	int texture_idx = 0;
	if (m_options.has_color_texture)
	{
		const GLTexture2D& tex = *params.tex_list[material.tex_idx_map];
		glActiveTexture(GL_TEXTURE0 + texture_idx);
		glBindTexture(GL_TEXTURE_2D, tex.tex_id);
		glUniform1i(m_bindings.location_tex_color, texture_idx);
		texture_idx++;
	}

	if (m_options.has_metalness_map)
	{
		const GLTexture2D& tex = *params.tex_list[material.tex_idx_metalnessMap];
		glActiveTexture(GL_TEXTURE0 + texture_idx);
		glBindTexture(GL_TEXTURE_2D, tex.tex_id);
		glUniform1i(m_bindings.location_tex_metalness, texture_idx);
		texture_idx++;
	}

	if (m_options.has_roughness_map)
	{
		const GLTexture2D& tex = *params.tex_list[material.tex_idx_roughnessMap];
		glActiveTexture(GL_TEXTURE0 + texture_idx);
		glBindTexture(GL_TEXTURE_2D, tex.tex_id);
		glUniform1i(m_bindings.location_tex_roughness, texture_idx);
		texture_idx++;
	}

	if (m_options.has_normal_map)
	{
		const GLTexture2D& tex = *params.tex_list[material.tex_idx_normalMap];
		glActiveTexture(GL_TEXTURE0 + texture_idx);
		glBindTexture(GL_TEXTURE_2D, tex.tex_id);
		glUniform1i(m_bindings.location_tex_normal, texture_idx);
		texture_idx++;
	}

	if (m_options.has_emissive_map)
	{
		const GLTexture2D& tex = *params.tex_list[material.tex_idx_emissiveMap];
		glActiveTexture(GL_TEXTURE0 + texture_idx);
		glBindTexture(GL_TEXTURE_2D, tex.tex_id);
		glUniform1i(m_bindings.location_tex_emissive, texture_idx);
		texture_idx++;
	}

	if (m_options.has_specular_map)
	{
		const GLTexture2D& tex = *params.tex_list[material.tex_idx_specularMap];
		glActiveTexture(GL_TEXTURE0 + texture_idx);
		glBindTexture(GL_TEXTURE_2D, tex.tex_id);
		glUniform1i(m_bindings.location_tex_specular, texture_idx);
		texture_idx++;
	}

	if (m_options.has_glossiness_map)
	{
		const GLTexture2D& tex = *params.tex_list[material.tex_idx_glossinessMap];
		glActiveTexture(GL_TEXTURE0 + texture_idx);
		glBindTexture(GL_TEXTURE_2D, tex.tex_id);
		glUniform1i(m_bindings.location_tex_glossiness, texture_idx);
		texture_idx++;
	}

	if (m_options.num_directional_shadows > 0)
	{
		std::vector<int> values(m_options.num_directional_shadows);
		for (int i = 0; i < m_options.num_directional_shadows; i++)
		{
			glActiveTexture(GL_TEXTURE0 + texture_idx);
			glBindTexture(GL_TEXTURE_2D, params.lights->directional_shadow_texs[i]);
			values[i] = texture_idx;
			texture_idx++;
		}
		int start_idx = m_bindings.location_tex_directional_shadow - m_options.num_directional_shadows + 1;
		int start_idx_depth = m_bindings.location_tex_directional_shadow_depth - m_options.num_directional_shadows + 1;
		glUniform1iv(start_idx, m_options.num_directional_shadows, values.data());
		glUniform1iv(start_idx_depth, m_options.num_directional_shadows, values.data());
	}

	if (m_options.has_lightmap)
	{
		glActiveTexture(GL_TEXTURE0 + texture_idx);
		glBindTexture(GL_TEXTURE_2D, params.tex_lightmap->tex_id);
		glUniform1i(m_bindings.location_tex_lightmap, texture_idx);
		texture_idx++;		
	}

	if (m_options.has_reflection_map)
	{
		glActiveTexture(GL_TEXTURE0 + texture_idx);
		glBindTexture(GL_TEXTURE_CUBE_MAP, params.lights->reflection_map->tex_id);
		glUniform1i(m_bindings.location_tex_reflection_map, texture_idx);
		texture_idx++;
	}

	if (m_options.has_reflection_distance)
	{
		glActiveTexture(GL_TEXTURE0 + texture_idx);
		glBindTexture(GL_TEXTURE_CUBE_MAP, params.lights->reflection_map->tex_id_dis);
		glUniform1i(m_bindings.location_tex_reflection_distance, texture_idx);
		texture_idx++;
	}

	if (m_options.has_reflector)
	{
		glActiveTexture(GL_TEXTURE0 + texture_idx);
		glBindTexture(GL_TEXTURE_2D, params.reflector->m_target.m_tex_video->tex_id);
		glUniform1i(m_bindings.location_tex_reflector, texture_idx);
		texture_idx++;

		glActiveTexture(GL_TEXTURE0 + texture_idx);
		glBindTexture(GL_TEXTURE_2D, params.reflector->m_target.m_tex_depth->tex_id);
		glUniform1i(m_bindings.location_tex_reflector_depth, texture_idx);
		texture_idx++;
	}

	if (m_options.has_probe_grid)
	{
		if (params.lights->probe_grid->m_tex_visibility != nullptr)
		{
			glActiveTexture(GL_TEXTURE0 + texture_idx);
			glBindTexture(GL_TEXTURE_2D, params.lights->probe_grid->m_tex_visibility->tex_id);
			glUniform1i(m_bindings.location_tex_visibility, texture_idx);
			texture_idx++;
		}

		if (params.lights->probe_grid->m_tex_irradiance != nullptr)
		{
			glActiveTexture(GL_TEXTURE0 + texture_idx);
			glBindTexture(GL_TEXTURE_2D, params.lights->probe_grid->m_tex_irradiance->tex_id);
			glUniform1i(m_bindings.location_tex_irradiance, texture_idx);
			texture_idx++;
		}
	}

	if (m_options.has_lod_probe_grid)
	{
		if (params.lights->lod_probe_grid->m_tex_visibility != nullptr)
		{
			glActiveTexture(GL_TEXTURE0 + texture_idx);
			glBindTexture(GL_TEXTURE_2D, params.lights->lod_probe_grid->m_tex_visibility->tex_id);
			glUniform1i(m_bindings.location_tex_visibility, texture_idx);
			texture_idx++;
		}

		if (params.lights->lod_probe_grid->m_tex_irradiance != nullptr)
		{
			glActiveTexture(GL_TEXTURE0 + texture_idx);
			glBindTexture(GL_TEXTURE_2D, params.lights->lod_probe_grid->m_tex_irradiance->tex_id);
			glUniform1i(m_bindings.location_tex_irradiance, texture_idx);
			texture_idx++;
		}
	}

	if (m_options.use_ssao && m_options.alpha_mode == AlphaMode::Opaque)
	{
		glActiveTexture(GL_TEXTURE0 + texture_idx);
		glBindTexture(GL_TEXTURE_2D, params.tex_ao->tex_id);
		glUniform1i(m_bindings.location_tex_ssao, texture_idx);
		texture_idx++;
	}
}

void StandardRoutine::render(const RenderParams& params)
{
	const MeshStandardMaterial& material = *(MeshStandardMaterial*)params.material_list[params.primitive->material_idx];	

	glEnable(GL_DEPTH_TEST);	
	glDepthFunc(GL_LEQUAL);

	if (m_options.alpha_mode == AlphaMode::Mask || (m_options.alpha_mode == AlphaMode::Opaque && params.camera->reflector != nullptr))
	{
		glDepthMask(GL_TRUE);
	}
	else
	{
		glDepthMask(GL_FALSE);
	}

	glFrontFace(params.camera->reflector != nullptr ? GL_CW : GL_CCW);
	if (material.doubleSided)
	{
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

	glUseProgram(m_prog->m_id);
	
	_render_common(params);

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


void StandardRoutine::render_batched(const RenderParams& params, const std::vector<void*>& offset_lst, const std::vector<int>& count_lst)
{
	const MeshStandardMaterial& material = *(MeshStandardMaterial*)params.material_list[params.primitive->material_idx];

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	if (m_options.alpha_mode == AlphaMode::Mask || (m_options.alpha_mode == AlphaMode::Opaque && params.camera->reflector!=nullptr))
	{
		glDepthMask(GL_TRUE);
	}
	else
	{
		glDepthMask(GL_FALSE);
	}

	if (material.doubleSided)
	{
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

	glUseProgram(m_prog->m_id);

	_render_common(params);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, params.primitive->index_buf->m_id);
	glMultiDrawElements(GL_TRIANGLES, count_lst.data(), GL_UNSIGNED_INT, offset_lst.data(), offset_lst.size());	

	glUseProgram(0);

}