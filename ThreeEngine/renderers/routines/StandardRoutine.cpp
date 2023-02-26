#include <GL/glew.h>
#include "models/ModelComponents.h"
#include "lights/DirectionalLight.h"
#include "StandardRoutine.h"

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

const vec2 Poisson32[32] = vec2[](
    vec2(-0.975402, -0.0711386),
    vec2(-0.920347, -0.41142),
    vec2(-0.883908, 0.217872),
    vec2(-0.884518, 0.568041),
    vec2(-0.811945, 0.90521),
    vec2(-0.792474, -0.779962),
    vec2(-0.614856, 0.386578),
    vec2(-0.580859, -0.208777),
    vec2(-0.53795, 0.716666),
    vec2(-0.515427, 0.0899991),
    vec2(-0.454634, -0.707938),
    vec2(-0.420942, 0.991272),
    vec2(-0.261147, 0.588488),
    vec2(-0.211219, 0.114841),
    vec2(-0.146336, -0.259194),
    vec2(-0.139439, -0.888668),
    vec2(0.0116886, 0.326395),
    vec2(0.0380566, 0.625477),
    vec2(0.0625935, -0.50853),
    vec2(0.125584, 0.0469069),
    vec2(0.169469, -0.997253),
    vec2(0.320597, 0.291055),
    vec2(0.359172, -0.633717),
    vec2(0.435713, -0.250832),
    vec2(0.507797, -0.916562),
    vec2(0.545763, 0.730216),
    vec2(0.56859, 0.11655),
    vec2(0.743156, -0.505173),
    vec2(0.736442, -0.189734),
    vec2(0.843562, 0.357036),
    vec2(0.865413, 0.763726),
    vec2(0.872005, -0.927)
);

