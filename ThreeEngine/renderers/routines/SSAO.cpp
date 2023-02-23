#include <glm.hpp>
#include <GL/glew.h>
#include "renderers/GLRenderTarget.h"
#include "SSAO.h"


static std::string g_comp_renorm =
R"(#version 430

#DEFINES#

#if MSAA
layout (location = 0) uniform highp sampler2DMS uDepthTex;
#else
layout (location = 0) uniform sampler2D uDepthTex;
#endif

layout (std140, binding = 0) uniform Camera
{
	mat4 uProjMat;
	mat4 uViewMat;	
	mat4 uInvProjMat;
	mat4 uInvViewMat;	
	vec3 uEyePos;
};

layout (binding=1, rgba8) uniform highp writeonly image2D uOutTex;

layout(local_size_x = 8, local_size_y = 8) in;


#if MSAA
float FetchDepth(in ivec2 id)
{
	float depth0 =  texelFetch(uDepthTex, id, 0).x;
	float depth1 =  texelFetch(uDepthTex, id, 1).x;
	float depth2 =  texelFetch(uDepthTex, id, 2).x;
	float depth3 =  texelFetch(uDepthTex, id, 3).x;
	return 0.25 * (depth0 + depth1 + depth2 + depth3);
}
#else
float FetchDepth(in ivec2 id)
{
	return texelFetch(uDepthTex, id, 0).x;
}
#endif

vec3 FetchFullResViewPos(in ivec2 id)
{
#if MSAA
	ivec2 size = textureSize(uDepthTex);
#else
	ivec2 size = textureSize(uDepthTex, 0);
#endif
	vec2 UV = (vec2(id)+0.5)/vec2(size);
	float depth = FetchDepth(id);
	vec3 pos_clip = vec3(UV, depth)*2.0-1.0;
	vec4 view_pos = uInvProjMat* vec4(pos_clip, 1.0);
	view_pos/=view_pos.w;
	return view_pos.xyz;
}

vec3 MinDiff(in vec3 P, in vec3 Pr, in vec3 Pl)
{
    vec3 V1 = Pr - P;
    vec3 V2 = P - Pl;
    return (dot(V1,V1) < dot(V2,V2)) ? V1 : V2;
}

vec3 ReconstructNormal(in ivec2 id, in vec3 P)
{
    vec3 Pr = FetchFullResViewPos(id + ivec2(1, 0));
    vec3 Pl = FetchFullResViewPos(id + ivec2(-1, 0));
    vec3 Pt = FetchFullResViewPos(id + ivec2(0, 1));
    vec3 Pb = FetchFullResViewPos(id + ivec2(0, -1));
    return normalize(cross(MinDiff(P, Pr, Pl), MinDiff(P, Pt, Pb)));
}


void main()
{
#if MSAA
	ivec2 size = textureSize(uDepthTex);
#else
	ivec2 size = textureSize(uDepthTex, 0);
#endif
	ivec2 id = ivec3(gl_GlobalInvocationID).xy;
	if (id.x>= size.x || id.y >=size.y) return;
	vec3 ViewPosition = FetchFullResViewPos(id);
	vec3 ViewNormal = ReconstructNormal(id, ViewPosition);
	imageStore(uOutTex, id, vec4(ViewNormal*0.5 + 0.5, 0.0));
}

)";


static std::string g_comp_deinterleave =
R"(#version 430

#DEFINES#

#if MSAA
layout (location = 0) uniform highp sampler2DMS uDepthTex;
#else
layout (location = 0) uniform sampler2D uDepthTex;
#endif

layout (binding=0, r32f) uniform highp writeonly image2D uOutTex;

layout(local_size_x = 8, local_size_y = 8) in;

#if MSAA
float FetchDepth(in ivec2 id)
{
	float depth0 =  texelFetch(uDepthTex, id, 0).x;
	float depth1 =  texelFetch(uDepthTex, id, 1).x;
	float depth2 =  texelFetch(uDepthTex, id, 2).x;
	float depth3 =  texelFetch(uDepthTex, id, 3).x;
	return 0.25 * (depth0 + depth1 + depth2 + depth3);
}
#else
float FetchDepth(in ivec2 id)
{
	return texelFetch(uDepthTex, id, 0).x;
}
#endif

