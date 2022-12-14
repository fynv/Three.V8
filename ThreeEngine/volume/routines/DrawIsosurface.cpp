#include <glm.hpp>
#include <string>
#include <GL/glew.h>
#include "DrawIsosurface.h"
#include "cameras/Camera.h"
#include "volume/VolumeIsosurfaceModel.h"


static std::string g_vertex =
R"(#version 430
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
	vec3 uSpacing;
	float uIsovalue;
};

layout (location = 0) out vec3 vDirModel;

void main()
{
    vec2 grid = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    vec2 pos_proj = grid * vec2(2.0, 2.0) + vec2(-1.0, -1.0);
    gl_Position = vec4(pos_proj, 1.0, 1.0);

	vec4 pos_view = uInvProjMat * gl_Position;
    pos_view = pos_view/pos_view.w;
    vec3 dir_world = mat3(uInvViewMat) * pos_view.xyz;
	vDirModel = mat3(uInvModelMat) * dir_world;
};
)";


static std::string g_frag =
R"(#version 430

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
	vec3 uSpacing;
	float uIsovalue;
};

layout (location = 0) uniform sampler3D uTex;

layout (location = 0) in vec3 vDirModel;
layout (location = 0) out vec4 outColor;

vec3 get_norm(in vec3 pos, in vec3 min_pos, in vec3 max_pos)
{
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
	return -normalize(delta/uSpacing);
}

void main()
{
	vec3 eye_pos = (uInvModelMat * vec4(uEyePos, 1.0)).xyz;
	vec3 dir = normalize(vDirModel);
	vec3 min_pos = - uSize.xyz*uSpacing *0.5;
	vec3 max_pos = uSize.xyz*uSpacing *0.5;
	
	vec3 t_min;
	t_min.x = (min_pos.x - eye_pos.x)/dir.x;
	t_min.y = (min_pos.y - eye_pos.y)/dir.y;
	t_min.z = (min_pos.z - eye_pos.z)/dir.z;

	vec3 t_max;
	t_max.x = (max_pos.x - eye_pos.x)/dir.x;
	t_max.y = (max_pos.y - eye_pos.y)/dir.y;
	t_max.z = (max_pos.z - eye_pos.z)/dir.z;

	vec3 t0;
	t0.x = min(t_min.x, t_max.x);
	t0.y = min(t_min.y, t_max.y);
	t0.z = min(t_min.z, t_max.z);
	float t_start = max(max(t0.x, t0.y), t0.z);

	vec3 t1;
	t1.x = max(t_min.x, t_max.x);
	t1.y = max(t_min.y, t_max.y);
	t1.z = max(t_min.z, t_max.z);
	float t_stop = min(min(t1.x, t1.y), t1.z);

	if (t_stop<t_start) discard;

	float t = t_start;
	float step = 0.001;
	float v0 = 0.0;
	float iso = 0.5;
	while(t<t_stop)
	{
		vec3 pos = eye_pos + t*dir;
		vec3 coord = (pos - min_pos)/(max_pos-min_pos);
		float v1 = texture(uTex, coord).x;
		if (v1>iso)
		{
			float k = (iso - v1)/(v1-v0);
			t += k*step;
			vec3 norm = get_norm(eye_pos + t*dir, min_pos, max_pos);			
			{
				float k = (norm.z + 1.0)*0.5;
				vec3 col = vec3(0.318, 0.318, 0.318)*k + vec3(0.01, 0.025, 0.025)*(1.0-k);
				outColor = vec4(col, 1.0);
			}			
			return;
		}

		v0 = v1;
		t+=step;
	}
	discard;
	
}
)";

DrawIsosurface::DrawIsosurface()
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

void DrawIsosurface::render(const Camera* camera, const VolumeIsosurfaceModel* model)
{
	GLint i_viewport[4];
	glGetIntegerv(GL_VIEWPORT, i_viewport);	

	glm::ivec2 origin, size;
	calc_scissor(camera, model, (float)i_viewport[2], (float)i_viewport[3], origin, size);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glEnable(GL_SCISSOR_TEST);
	glScissor(origin.x, origin.y, size.x, size.y);

	glUseProgram(m_prog->m_id);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, camera->m_constant.m_id);	
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, model->m_constant.m_id);

	const GLTexture3D& tex = model->m_data->texture;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, tex.tex_id);
	glUniform1i(0,0);

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glUseProgram(0);

	glDisable(GL_SCISSOR_TEST);

}