const vec2 Poisson64[64] = vec2[](
    vec2(-0.934812, 0.366741),
    vec2(-0.918943, -0.0941496),
    vec2(-0.873226, 0.62389),
    vec2(-0.8352, 0.937803),
    vec2(-0.822138, -0.281655),
    vec2(-0.812983, 0.10416),
    vec2(-0.786126, -0.767632),
    vec2(-0.739494, -0.535813),
    vec2(-0.681692, 0.284707),
    vec2(-0.61742, -0.234535),
    vec2(-0.601184, 0.562426),
    vec2(-0.607105, 0.847591),
    vec2(-0.581835, -0.00485244),
    vec2(-0.554247, -0.771111),
    vec2(-0.483383, -0.976928),
    vec2(-0.476669, -0.395672),
    vec2(-0.439802, 0.362407),
    vec2(-0.409772, -0.175695),
    vec2(-0.367534, 0.102451),
    vec2(-0.35313, 0.58153),
    vec2(-0.341594, -0.737541),
    vec2(-0.275979, 0.981567),
    vec2(-0.230811, 0.305094),
    vec2(-0.221656, 0.751152),
    vec2(-0.214393, -0.0592364),
    vec2(-0.204932, -0.483566),
    vec2(-0.183569, -0.266274),
    vec2(-0.123936, -0.754448),
    vec2(-0.0859096, 0.118625),
    vec2(-0.0610675, 0.460555),
    vec2(-0.0234687, -0.962523),
    vec2(-0.00485244, -0.373394),
    vec2(0.0213324, 0.760247),
    vec2(0.0359813, -0.0834071),
    vec2(0.0877407, -0.730766),
    vec2(0.14597, 0.281045),
    vec2(0.18186, -0.529649),
    vec2(0.188208, -0.289529),
    vec2(0.212928, 0.063509),
    vec2(0.23661, 0.566027),
    vec2(0.266579, 0.867061),
    vec2(0.320597, -0.883358),
    vec2(0.353557, 0.322733),
    vec2(0.404157, -0.651479),
    vec2(0.410443, -0.413068),
    vec2(0.413556, 0.123325),
    vec2(0.46556, -0.176183),
    vec2(0.49266, 0.55388),
    vec2(0.506333, 0.876888),
    vec2(0.535875, -0.885556),
    vec2(0.615894, 0.0703452),
    vec2(0.637135, -0.637623),
    vec2(0.677236, -0.174291),
    vec2(0.67626, 0.7116),
    vec2(0.686331, -0.389935),
    vec2(0.691031, 0.330729),
    vec2(0.715629, 0.999939),
    vec2(0.8493, -0.0485549),
    vec2(0.863582, -0.85229),
    vec2(0.890622, 0.850581),
    vec2(0.898068, 0.633778),
    vec2(0.92053, -0.355693),
    vec2(0.933348, -0.62981),
    vec2(0.95294, 0.156896)
);

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

	// case POISSON_32_64:
    {
        maxBlockers = 32.0;
        for (int i = 0; i < 32; ++i)
        {
            vec2 offset = Poisson32[i] * searchRegionRadiusUV;
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

	//case POISSON_32_64:
    {
        for (int i = 0; i < 64; ++i)
        {
            vec2 offset = Poisson64[i] * filterRadiusUV;        
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

vec3 getRadiance(in vec3 reflectVec, float roughness)
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

vec3 getIrradiance(in vec3 normal)
{
	return shGetIrradianceAt(normal, uSHCoefficients);
}

#if !HAS_REFLECTION_MAP
vec3 getRadiance(in vec3 reflectVec, float roughness)
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

float get_visibility(in ivec3 vert, in vec3 vert_world)
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
	vec3 dir = vWorldPos - vert_world;	
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

vec3 getIrradiance(in vec3 normal)
{
	vec3 size_grid = uCoverageMax.xyz - uCoverageMin.xyz;
	vec3 pos_normalized = (vWorldPos - uCoverageMin.xyz)/size_grid;
	pos_normalized.y = pow(pos_normalized.y, 1.0/uYpower);	
	vec3 pos_voxel = pos_normalized * vec3(uDivisions) - vec3(0.5);
	pos_voxel = clamp(pos_voxel, vec3(0.0), vec3(uDivisions) - vec3(1.0));
	
	ivec3 i_voxel = clamp(ivec3(pos_voxel), ivec3(0), ivec3(uDivisions) - ivec3(2));
	vec3 frac_voxel = pos_voxel - vec3(i_voxel);

	float sum_weight = 0.0;
	vec4 coeffs[9];
	for (int i=0; i<9; i++) coeffs[i] = vec4(0.0);

	vec3 dx = dFdx(vWorldPos);
	vec3 dy = dFdy(vWorldPos);
	vec3 N = normalize(cross(dx, dy));

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

				if (dot(dir, N)>=0.0)
				{					
					vec3 w = vec3(1.0) - abs(vec3(x,y,z) - frac_voxel);
					float weight = w.x * w.y * w.z;
					if (weight>0.0)
					{
						weight*= get_visibility(vert,vert_world);					
						sum_weight += weight;
						acc_coeffs(coeffs, vert, weight);
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
vec3 getRadiance(in vec3 reflectVec, float roughness)
{
	return getIrradiance(reflectVec) * RECIPROCAL_PI;
}
#endif

#endif

)";

static std::string g_frag_part3 =
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

vec3 getIrradiance(in vec3 normal)
{
	float lod = get_probe_lod_f(vWorldPos);
	int i_lod = int(lod);
	float frac_lod = lod - float(i_lod);

	float sum_weight = 0.0;
	vec4 coeffs[9];
	for (int i=0; i<9; i++) coeffs[i] = vec4(0.0);

	vec3 dx = dFdx(vWorldPos);
	vec3 dy = dFdy(vWorldPos);
	vec3 N = normalize(cross(dx, dy));

	vec3 size_grid = uCoverageMax.xyz - uCoverageMin.xyz;
	vec3 pos_normalized = (vWorldPos - uCoverageMin.xyz)/size_grid;

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
					vec3 dir = normalize(vert_world - vWorldPos);
					
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
					vec3 dir = normalize(vert_world - vWorldPos);
					
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
vec3 getRadiance(in vec3 reflectVec, float roughness)
{
	return getIrradiance(reflectVec) * RECIPROCAL_PI;
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

vec3 getIrradiance(in vec3 normal)
{
	return uAmbientColor.xyz * PI;
}

#if !HAS_REFLECTION_MAP
vec3 getRadiance(in vec3 reflectVec, float roughness)
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

vec3 getIrradiance(in vec3 normal)
{
	return HemisphereColor(normal) * PI;
}

#if !HAS_REFLECTION_MAP
vec3 getRadiance(in vec3 reflectVec, float roughness)
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

#if USE_SSAO
layout (location = LOCATION_TEX_SSAO) uniform sampler2D uTexSSAO;
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

	float tex_alpha = 1.0;

#if HAS_COLOR_TEX
	vec4 tex_color = texture(uTexColor, vUV);
	tex_alpha = tex_color.w;
	base_color *= tex_color;
#endif

#if ALPHA_MASK
	base_color.w = base_color.w > uAlphaCutoff ? 1.0 : 0.0;
#endif

#if ALPHA_MASK || ALPHA_BLEND
	if (base_color.w == 0.0) discard;
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

	if (uDoubleSided!=0)
	{		
		if (dot(viewDir, norm)<0.0) norm = -norm;
	}

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

	vec3 dxy = max(abs(dFdx(norm)), abs(dFdy(norm)));
	float geometryRoughness = max(max(dxy.x, dxy.y), dxy.z);	
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
#endif


#if HAS_INDIRECT_LIGHT
	{
		vec3 reflectVec = reflect(-viewDir, norm);
		reflectVec = normalize( mix( reflectVec, norm, material.roughness * material.roughness) );
		vec3 irradiance = getIrradiance(norm);
		vec3 radiance = getRadiance(reflectVec, material.roughness);

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
#endif

	vec3 col = emissive + specular;
#if !IS_HIGHTLIGHT
	col += diffuse;
#endif
	// col = clamp(col, 0.0, 1.0);	

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

	{
		bindings.binding_model = bindings.binding_camera + 1;
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
		bindings.location_attrib_tangent = bindings.location_attrib_uv + 1;
		bindings.location_varying_tangent = bindings.location_varying_uv + 1;
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
		bindings.location_attrib_tangent = bindings.location_attrib_uv;
		bindings.location_varying_tangent = bindings.location_varying_uv;
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

	if (options.has_reflection_map)
	{
		defines += "#define HAS_REFLECTION_MAP 1\n";
		bindings.location_tex_reflection_map = bindings.location_tex_directional_shadow_depth + 1;
		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_REFLECTION_MAP %d\n", bindings.location_tex_reflection_map);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_REFLECTION_MAP 0\n";
		bindings.location_tex_reflection_map = bindings.location_tex_directional_shadow_depth;
	}

	if (options.has_environment_map)
	{
		defines += "#define HAS_ENVIRONMENT_MAP 1\n";
		bindings.binding_environment_map = bindings.binding_directional_shadows + 1;
		
		{
			char line[64];
			sprintf(line, "#define BINDING_ENVIRONMEN_MAP %d\n", bindings.binding_environment_map);
			defines += line;
		}
		
	}
	else
	{
		defines += "#define HAS_ENVIRONMENT_MAP 0\n";
		bindings.binding_environment_map = bindings.binding_directional_shadows;		
	}

	if (options.has_probe_grid)
	{
		defines += "#define HAS_PROBE_GRID 1\n";
		bindings.binding_probe_grid = bindings.binding_environment_map + 1;
		bindings.binding_probes = bindings.binding_probe_grid + 1;
		bindings.binding_probe_visibility= bindings.binding_probes + 1;

		{
			char line[64];
			sprintf(line, "#define BINDING_PROBE_GRID %d\n", bindings.binding_probe_grid);
			defines += line;
		}

		{
			char line[64];
			sprintf(line, "#define BINDING_PROBES %d\n", bindings.binding_probes);
			defines += line;
		}

		{
			char line[64];
			sprintf(line, "#define BINDING_PROBE_VISIBILITY %d\n", bindings.binding_probe_visibility);
			defines += line;
		}
	}
	else
	{
		defines += "#define HAS_PROBE_GRID 0\n";
		bindings.binding_probe_grid = bindings.binding_environment_map;
		bindings.binding_probes = bindings.binding_environment_map;
		bindings.binding_probe_visibility = bindings.binding_environment_map;
	}

	if (options.probe_reference_recorded)
	{
		defines += "#define PROBE_REFERENCE_RECORDED 1\n";
		bindings.binding_probe_references = bindings.binding_probe_visibility + 1;

		{
			char line[64];
			sprintf(line, "#define BINDING_PROBE_REFERENCES %d\n", bindings.binding_probe_references);
			defines += line;
		}
	}
	else
	{
		defines += "#define PROBE_REFERENCE_RECORDED 0\n";
		bindings.binding_probe_references = bindings.binding_probe_visibility;
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

	if (options.use_ssao)
	{
		defines += "#define USE_SSAO 1\n";
		bindings.location_tex_ssao = bindings.location_tex_reflection_map + 1;
		{
			char line[64];
			sprintf(line, "#define LOCATION_TEX_SSAO %d\n", bindings.location_tex_ssao);
			defines += line;
		}
	}
	else
	{
		defines += "#define USE_SSAO 0\n";
		bindings.location_tex_ssao = bindings.location_tex_reflection_map;
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
		glCullFace(GL_BACK);
	}

	glUseProgram(m_prog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_camera, params.constant_camera->m_id);
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

	if (m_options.has_environment_map)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, m_bindings.binding_environment_map, params.lights->environment_map->m_constant.m_id);
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

	if (m_options.has_reflection_map)
	{
		glActiveTexture(GL_TEXTURE0 + texture_idx);
		glBindTexture(GL_TEXTURE_CUBE_MAP, params.lights->reflection_map->tex_id);
		glUniform1i(m_bindings.location_tex_reflection_map, texture_idx);
		texture_idx++;
	}

	if (m_options.use_ssao)
	{
		glActiveTexture(GL_TEXTURE0 + texture_idx);
		glBindTexture(GL_TEXTURE_2D, params.tex_ao->tex_id);
		glUniform1i(m_bindings.location_tex_ssao, texture_idx);
		texture_idx++;
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