void main()
{
	ivec2 size_out = imageSize(uOutTex);
	ivec2 id_out = ivec3(gl_GlobalInvocationID).xy;
	if (id_out.x>= size_out.x || id_out.y >=size_out.y) return;
	
	ivec2 block_size = size_out/4;
	ivec2 block_id = id_out / block_size;
	ivec2 sub_id = id_out - block_id * block_size;

#if MSAA
	ivec2 size_in = textureSize(uDepthTex);
#else
	ivec2 size_in = textureSize(uDepthTex, 0);
#endif
	ivec2 id_in = clamp(sub_id * 4 + block_id, ivec2(0), size_in - 1);
	float d = FetchDepth(id_in);
	imageStore(uOutTex, id_out, vec4(d));

}
)";

static std::string g_comp_coarse_ao =
R"(#version 430

layout (location = 0) uniform sampler2D uDepthTex;
layout (location = 1) uniform sampler2D uNormal;

layout (std140, binding = 0) uniform Camera
{
	mat4 uProjMat;
	mat4 uViewMat;	
	mat4 uInvProjMat;
	mat4 uInvViewMat;	
	vec3 uEyePos;
};

layout (binding=1, rg16f) uniform highp writeonly image2D uOutTex;

const float g_fNDotVBias = 0.1;
const float g_fSmallScaleAO = 1.0;
const float g_fLargeScaleAO = 1.0;
const float g_fR = 2.0;
const float g_fR2 = g_fR * g_fR;
const float g_fNegInvR2 = -1.0/g_fR2;

layout(local_size_x = 8, local_size_y = 8) in;

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


ivec2 size, block_size, id_out, block_id, block_offset, sub_id;
uint rand_seed;

void ComputeGlobals()
{
    size = imageSize(uOutTex);    
    block_size = size/4;

    id_out = ivec3(gl_GlobalInvocationID).xy; 
    block_id = id_out / block_size;
    block_offset = block_id* block_size; 
    sub_id = id_out - block_offset;

    rand_seed = InitRandomSeed(uint(id_out.x), uint(id_out.y));
}

vec3 FetchFullResViewNormal(in ivec2 id_full)
{	
    ivec2 size_full = textureSize(uNormal, 0); 
    ivec2 id_fetch = clamp(id_full, ivec2(0), size_full - 1);
	return texelFetch(uNormal, id_fetch, 0).xyz  * 2.0 - 1.0;	
}

vec3 FetchQuarterResViewPos(in ivec2 id_full)
{   
    ivec2 size_full = textureSize(uNormal, 0);
    
    float depth;
    if (id_full.x<0 || id_full.x>=size_full.x || 
        id_full.y<0 || id_full.y >= size_full.y)    
    {
        depth = 1.0;
    }
    else
    {
        ivec2 id = block_offset + id_full/4;
        depth = texelFetch(uDepthTex, id, 0).x;
    }
            
    vec2 uv = (vec2(id_full) + 0.5)/vec2(size_full);
    
	vec3 pos_clip = vec3(uv, depth)*2.0-1.0;
	vec4 view_pos = uInvProjMat* vec4(pos_clip, 1.0);
	view_pos/=view_pos.w;
	return view_pos.xyz;
}

vec2 RotateDirection(in vec2 V, in vec2 RotationCosSin)
{   
    return vec2(V.x*RotationCosSin.x - V.y*RotationCosSin.y,
                  V.x*RotationCosSin.y + V.y*RotationCosSin.x);
}

struct AORadiusParams
{
    float fRadiusPixels;
    float fNegInvR2;
};

AORadiusParams GetAORadiusParams(float ViewZ)
{    
    vec4 view_pos = vec4(g_fR, 0.0, ViewZ, 1.0);
    vec4 clip_pos = uProjMat * view_pos;
    float clip_r = clip_pos.x/clip_pos.w;
    AORadiusParams Params;
    Params.fRadiusPixels = clip_r *0.5 * float(block_size.x);
    Params.fNegInvR2 = g_fNegInvR2;
    return Params;
}

float Falloff(float DistanceSquare, in AORadiusParams Params)
{    
    return DistanceSquare * Params.fNegInvR2 + 1.0;
}


