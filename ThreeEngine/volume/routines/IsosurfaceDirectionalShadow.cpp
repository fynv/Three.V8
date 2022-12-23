#include <glm.hpp>
#include <string>
#include <GL/glew.h>
#include "IsosurfaceDirectionalShadow.h"
#include "lights/DirectionalLight.h"
#include "lights/DirectionalLightShadow.h"
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


static std::string g_frag =
R"(#version 430

layout (std140, binding = 0) uniform Shadow
{
	mat4 uVPSBMat;
	mat4 uProjMat;
	mat4 uViewMat;	
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
layout (location = 2) uniform sampler2D uDepthTex;

layout (location = 0) in vec2 vPosProj;

void main()
{
	ivec2 coord = ivec2(gl_FragCoord.xy);		
	float depth = texelFetch(uDepthTex, coord, 0).x*2.0-1.0;

	mat4 invProj = inverse(uProjMat);
	mat4 invView = inverse(uViewMat);

	vec4 pos_view0 = invProj * vec4(vPosProj, -1.0, 1.0);
	pos_view0 *= 1.0/pos_view0.w;
	vec4 eye_pos_world = invView * pos_view0;
	vec3 eye_pos = (uInvModelMat * eye_pos_world).xyz;

	vec4 pos_view = invProj * vec4(vPosProj, depth, 1.0);
	pos_view *= 1.0/pos_view.w;
	vec4 pos_world = invView * pos_view;
	vec3 pos_model = (uInvModelMat * pos_world).xyz;
	
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
		pos_world = uModelMat * vec4(pos, 1.0);
		pos_view = uViewMat * pos_world;
		vec4 pos_proj = uProjMat * pos_view;
		pos_proj*= 1.0/pos_proj.w;				
		gl_FragDepth= pos_proj.z * 0.5 + 0.5;
		return;
	}
	
	discard;

}
)";

IsosurfaceDirectionalShadow::IsosurfaceDirectionalShadow()
{
	GLShader vert_shader(GL_VERTEX_SHADER, g_vertex.c_str());
	GLShader frag_shader(GL_FRAGMENT_SHADER, g_frag.c_str());
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




inline void calc_scissor(const DirectionalLightShadow* shadow, const VolumeIsosurfaceModel* model, float width, float height, glm::ivec2& origin, glm::ivec2& size)
{
	origin = { 0,0 };
	size = { 0,0 };

	glm::vec3 min_pos, max_pos;
	model->m_data->GetMinMax(min_pos, max_pos);

	glm::mat4 matView = glm::inverse(shadow->m_light->matrixWorld);
	glm::mat4 MV = matView * model->matrixWorld;
	glm::vec3 min_pos_view, max_pos_view;
	toViewAABB(MV, min_pos, max_pos, min_pos_view, max_pos_view);

	glm::mat4 invP = glm::inverse(shadow->m_light_proj_matrix);
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

	glm::mat4 P = shadow->m_light_proj_matrix;

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


void IsosurfaceDirectionalShadow::render(const RenderParams& params)
{
	GLint i_viewport[4];
	glGetIntegerv(GL_VIEWPORT, i_viewport);

	glm::ivec2 origin, size;
	calc_scissor(params.shadow, params.model, (float)i_viewport[2], (float)i_viewport[3], origin, size);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glEnable(GL_SCISSOR_TEST);
	glScissor(origin.x, origin.y, size.x, size.y);

	glUseProgram(m_prog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, params.shadow->constant_shadow.m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, params.model->m_constant.m_id);
	

	const GLTexture3D& tex = params.model->m_data->texture;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, tex.tex_id);
	glUniform1i(0, 0);

	const GLTexture3D& grid = params.model->m_partition->minmax_texture;
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, grid.tex_id);
	glUniform1i(1, 1);

	glActiveTexture(GL_TEXTURE2);	
	glBindTexture(GL_TEXTURE_2D, params.shadow->m_lightTex);
	glUniform1i(2, 2);

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glUseProgram(0);

	glDisable(GL_SCISSOR_TEST);

	

}