float ComputeAO(in vec3 P, in vec3 N, in vec3 S, AORadiusParams Params)
{
    vec3 V = S - P;
    float VdotV = dot(V, V);
    float NdotV = dot(N, V) / sqrt(VdotV);
    
    return clamp(NdotV - g_fNDotVBias, 0.0, 1.0) * clamp(Falloff(VdotV, Params), 0.0, 1.0);
}

void AccumulateAO(
    inout float AO,
    inout float RayPixels,
    float StepSizePixels,
    in vec2 Direction,
    in ivec2 FullResID,
    in vec3 ViewPosition,
    in vec3 ViewNormal,
    in AORadiusParams Params
)
{    
    ivec2 SnappedID = FullResID + ivec2(round(RayPixels * Direction))*4;
    vec3 S = FetchQuarterResViewPos(SnappedID);
    RayPixels += StepSizePixels;
    AO += ComputeAO(ViewPosition, ViewNormal, S, Params);
}

const int NUM_STEPS = 4;
const int NUM_DIRECTIONS = 8;
const float PI = 3.14159265;

float ComputeCoarseAO(in ivec2 FullResID, in vec3 ViewPosition, in vec3 ViewNormal, AORadiusParams Params)
{
    float StepSizePixels = (Params.fRadiusPixels / 4.0) / float(NUM_STEPS + 1);
    vec4 Rand;
    float r1 = RandomFloat(rand_seed);
	float r2 = RandomFloat(rand_seed);
	float r3 = RandomFloat(rand_seed);
	float angle = 2.0 * PI * r1 / float(NUM_DIRECTIONS);

	Rand.x = cos(angle);
	Rand.y = sin(angle);
	Rand.z = r2;
	Rand.w = r3;
    
    const float Alpha = 2.0 * PI / float(NUM_DIRECTIONS);
    float SmallScaleAO = 0.0;
    float LargeScaleAO = 0.0;

    for (int DirectionIndex = 0; DirectionIndex < NUM_DIRECTIONS; ++DirectionIndex)
    {
        float Angle = Alpha * float(DirectionIndex);
        vec2 Direction = RotateDirection(vec2(cos(Angle), sin(Angle)), Rand.xy);
        float RayPixels = (Rand.z * StepSizePixels + 1.0);
        AccumulateAO(SmallScaleAO, RayPixels, StepSizePixels, Direction, FullResID, ViewPosition, ViewNormal, Params);
        for (int StepIndex = 1; StepIndex < NUM_STEPS; ++StepIndex)
        {
            AccumulateAO(LargeScaleAO, RayPixels, StepSizePixels, Direction, FullResID, ViewPosition, ViewNormal, Params);
        }
    }    

    float AOAmountScaleFactor = 1.0 / (1.0 - g_fNDotVBias);
    float fSmallScaleAOAmount = g_fSmallScaleAO * AOAmountScaleFactor * 2.0;
    float fLargeScaleAOAmount = g_fLargeScaleAO * AOAmountScaleFactor;

    float AO = (SmallScaleAO * fSmallScaleAOAmount) + (LargeScaleAO * fLargeScaleAOAmount);

    AO /= float(NUM_DIRECTIONS * NUM_STEPS);

    return AO;
}


void main()
{
    ComputeGlobals(); 
    if (id_out.x>= size.x || id_out.y >=size.y) return;
    	
    ivec2 id_full = sub_id * 4 + block_id;    
    vec3 ViewPosition = FetchQuarterResViewPos(id_full);
    vec3 ViewNormal = FetchFullResViewNormal(id_full);

    AORadiusParams Params = GetAORadiusParams(ViewPosition.z);
    float AO = 1.0;
    if (Params.fRadiusPixels >= 1.0)
    {
        AO = ComputeCoarseAO(id_full, ViewPosition, ViewNormal, Params);
        AO = clamp(1.0 - AO * 2.0, 0.0, 1.0);
    }    
	float z = -ViewPosition.z;
    imageStore(uOutTex, id_out, vec4(AO,z,AO,z));
}

)";


static std::string g_comp_reinterleave =
R"(#version 430
layout (location = 0) uniform sampler2D uAOTex;
layout (binding=0, rg16f) uniform highp writeonly image2D uOutTex;

layout(local_size_x = 8, local_size_y = 8) in;

void main()
{
	ivec2 size_out = imageSize(uOutTex); 
	ivec2 id_out = ivec3(gl_GlobalInvocationID).xy;
	if (id_out.x>= size_out.x || id_out.y >=size_out.y) return;	

	ivec2 size_in = textureSize(uAOTex, 0);
	ivec2 block_size = size_in/4;
	ivec2 sub_id = id_out/4;
	ivec2 block_id = id_out - sub_id * 4;
	ivec2 id_in = block_size * block_id + sub_id;
	
	vec2 aoz = texelFetch(uAOTex, id_in, 0).xy;
	imageStore(uOutTex, id_out, vec4(aoz, aoz));
}
)";

static std::string g_comp_blur_common =
R"(#version 430

layout (location = 0) uniform sampler2D uAOInTex;
layout (binding=0, rg16f) uniform image2D uAOOutTex;

const int KERNEL_RADIUS = 4;

vec2 PointSampleAODepth(in vec2 UV)
{
    ivec2 size = imageSize(uAOOutTex); 
    ivec2 id = ivec2(UV * vec2(size));
    return texelFetch(uAOInTex, id, 0).xy;
}
vec2 LinearSampleAODepth(in vec2 UV)
{
    return textureLod(uAOInTex, UV, 0.0).xy;
}

struct CenterPixelData
{
    vec2 UV;
    float Depth;
    float Sharpness;
    float Scale;
    float Bias;
};

float CrossBilateralWeight(float R, float SampleDepth, float DepthSlope, in CenterPixelData Center)
{
    const float BlurSigma = (float(KERNEL_RADIUS)+1.0) * 0.5;
    const float BlurFalloff = 1.0 / (2.0*BlurSigma*BlurSigma);
    SampleDepth -= DepthSlope * R;
    float DeltaZ = SampleDepth * Center.Scale + Center.Bias;
    return exp2(-R*R*BlurFalloff - DeltaZ*DeltaZ);
}


void ProcessSample(in vec2 AOZ,
                   float R,
                   float DepthSlope,
                   in CenterPixelData Center,
                   inout float TotalAO,
                   inout float TotalW)
{
    float AO = AOZ.x;
    float Z = AOZ.y;

    float W = CrossBilateralWeight(R, Z, DepthSlope, Center);
    TotalAO += W * AO;
    TotalW += W;
}

void ProcessRadius(float R0,
                   in vec2 DeltaUV,
                   float DepthSlope,
                   in CenterPixelData Center,
                   inout float TotalAO,
                   inout float TotalW)
{
    float R = R0;
    for (; R <= float(KERNEL_RADIUS)/2.0; R += 1.0)
    {
        vec2 UV = R * DeltaUV + Center.UV;
        vec2 AOZ = PointSampleAODepth(UV);
        ProcessSample(AOZ, R, DepthSlope, Center, TotalAO, TotalW);
    }

    for (; R <= float(KERNEL_RADIUS); R += 2.0)
    {
        vec2 UV = (R + 0.5) * DeltaUV + Center.UV;
        vec2 AOZ = LinearSampleAODepth(UV);
        ProcessSample(AOZ, R, DepthSlope, Center, TotalAO, TotalW);
    }
}

void ProcessRadius1(in vec2 DeltaUV,
                    in CenterPixelData Center,
                    inout float TotalAO,
                    inout float TotalW)
{
    vec2 AODepth = PointSampleAODepth(Center.UV + DeltaUV);
    float DepthSlope = AODepth.y - Center.Depth;

    ProcessSample(AODepth, 1.0, DepthSlope, Center, TotalAO, TotalW);
    ProcessRadius(2.0, DeltaUV, DepthSlope, Center, TotalAO, TotalW);
}

const float BaseSharpness = 16.0;

float ComputeBlur(in vec2 DeltaUV, out float CenterDepth)
{
    ivec2 size = imageSize(uAOOutTex); 
    ivec2 id = ivec3(gl_GlobalInvocationID).xy;
    vec2 AOZ = texelFetch(uAOInTex, id, 0).xy;
    CenterDepth = AOZ.y;

    CenterPixelData Center;
    Center.UV = (vec2(id) + 0.5)/vec2(size);
    Center.Depth = CenterDepth;
    Center.Sharpness = BaseSharpness;
    Center.Scale = Center.Sharpness;
    Center.Bias = -Center.Depth * Center.Sharpness;

    float TotalAO = AOZ.x;
    float TotalW = 1.0;

    ProcessRadius1(DeltaUV, Center, TotalAO, TotalW);
    ProcessRadius1(-DeltaUV, Center, TotalAO, TotalW);
    
    return TotalAO / TotalW;
}
)";

static std::string g_comp_blur_x =
R"(
layout(local_size_x = 8, local_size_y = 8) in;

void main()
{
    ivec2 size = imageSize(uAOOutTex); 
    ivec2 id = ivec3(gl_GlobalInvocationID).xy;
    if (id.x>= size.x || id.y >=size.y) return;

    float z; 
    vec2 DeltaUV = vec2(1.0/float(size.x),0.0);
    float AO = ComputeBlur(DeltaUV, z);
    imageStore(uAOOutTex, id, vec4(AO,z,AO,z));
}
)";

static std::string g_comp_blur_y =
R"(
layout(local_size_x = 8, local_size_y = 8) in;

void main()
{
    ivec2 size = imageSize(uAOOutTex); 
    ivec2 id = ivec3(gl_GlobalInvocationID).xy;
    if (id.x>= size.x || id.y >=size.y) return;

    float z; 
    vec2 DeltaUV = vec2(0.0, 1.0/float(size.y));
    float AO = ComputeBlur(DeltaUV, z);
    imageStore(uAOOutTex, id, vec4(AO,z,AO,z));
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


SSAO::SSAO(bool msaa) : m_msaa(msaa)
{
	{
		std::string s_comp = g_comp_renorm;

		std::string defines = "";
		if (msaa)
		{
			defines += "#define MSAA 1\n";
		}
		else
		{
			defines += "#define MSAA 0\n";
		}

		replace(s_comp, "#DEFINES#", defines.c_str());

		GLShader comp_shader(GL_COMPUTE_SHADER, s_comp.c_str());
		m_prog_reconstruct_normal = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
	}

	{
		std::string s_comp = g_comp_deinterleave;

		std::string defines = "";
		if (msaa)
		{
			defines += "#define MSAA 1\n";
		}
		else
		{
			defines += "#define MSAA 0\n";
		}

		replace(s_comp, "#DEFINES#", defines.c_str());

		GLShader comp_shader(GL_COMPUTE_SHADER, s_comp.c_str());
		m_prog_deinterleave = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
	}

	{
		GLShader comp_shader(GL_COMPUTE_SHADER, g_comp_coarse_ao.c_str());
		m_prog_coarse_ao = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
	}

	{
		GLShader comp_shader(GL_COMPUTE_SHADER, g_comp_reinterleave.c_str());
		m_prog_reinterleave = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
	}
	{
		std::string s_comp = g_comp_blur_common + g_comp_blur_x;
		GLShader comp_shader(GL_COMPUTE_SHADER, s_comp.c_str());
		m_prog_blur_x = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
	}
	{
		std::string s_comp = g_comp_blur_common + g_comp_blur_y;
		GLShader comp_shader(GL_COMPUTE_SHADER, s_comp.c_str());
		m_prog_blur_y = (std::unique_ptr<GLProgram>)(new GLProgram(comp_shader));
	}
}

SSAO::~SSAO()
{

}

SSAO::Buffers::Buffers()
{

}

SSAO::Buffers::~Buffers()
{

}

void SSAO::Buffers::update(int width, int height)
{
	if (m_width != width || m_height != height)
	{
		m_width = width;
		m_height = height;
		m_quat_width = (width + 3) / 4 * 4;
		m_quat_height = (height + 3) / 4 * 4;

		{
			m_tex_norm = std::unique_ptr<GLTexture2D>(new GLTexture2D);
			glBindTexture(GL_TEXTURE_2D, m_tex_norm->tex_id);			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);			
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		{
			m_tex_deinterleave_depth = std::unique_ptr<GLTexture2D>(new GLTexture2D);
			glBindTexture(GL_TEXTURE_2D, m_tex_deinterleave_depth->tex_id);			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, m_quat_width, m_quat_height);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		{
			m_tex_aoz_quat = std::unique_ptr<GLTexture2D>(new GLTexture2D);
			glBindTexture(GL_TEXTURE_2D, m_tex_aoz_quat->tex_id);			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG16F, m_quat_width, m_quat_height);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		{
			m_tex_aoz = std::unique_ptr<GLTexture2D>(new GLTexture2D);
			glBindTexture(GL_TEXTURE_2D, m_tex_aoz->tex_id);			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG16F, width, height);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		{
			m_tex_aoz2 = std::unique_ptr<GLTexture2D>(new GLTexture2D);
			glBindTexture(GL_TEXTURE_2D, m_tex_aoz2->tex_id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG16F, width, height);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
}

void SSAO::render(const RenderParams& params)
{
	// recontruct normal
	{
		glUseProgram(m_prog_reconstruct_normal->m_id);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.constant_camera->m_id);

		glActiveTexture(GL_TEXTURE0);
		if (m_msaa)
		{
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, params.depth_in->tex_id);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, params.depth_in->tex_id);
		}
		glUniform1i(0, 0);

		glBindImageTexture(1, params.buffers->m_tex_norm->tex_id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);

		glm::ivec2 blocks = { (params.buffers->m_width + 7) / 8, (params.buffers->m_height + 7) / 8 };
		glDispatchCompute(blocks.x, blocks.y, 1);
		glUseProgram(0);
	}

	// deinterleave depth
	{
		glUseProgram(m_prog_deinterleave->m_id);

		glActiveTexture(GL_TEXTURE0);
		if (m_msaa)
		{
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, params.depth_in->tex_id);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, params.depth_in->tex_id);
		}
		glUniform1i(0, 0);

		glBindImageTexture(0, params.buffers->m_tex_deinterleave_depth->tex_id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);

		glm::ivec2 blocks = { (params.buffers->m_quat_width + 7) / 8, (params.buffers->m_quat_height + 7) / 8 };
		glDispatchCompute(blocks.x, blocks.y, 1);
		glUseProgram(0);
	}

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	// coarse-ao
	{		
		glUseProgram(m_prog_coarse_ao->m_id);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, params.buffers->m_tex_deinterleave_depth->tex_id);
		glUniform1i(0, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, params.buffers->m_tex_norm->tex_id);
		glUniform1i(1, 1);

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.constant_camera->m_id);		
		glBindImageTexture(1, params.buffers->m_tex_aoz_quat->tex_id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG16F);

		glm::ivec2 blocks = { (params.buffers->m_quat_width + 7) / 8, (params.buffers->m_quat_height + 7) / 8 };
		glDispatchCompute(blocks.x, blocks.y, 1);

		glUseProgram(0);

	}

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	// reinterleave
	{
		glUseProgram(m_prog_reinterleave->m_id);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, params.buffers->m_tex_aoz_quat->tex_id);
		glUniform1i(0, 0);

		glBindImageTexture(0, params.buffers->m_tex_aoz->tex_id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG16F);
		glm::ivec2 blocks = { (params.buffers->m_width + 7) / 8, (params.buffers->m_height + 7) / 8 };
		glDispatchCompute(blocks.x, blocks.y, 1);

		glUseProgram(0);

	}

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	// blur_x
	{
		glUseProgram(m_prog_blur_x->m_id);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, params.buffers->m_tex_aoz->tex_id);
		glUniform1i(0, 0);

		glBindImageTexture(0, params.buffers->m_tex_aoz2->tex_id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG16F);
		glm::ivec2 blocks = { (params.buffers->m_width + 7) / 8, (params.buffers->m_height + 7) / 8 };
		glDispatchCompute(blocks.x, blocks.y, 1);

		glUseProgram(0);
	}
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	// blur_y
	{
		glUseProgram(m_prog_blur_y->m_id);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, params.buffers->m_tex_aoz2->tex_id);
		glUniform1i(0, 0);

		glBindImageTexture(0, params.buffers->m_tex_aoz->tex_id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG16F);
		glm::ivec2 blocks = { (params.buffers->m_width + 7) / 8, (params.buffers->m_height + 7) / 8 };
		glDispatchCompute(blocks.x, blocks.y, 1);

		glUseProgram(0);

	}

}