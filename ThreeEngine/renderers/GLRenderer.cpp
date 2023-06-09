#include <GL/glew.h>
#include <gtc/random.hpp>
#include "crc64/crc64.h"
#include "GLUtils.h"
#include "GLRenderer.h"
#include "GLRenderTarget.h"
#include "LightmapRenderTarget.h"
#include "GLPickingTarget.h"
#include "GLSpaceProbeTarget.h"
#include "CubeRenderTarget.h"
#include "BVHRenderTarget.h"
#include "ProbeRenderTarget.h"
#include "cameras/Camera.h"
#include "cameras/PerspectiveCamera.h"
#include "scenes/Scene.h"
#include "backgrounds/Background.h"
#include "backgrounds/BackgroundScene.h"
#include "models/ModelComponents.h"
#include "models/SimpleModel.h"
#include "models/GLTFModel.h"
#include "volume/VolumeIsosurfaceModel.h"
#include "materials/MeshStandardMaterial.h"
#include "lights/DirectionalLight.h"
#include "lights/DirectionalLightShadow.h"
#include "scenes/Fog.h"
#include "lights/ProbeGridWidget.h"
#include "lights/LODProbeGridWidget.h"
#include "lights/ProbeRayList.h"
#include "LightmapRayList.h"

//#include <gtx/string_cast.hpp>

const double PI = 3.14159265359;

inline double rand01()
{
	return (double)rand() / ((double)RAND_MAX + 1.0);
}

inline double randRad()
{
	return rand01() * 2.0 * PI;
}



GLRenderer::GLRenderer()
{
	
}

GLRenderer::~GLRenderer()
{

}

void GLRenderer::update_model(SimpleModel* model)
{
	model->updateConstant();
}

void GLRenderer::update_model(GLTFModel* model)
{
	for (size_t i = 0; i < model->m_meshs.size(); i++)
	{
		Mesh& mesh = model->m_meshs[i];

		if (mesh.needUpdateMorphTargets)
		{
			mesh.buf_weights->upload(mesh.weights.data());
			for (size_t j = 0; j < mesh.primitives.size(); j++)
			{
				Primitive& primitive = mesh.primitives[j];
				bool has_tangent = primitive.geometry[0].tangent_buf != nullptr;
				bool sparse = primitive.none_zero_buf!=nullptr;
				int idx_morpher = (has_tangent ? 1 : 0) + (sparse ? 2 : 0);
				
				if (morphers[idx_morpher] == nullptr)
				{
					morphers[idx_morpher] = std::unique_ptr<MorphUpdate>(new MorphUpdate(has_tangent, sparse));
				}
				MorphUpdate* morpher = morphers[idx_morpher].get();

				MorphUpdate::Params params = {
					mesh.buf_weights.get(),
					&primitive
				};
				morpher->update(params);
			}
			mesh.needUpdateMorphTargets = false;
		}
	}

	if (model->needUpdateSkinnedMeshes)
	{
		for (size_t i = 0; i < model->m_meshs.size(); i++)
		{
			Mesh& mesh = model->m_meshs[i];
			if (mesh.skin_id >= 0)
			{
				Skin& skin = model->m_skins[mesh.skin_id];
				for (size_t j = 0; j < mesh.primitives.size(); j++)
				{
					Primitive& primitive = mesh.primitives[j];
					bool has_tangent = primitive.geometry[0].tangent_buf != nullptr;
					SkinUpdate* skinner = nullptr;
					if (!has_tangent)
					{
						if (skinners[0] == nullptr)
						{
							skinners[0] = std::unique_ptr<SkinUpdate>(new SkinUpdate(false));
						}
						skinner = skinners[0].get();
					}
					else
					{
						if (skinners[1] == nullptr)
						{
							skinners[1] = std::unique_ptr<SkinUpdate>(new SkinUpdate(true));
						}
						skinner = skinners[1].get();
					}

					SkinUpdate::Params params = {
						&skin,
						&primitive
					};
					skinner->update(params);
				}
			}
		}
		model->needUpdateSkinnedMeshes = false;
	}	

	model->updateMeshConstants();
}

void GLRenderer::update_model(VolumeIsosurfaceModel* model)
{
	model->updateConstant();
}

StandardRoutine* GLRenderer::get_routine(const StandardRoutine::Options& options)
{
	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(StandardRoutine::Options));
	auto iter = routine_map.find(hash);
	if (iter == routine_map.end())
	{
		routine_map[hash] = std::unique_ptr<StandardRoutine>(new StandardRoutine(options));
	}
	return routine_map[hash].get();
}


SimpleRoutine* GLRenderer::get_simple_routine(const SimpleRoutine::Options& options)
{
	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(SimpleRoutine::Options));
	auto iter = simple_routine_map.find(hash);
	if (iter == simple_routine_map.end())
	{
		simple_routine_map[hash] = std::unique_ptr<SimpleRoutine>(new SimpleRoutine(options));
	}
	return simple_routine_map[hash].get();
}

DrawIsosurface* GLRenderer::get_isosurface_draw(const DrawIsosurface::Options& options)
{
	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(DrawIsosurface::Options));
	auto iter = IsosurfaceDraw.find(hash);
	if (iter == IsosurfaceDraw.end())
	{
		IsosurfaceDraw[hash] = std::unique_ptr<DrawIsosurface>(new DrawIsosurface(options));
	}
	return IsosurfaceDraw[hash].get();

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

inline bool visible(const glm::mat4& MV, const glm::mat4& P, const glm::vec3& min_pos, const glm::vec3& max_pos)
{
	glm::vec3 min_pos_view, max_pos_view;
	toViewAABB(MV, min_pos, max_pos, min_pos_view, max_pos_view);

	glm::mat4 invP = glm::inverse(P);
	glm::vec4 view_far = invP * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	view_far /= view_far.w;
	glm::vec4 view_near = invP * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
	view_near /= view_near.w;
	
	if (min_pos_view.z > view_near.z) return false;
	if (max_pos_view.z < view_far.z) return false;
	if (min_pos_view.z < view_far.z)
	{
		min_pos_view.z = view_far.z;
	}	

	glm::vec4 min_pos_proj = P * glm::vec4(min_pos_view.x, min_pos_view.y, min_pos_view.z, 1.0f);
	min_pos_proj /= min_pos_proj.w;

	glm::vec4 max_pos_proj = P * glm::vec4(max_pos_view.x, max_pos_view.y, min_pos_view.z, 1.0f);
	max_pos_proj /= max_pos_proj.w;

	return  max_pos_proj.x >= -1.0f && min_pos_proj.x <= 1.0f && max_pos_proj.y >= -1.0f && min_pos_proj.y <= 1.0f;
}

void GLRenderer::render_primitive(const StandardRoutine::RenderParams& params, Pass pass)
{
	const MeshStandardMaterial* material = params.material_list[params.primitive->material_idx];
	const Lights* lights = params.lights;	

	if (pass == Pass::Opaque && material->alphaMode== AlphaMode::Opaque &&  material->tone_shading > 0)
	{
		if (WireDraw == nullptr)
		{
			WireDraw = std::unique_ptr<DrawWire>(new DrawWire);
		}
		DrawWire::RenderParams params2;
		params2.constant_camera = params.constant_camera;
		params2.constant_model = params.constant_model;
		params2.primitive = params.primitive;
		params2.radius = material->wire_width;
		params2.wire_color = material->wire_color;
		WireDraw->render(params2);		
	}
	
	StandardRoutine::Options options;
	options.alpha_mode = material->alphaMode;
	options.is_highlight_pass = pass == Pass::Highlight;
	options.specular_glossiness = material->specular_glossiness;
	options.has_color = params.primitive->color_buf != nullptr;
	options.has_color_texture = material->tex_idx_map >= 0;
	options.has_metalness_map = material->tex_idx_metalnessMap >= 0;
	options.has_roughness_map = material->tex_idx_roughnessMap >= 0;
	options.has_normal_map = material->tex_idx_normalMap >= 0;
	options.has_emissive_map = material->tex_idx_emissiveMap >= 0;
	options.has_specular_map = material->tex_idx_specularMap >= 0;
	options.has_glossiness_map = material->tex_idx_glossinessMap >= 0;
	options.num_directional_lights = lights->num_directional_lights;
	options.num_directional_shadows = lights->num_directional_shadows;
	options.has_lightmap = params.tex_lightmap != nullptr;	
	if (!options.has_lightmap)
	{
		options.has_environment_map = lights->environment_map != nullptr;
		options.has_probe_grid = lights->probe_grid != nullptr;
		if (options.has_probe_grid)
		{
			if (lights->probe_grid->per_primitive)
			{
				options.has_probe_grid = false;
				options.has_environment_map = true;
			}
			else
			{
				options.probe_reference_recorded = lights->probe_grid->record_references;
			}
		}
		options.has_lod_probe_grid = lights->lod_probe_grid != nullptr;
		if (options.has_lod_probe_grid)
		{
			if (lights->lod_probe_grid->per_primitive)
			{
				options.has_lod_probe_grid = false;
				options.has_environment_map = true;
			}
		}
		options.has_ambient_light = lights->ambient_light != nullptr;
		options.has_hemisphere_light = lights->hemisphere_light != nullptr;
		options.use_ssao = m_use_ssao;
	}

	options.has_reflection_map = lights->reflection_map != nullptr;
	if (options.has_reflection_map)
	{
		if (lights->reflection_map->tex_id_dis != (unsigned)(-1))
		{
			options.has_reflection_distance = true;
		}
	}

	options.has_fog = params.constant_fog != nullptr;	
	options.tone_shading = material->tone_shading;
	StandardRoutine* routine = get_routine(options);
	routine->render(params);	
}

void GLRenderer::render_primitives(const StandardRoutine::RenderParams& params, Pass pass, const std::vector<void*>& offset_lst, const std::vector<int>& count_lst)
{
	const MeshStandardMaterial* material = params.material_list[params.primitive->material_idx];
	const Lights* lights = params.lights;

	StandardRoutine::Options options;
	options.alpha_mode = material->alphaMode;
	options.is_highlight_pass = pass == Pass::Highlight;
	options.specular_glossiness = material->specular_glossiness;
	options.has_color = params.primitive->color_buf != nullptr;
	options.has_color_texture = material->tex_idx_map >= 0;
	options.has_metalness_map = material->tex_idx_metalnessMap >= 0;
	options.has_roughness_map = material->tex_idx_roughnessMap >= 0;
	options.has_normal_map = material->tex_idx_normalMap >= 0;
	options.has_emissive_map = material->tex_idx_emissiveMap >= 0;
	options.has_specular_map = material->tex_idx_specularMap >= 0;
	options.has_glossiness_map = material->tex_idx_glossinessMap >= 0;
	options.num_directional_lights = lights->num_directional_lights;
	options.num_directional_shadows = lights->num_directional_shadows;
	options.has_lightmap = params.tex_lightmap != nullptr;	
	if (!options.has_lightmap)
	{
		options.has_environment_map = lights->environment_map != nullptr;
		options.has_probe_grid = lights->probe_grid != nullptr;
		if (options.has_probe_grid)
		{
			options.probe_reference_recorded = lights->probe_grid->record_references;
		}
		options.has_lod_probe_grid = lights->lod_probe_grid != nullptr;
		options.has_ambient_light = lights->ambient_light != nullptr;
		options.has_hemisphere_light = lights->hemisphere_light != nullptr;
		options.use_ssao = m_use_ssao;
	}
	options.has_reflection_map = lights->reflection_map != nullptr;
	if (options.has_reflection_map)
	{
		if (lights->reflection_map->tex_id_dis != (unsigned)(-1))
		{
			options.has_reflection_distance = true;
		}
	}
	options.has_fog = params.constant_fog != nullptr;	
	options.tone_shading = material->tone_shading;
	StandardRoutine* routine = get_routine(options);
	routine->render_batched(params, offset_lst, count_lst);

}

void GLRenderer::render_model(Camera* p_camera, const Lights& lights, const Fog* fog, SimpleModel* model, GLRenderTarget& target, Pass pass)
{		
	const GLTexture2D* tex = &model->texture;
	if (model->repl_texture != nullptr)
	{
		tex = model->repl_texture;
	}

	const MeshStandardMaterial* material = &model->material;

	if (pass == Pass::Opaque)
	{
		if (material->alphaMode == AlphaMode::Blend) return;
	}
	else if (pass == Pass::Alpha || pass == Pass::Highlight)
	{
		if (material->alphaMode != AlphaMode::Blend) return;
	}

	if (material->tone_shading > 0 && model->geometry.wire_ind_buf==nullptr)
	{
		model->geometry.compute_wires();
	}

	StandardRoutine::RenderParams params;
	params.tex_list = &tex;
	params.material_list = &material;
	params.constant_camera = &p_camera->m_constant;
	params.constant_model = &model->m_constant;
	params.primitive = &model->geometry;
	params.lights = &lights;
	params.tex_lightmap = nullptr;

	if (fog != nullptr)
	{
		params.constant_fog = &fog->m_constant;
	}
	else
	{
		params.constant_fog = nullptr;
	}

	if (m_use_ssao)
	{
		params.tex_ao = target.m_ssao_buffers->m_tex_aoz.get();
	}
	else
	{
		params.tex_ao = nullptr;
	}

	render_primitive(params, pass);
}

void GLRenderer::render_model(Camera* p_camera, const Lights& lights, const Fog* fog, GLTFModel* model, GLRenderTarget& target, Pass pass)
{
	std::vector<const GLTexture2D*> tex_lst(model->m_textures.size());
	for (size_t i = 0; i < tex_lst.size(); i++)         
	{ 
		auto iter = model->m_repl_textures.find(i);
		if (iter != model->m_repl_textures.end())
		{
			tex_lst[i] = iter->second;
		}
		else
		{
			tex_lst[i] = model->m_textures[i].get();
		}
	}

	std::vector<const MeshStandardMaterial*> material_lst(model->m_materials.size());
	for (size_t i = 0; i < material_lst.size(); i++)
		material_lst[i] = model->m_materials[i].get();

	if (model->batched_mesh != nullptr)
	{
		std::vector<std::vector<void*>> offset_lists(material_lst.size());
		std::vector<std::vector<int>> count_lists(material_lst.size());
		for (size_t i = 0; i < model->m_meshs.size(); i++)
		{
			Mesh& mesh = model->m_meshs[i];
			glm::mat4 matrix = model->matrixWorld;
			if (mesh.node_id >= 0 && mesh.skin_id < 0)
			{
				Node& node = model->m_nodes[mesh.node_id];
				matrix *= node.g_trans;
			}
			glm::mat4 MV = p_camera->matrixWorldInverse * matrix;

			for (size_t j = 0; j < mesh.primitives.size(); j++)
			{
				Primitive& primitive = mesh.primitives[j];
				if (!visible(MV, p_camera->projectionMatrix, primitive.min_pos, primitive.max_pos)) continue;

				int idx_list = primitive.material_idx;
				offset_lists[idx_list].push_back((void*)(model->batch_map[i][j]));
				count_lists[idx_list].push_back(primitive.num_face * 3);
			}
		}
		
		{
			Mesh& mesh = *model->batched_mesh;
			for (size_t j = 0; j < mesh.primitives.size(); j++)
			{
				Primitive& primitive = mesh.primitives[j];
				int idx_material = primitive.material_idx;

				std::vector<void*>& material_offsets = offset_lists[idx_material];
				std::vector<int>& material_counts = count_lists[idx_material];
				if (material_offsets.size() < 1) continue;				

				const MeshStandardMaterial* material = material_lst[idx_material];
				if (pass == Pass::Opaque)
				{
					if (material->alphaMode == AlphaMode::Blend) continue;
				}
				else if (pass == Pass::Alpha || pass == Pass::Highlight)
				{
					if (material->alphaMode != AlphaMode::Blend) continue;
				}

				StandardRoutine::RenderParams params;
				params.tex_list = tex_lst.data();
				params.material_list = material_lst.data();
				params.constant_camera = &p_camera->m_constant;
				params.constant_model = mesh.model_constant.get();
				params.primitive = &primitive;
				params.lights = &lights;
				params.tex_lightmap = nullptr;
				if (model->lightmap != nullptr)
				{
					params.tex_lightmap = model->lightmap->lightmap.get();
				}

				if (fog != nullptr)
				{
					params.constant_fog = &fog->m_constant;
				}
				else
				{
					params.constant_fog = nullptr;
				}

				if (m_use_ssao)
				{
					params.tex_ao = target.m_ssao_buffers->m_tex_aoz.get();
				}
				else
				{
					params.tex_ao = nullptr;
				}
				render_primitives(params, pass, material_offsets, material_counts);				
			}

		}

	}
	else
	{
		for (size_t i = 0; i < model->m_meshs.size(); i++)
		{
			Mesh& mesh = model->m_meshs[i];
			glm::mat4 matrix = model->matrixWorld;
			if (mesh.node_id >= 0 && mesh.skin_id < 0)
			{
				Node& node = model->m_nodes[mesh.node_id];
				matrix *= node.g_trans;
			}
			glm::mat4 MV = p_camera->matrixWorldInverse * matrix;

			for (size_t j = 0; j < mesh.primitives.size(); j++)
			{
				Primitive& primitive = mesh.primitives[j];
				if (!visible(MV, p_camera->projectionMatrix, primitive.min_pos, primitive.max_pos)) continue;

				const MeshStandardMaterial* material = material_lst[primitive.material_idx];
				if (pass == Pass::Opaque)
				{
					if (material->alphaMode == AlphaMode::Blend) continue;
				}
				else if (pass == Pass::Alpha || pass == Pass::Highlight)
				{
					if (material->alphaMode != AlphaMode::Blend) continue;
				}
				if (material->tone_shading > 0 && primitive.wire_ind_buf == nullptr)
				{
					primitive.compute_wires();
				}
				StandardRoutine::RenderParams params;
				params.tex_list = tex_lst.data();
				params.material_list = material_lst.data();
				params.constant_camera = &p_camera->m_constant;
				params.constant_model = mesh.model_constant.get();
				params.primitive = &primitive;
				params.lights = &lights;
				params.tex_lightmap = nullptr;
				if (model->lightmap != nullptr)
				{
					params.tex_lightmap = model->lightmap->lightmap.get();
				}

				if (fog != nullptr)
				{
					params.constant_fog = &fog->m_constant;
				}
				else
				{
					params.constant_fog = nullptr;
				}

				if (m_use_ssao)
				{
					params.tex_ao = target.m_ssao_buffers->m_tex_aoz.get();
				}
				else
				{
					params.tex_ao = nullptr;
				}
				render_primitive(params, pass);
			}
		}
	}

	if (pass == Pass::Opaque && model->m_show_skeleton)
	{
		glm::mat4 mat_proj = p_camera->projectionMatrix;
		glm::mat4 mat_camera = p_camera->matrixWorldInverse;
		glm::mat4 mat_model = model->matrixWorld;
		glm::mat4 model_view = mat_camera * mat_model;
		glMatrixLoadfEXT(GL_PROJECTION, (float*)&mat_proj);
		glMatrixLoadfEXT(GL_MODELVIEW, (float*)&model_view);

		glDisable(GL_DEPTH_TEST);
		glColor3f(0.0f, 1.0f, 0.0f);
		glBegin(GL_LINES);

		int num_nodes = (int)model->m_nodes.size();
		for (int i = 0; i < num_nodes; i++)
		{
			Node& node1 = model->m_nodes[i];	
			glm::vec3 p1 = node1.g_trans[3];

			int num_children = node1.children.size();
			for (int j = 0; j < num_children; j++)
			{
				Node& node2 = model->m_nodes[node1.children[j]];
				glm::vec3 p2 = node2.g_trans[3];

				if (p1 != p2)
				{
					glVertex3fv((float*)&p1);
					glVertex3fv((float*)&p2);
				}
			}

		}

		glEnd();
	}
}

void GLRenderer::render_model(Camera* p_camera, const Lights& lights, const Fog* fog, VolumeIsosurfaceModel* model, GLRenderTarget& target, Pass pass)
{
	DrawIsosurface::Options options;
	options.msaa = target.msaa();
	options.num_directional_lights = lights.num_directional_lights;
	options.num_directional_shadows = lights.num_directional_shadows;
	options.has_reflection_map = lights.reflection_map != nullptr;
	options.has_environment_map = lights.environment_map != nullptr;
	options.has_probe_grid = lights.probe_grid != nullptr;
	if (options.has_probe_grid)
	{
		options.probe_reference_recorded = lights.probe_grid->record_references;
	}
	options.has_lod_probe_grid = lights.lod_probe_grid != nullptr;
	options.has_ambient_light = lights.ambient_light != nullptr;
	options.has_hemisphere_light = lights.hemisphere_light != nullptr;
	options.has_fog = fog != nullptr;	

	DrawIsosurface* draw = get_isosurface_draw(options);

	DrawIsosurface::RenderParams params;
	params.camera = p_camera;
	params.model = model;	
	params.tex_depth = target.m_tex_depth.get();
	params.lights = &lights;

	if (fog != nullptr)
	{
		params.constant_fog = &fog->m_constant;
	}
	else
	{
		params.constant_fog = nullptr;
	}
	draw->render(params);
}


void GLRenderer::render_widget(Camera* p_camera, DirectionalLight* light)
{
	if (light->shadow != nullptr)
	{
		DirectionalLightShadow* shadow = light->shadow.get();

		glm::mat4 mat_proj = p_camera->projectionMatrix;
		glm::mat4 mat_camera = p_camera->matrixWorldInverse;
		glm::mat4 mat_model = light->matrixWorld;
		glm::mat4 model_view = mat_camera * mat_model;
		glMatrixLoadfEXT(GL_PROJECTION, (float*)&mat_proj);
		glMatrixLoadfEXT(GL_MODELVIEW, (float*)&model_view);

		glEnable(GL_DEPTH_TEST);

		glLineWidth(2.0f);

		glBegin(GL_LINES);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(shadow->m_left, 0.0, 0.0f);
		glVertex3f(shadow->m_right, 0.0, 0.0f);

		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f(0.0, shadow->m_bottom, 0.0f);
		glVertex3f(0.0, shadow->m_top, 0.0f);

		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3f(shadow->m_left, shadow->m_bottom, -shadow->m_near);
		glVertex3f(shadow->m_left, shadow->m_bottom, -shadow->m_far);
		glVertex3f(shadow->m_left, shadow->m_top, -shadow->m_near);
		glVertex3f(shadow->m_left, shadow->m_top, -shadow->m_far);
		glVertex3f(shadow->m_right, shadow->m_bottom, -shadow->m_near);
		glVertex3f(shadow->m_right, shadow->m_bottom, -shadow->m_far);
		glVertex3f(shadow->m_right, shadow->m_top, -shadow->m_near);
		glVertex3f(shadow->m_right, shadow->m_top, -shadow->m_far);

		glVertex3f(shadow->m_left, shadow->m_bottom, -shadow->m_near);
		glVertex3f(shadow->m_right, shadow->m_bottom, -shadow->m_near);
		glVertex3f(shadow->m_left, shadow->m_top, -shadow->m_near);
		glVertex3f(shadow->m_right, shadow->m_top, -shadow->m_near);
		glVertex3f(shadow->m_left, shadow->m_top, -shadow->m_near);
		glVertex3f(shadow->m_left, shadow->m_bottom, -shadow->m_near);
		glVertex3f(shadow->m_right, shadow->m_top, -shadow->m_near);
		glVertex3f(shadow->m_right, shadow->m_bottom, -shadow->m_near);

		glVertex3f(shadow->m_left, shadow->m_bottom, -shadow->m_far);
		glVertex3f(shadow->m_right, shadow->m_bottom, -shadow->m_far);
		glVertex3f(shadow->m_left, shadow->m_top, -shadow->m_far);
		glVertex3f(shadow->m_right, shadow->m_top, -shadow->m_far);
		glVertex3f(shadow->m_left, shadow->m_top, -shadow->m_far);
		glVertex3f(shadow->m_left, shadow->m_bottom, -shadow->m_far);
		glVertex3f(shadow->m_right, shadow->m_top, -shadow->m_far);
		glVertex3f(shadow->m_right, shadow->m_bottom, -shadow->m_far);

		glEnd();

	}
}

inline void draw_round(glm::vec3 center, float radius, Camera* p_camera)
{
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(center.x, center.y, center.z);

	glm::vec3 axis_x = p_camera->matrixWorld * glm::vec4(radius, 0.0f, 0.0f, 0.0f);
	glm::vec3 axis_y = p_camera->matrixWorld * glm::vec4(0.0f, radius, 0.0f, 0.0f);

	int divs = 32;
	float delta = 3.14159265f * 2.0f / (float)divs;
	for (int i = 0; i <= divs; i++)
	{
		float rad = delta * (float)i;
		glm::vec3 pos = center + axis_x * cosf(rad) + axis_y * sinf(rad);
		glVertex3f(pos.x, pos.y, pos.z);
	}

	glEnd();
}

void GLRenderer::render_widget(Camera* p_camera, ProbeGridWidget* widget)
{
	glm::mat4 mat_proj = p_camera->projectionMatrix;
	glm::mat4 mat_camera = p_camera->matrixWorldInverse;
	glm::mat4 mat_model = widget->matrixWorld;
	glm::mat4 model_view = mat_camera * mat_model;
	glMatrixLoadfEXT(GL_PROJECTION, (float*)&mat_proj);
	glMatrixLoadfEXT(GL_MODELVIEW, (float*)&model_view);

	glEnable(GL_DEPTH_TEST);

	glLineWidth(1.0f);

	glBegin(GL_LINES);

	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(widget->coverage_min.x, widget->coverage_min.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_min.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_min.x, widget->coverage_max.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_max.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_min.x, widget->coverage_min.y, widget->coverage_max.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_min.y, widget->coverage_max.z);
	glVertex3f(widget->coverage_min.x, widget->coverage_max.y, widget->coverage_max.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_max.y, widget->coverage_max.z);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(widget->coverage_min.x, widget->coverage_min.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_min.x, widget->coverage_max.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_min.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_max.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_min.x, widget->coverage_min.y, widget->coverage_max.z);
	glVertex3f(widget->coverage_min.x, widget->coverage_max.y, widget->coverage_max.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_min.y, widget->coverage_max.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_max.y, widget->coverage_max.z);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(widget->coverage_min.x, widget->coverage_min.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_min.x, widget->coverage_min.y, widget->coverage_max.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_min.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_min.y, widget->coverage_max.z);
	glVertex3f(widget->coverage_min.x, widget->coverage_max.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_min.x, widget->coverage_max.y, widget->coverage_max.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_max.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_max.y, widget->coverage_max.z);

	glEnd();

	glColor3f(1.0f, 0.2f, 1.0f);

	glm::vec3 size = widget->coverage_max - widget->coverage_min;
	for (int z = 0; z < widget->divisions.z; z++)
	{
		for (int y = 0; y < widget->divisions.y; y++)
		{
			for (int x = 0; x < widget->divisions.x; x++)
			{
				glm::vec3 pos_normalized = (glm::vec3(x, y, z) + glm::vec3(0.5)) / glm::vec3(widget->divisions);
				pos_normalized.y = powf(pos_normalized.y, widget->ypower);
				glm::vec3 center = widget->coverage_min + size * pos_normalized;
				draw_round(center, 0.05, p_camera);
			}
		}

	}


}

void GLRenderer::render_widget(Camera* p_camera, LODProbeGridWidget* widget)
{
	glm::mat4 mat_proj = p_camera->projectionMatrix;
	glm::mat4 mat_camera = p_camera->matrixWorldInverse;
	glm::mat4 mat_model = widget->matrixWorld;
	glm::mat4 model_view = mat_camera * mat_model;
	glMatrixLoadfEXT(GL_PROJECTION, (float*)&mat_proj);
	glMatrixLoadfEXT(GL_MODELVIEW, (float*)&model_view);

	glEnable(GL_DEPTH_TEST);

	glLineWidth(1.0f);

	glBegin(GL_LINES);

	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(widget->coverage_min.x, widget->coverage_min.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_min.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_min.x, widget->coverage_max.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_max.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_min.x, widget->coverage_min.y, widget->coverage_max.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_min.y, widget->coverage_max.z);
	glVertex3f(widget->coverage_min.x, widget->coverage_max.y, widget->coverage_max.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_max.y, widget->coverage_max.z);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(widget->coverage_min.x, widget->coverage_min.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_min.x, widget->coverage_max.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_min.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_max.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_min.x, widget->coverage_min.y, widget->coverage_max.z);
	glVertex3f(widget->coverage_min.x, widget->coverage_max.y, widget->coverage_max.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_min.y, widget->coverage_max.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_max.y, widget->coverage_max.z);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(widget->coverage_min.x, widget->coverage_min.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_min.x, widget->coverage_min.y, widget->coverage_max.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_min.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_min.y, widget->coverage_max.z);
	glVertex3f(widget->coverage_min.x, widget->coverage_max.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_min.x, widget->coverage_max.y, widget->coverage_max.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_max.y, widget->coverage_min.z);
	glVertex3f(widget->coverage_max.x, widget->coverage_max.y, widget->coverage_max.z);

	glEnd();	

	if (widget->probe_grid == nullptr) return;

	glColor3f(1.0f, 0.2f, 1.0f);

	size_t num_probes = widget->probe_grid->getNumberOfProbes();
	for (size_t i = 0; i < num_probes; i++)
	{
		glm::vec4 pos_level = widget->probe_grid->m_probe_data[i * 10];
		glm::vec3 pos = glm::vec3(pos_level);
		float level = pos_level.w;
		draw_round(pos, 0.05 * powf(0.5f, level), p_camera);
	}

}

void GLRenderer::render_primitive_simple(const SimpleRoutine::RenderParams& params, Pass pass)
{
	const MeshStandardMaterial* material = params.material_list[params.primitive->material_idx];
	const Lights* lights = params.lights;

	SimpleRoutine::Options options;
	options.alpha_mode = material->alphaMode;
	options.is_highlight_pass = pass == Pass::Highlight;
	options.specular_glossiness = material->specular_glossiness;
	options.has_color = params.primitive->color_buf != nullptr;
	options.has_color_texture = material->tex_idx_map >= 0;
	options.has_metalness_map = material->tex_idx_metalnessMap >= 0;
	options.has_roughness_map = material->tex_idx_roughnessMap >= 0;		
	options.has_emissive_map = material->tex_idx_emissiveMap >= 0;
	options.has_specular_map = material->tex_idx_specularMap >= 0;
	options.has_glossiness_map = material->tex_idx_glossinessMap >= 0;
	options.num_directional_lights = lights->num_directional_lights;
	options.num_directional_shadows = lights->num_directional_shadows;	
	options.has_lightmap = params.tex_lightmap != nullptr;
	if (!options.has_lightmap)
	{
		options.has_environment_map = lights->environment_map != nullptr;
		options.has_probe_grid = lights->probe_grid != nullptr;
		if (options.has_probe_grid)
		{
			if (lights->probe_grid->per_primitive)
			{
				options.has_probe_grid = false;
				options.has_environment_map = true;
			}
			else
			{
				options.probe_reference_recorded = lights->probe_grid->record_references;
			}
		}
		options.has_lod_probe_grid = lights->lod_probe_grid != nullptr;
		if (options.has_lod_probe_grid)
		{
			if (lights->lod_probe_grid->per_primitive)
			{
				options.has_lod_probe_grid = false;
				options.has_environment_map = true;
			}
		}
		options.has_ambient_light = lights->ambient_light != nullptr;
		options.has_hemisphere_light = lights->hemisphere_light != nullptr;
	}
	options.has_fog = params.constant_fog != nullptr;		
	SimpleRoutine* routine = get_simple_routine(options);
	routine->render(params);
}

void GLRenderer::render_primitives_simple(const SimpleRoutine::RenderParams& params, Pass pass, const std::vector<void*>& offset_lst, const std::vector<int>& count_lst)
{
	const MeshStandardMaterial* material = params.material_list[params.primitive->material_idx];
	const Lights* lights = params.lights;

	SimpleRoutine::Options options;
	options.alpha_mode = material->alphaMode;
	options.is_highlight_pass = pass == Pass::Highlight;
	options.specular_glossiness = material->specular_glossiness;
	options.has_color = params.primitive->color_buf != nullptr;
	options.has_color_texture = material->tex_idx_map >= 0;
	options.has_metalness_map = material->tex_idx_metalnessMap >= 0;
	options.has_roughness_map = material->tex_idx_roughnessMap >= 0;
	options.has_emissive_map = material->tex_idx_emissiveMap >= 0;
	options.has_specular_map = material->tex_idx_specularMap >= 0;
	options.has_glossiness_map = material->tex_idx_glossinessMap >= 0;
	options.num_directional_lights = lights->num_directional_lights;
	options.num_directional_shadows = lights->num_directional_shadows;
	options.has_lightmap = params.tex_lightmap != nullptr;
	if (!options.has_lightmap)
	{
		options.has_environment_map = lights->environment_map != nullptr;
		options.has_probe_grid = lights->probe_grid != nullptr;
		if (options.has_probe_grid)
		{
			options.probe_reference_recorded = lights->probe_grid->record_references;
		}
		options.has_lod_probe_grid = lights->lod_probe_grid != nullptr;
		options.has_ambient_light = lights->ambient_light != nullptr;
		options.has_hemisphere_light = lights->hemisphere_light != nullptr;
	}
	options.has_fog = params.constant_fog != nullptr;
	SimpleRoutine* routine = get_simple_routine(options);
	routine->render_batched(params, offset_lst, count_lst);
}

void GLRenderer::render_model_simple(Camera* p_camera, const Lights& lights, const Fog* fog, SimpleModel* model, Pass pass)
{
	const GLTexture2D* tex = &model->texture;
	if (model->repl_texture != nullptr)
	{
		tex = model->repl_texture;
	}

	const MeshStandardMaterial* material = &model->material;

	if (pass == Pass::Opaque)
	{
		if (material->alphaMode == AlphaMode::Blend) return;
	}
	else if (pass == Pass::Alpha || pass == Pass::Highlight)
	{
		if (material->alphaMode != AlphaMode::Blend) return;
	}

	SimpleRoutine::RenderParams params;
	params.tex_list = &tex;
	params.material_list = &material;
	params.constant_camera = &p_camera->m_constant;
	params.constant_model = &model->m_constant;
	params.primitive = &model->geometry;
	params.lights = &lights;
	params.tex_lightmap = nullptr;

	if (fog != nullptr)
	{
		params.constant_fog = &fog->m_constant;
	}
	else
	{
		params.constant_fog = nullptr;
	}

	render_primitive_simple(params, pass);
}

void GLRenderer::render_model_simple(Camera* p_camera, const Lights& lights, const Fog* fog, GLTFModel* model, Pass pass)
{
	std::vector<const GLTexture2D*> tex_lst(model->m_textures.size());
	for (size_t i = 0; i < tex_lst.size(); i++)
	{
		auto iter = model->m_repl_textures.find(i);
		if (iter != model->m_repl_textures.end())
		{
			tex_lst[i] = iter->second;
		}
		else
		{
			tex_lst[i] = model->m_textures[i].get();
		}
	}

	std::vector<const MeshStandardMaterial*> material_lst(model->m_materials.size());
	for (size_t i = 0; i < material_lst.size(); i++)
		material_lst[i] = model->m_materials[i].get();

	if (model->batched_mesh != nullptr)
	{
		std::vector<std::vector<void*>> offset_lists(material_lst.size());
		std::vector<std::vector<int>> count_lists(material_lst.size());
		for (size_t i = 0; i < model->m_meshs.size(); i++)
		{
			Mesh& mesh = model->m_meshs[i];
			glm::mat4 matrix = model->matrixWorld;
			if (mesh.node_id >= 0 && mesh.skin_id < 0)
			{
				Node& node = model->m_nodes[mesh.node_id];
				matrix *= node.g_trans;
			}
			glm::mat4 MV = p_camera->matrixWorldInverse * matrix;

			for (size_t j = 0; j < mesh.primitives.size(); j++)
			{
				Primitive& primitive = mesh.primitives[j];
				if (!visible(MV, p_camera->projectionMatrix, primitive.min_pos, primitive.max_pos)) continue;

				int idx_list = primitive.material_idx;
				offset_lists[idx_list].push_back((void*)(model->batch_map[i][j]));
				count_lists[idx_list].push_back(primitive.num_face * 3);
			}
		}

		{
			Mesh& mesh = *model->batched_mesh;
			for (size_t j = 0; j < mesh.primitives.size(); j++)
			{
				Primitive& primitive = mesh.primitives[j];
				int idx_material = primitive.material_idx;

				std::vector<void*>& material_offsets = offset_lists[idx_material];
				std::vector<int>& material_counts = count_lists[idx_material];
				if (material_offsets.size() < 1) continue;

				const MeshStandardMaterial* material = material_lst[primitive.material_idx];
				if (pass == Pass::Opaque)
				{
					if (material->alphaMode == AlphaMode::Blend) continue;
				}
				else if (pass == Pass::Alpha || pass == Pass::Highlight)
				{
					if (material->alphaMode != AlphaMode::Blend) continue;
				}

				SimpleRoutine::RenderParams params;
				params.tex_list = tex_lst.data();
				params.material_list = material_lst.data();
				params.constant_camera = &p_camera->m_constant;
				params.constant_model = mesh.model_constant.get();
				params.primitive = &primitive;
				params.lights = &lights;
				params.tex_lightmap = nullptr;
				if (model->lightmap != nullptr)
				{
					params.tex_lightmap = model->lightmap->lightmap.get();
				}

				if (fog != nullptr)
				{
					params.constant_fog = &fog->m_constant;
				}
				else
				{
					params.constant_fog = nullptr;
				}

				render_primitives_simple(params, pass, material_offsets, material_counts);
			}

		}
	}
	else
	{
		for (size_t i = 0; i < model->m_meshs.size(); i++)
		{
			Mesh& mesh = model->m_meshs[i];
			glm::mat4 matrix = model->matrixWorld;
			if (mesh.node_id >= 0 && mesh.skin_id < 0)
			{
				Node& node = model->m_nodes[mesh.node_id];
				matrix *= node.g_trans;
			}
			glm::mat4 MV = p_camera->matrixWorldInverse * matrix;

			for (size_t j = 0; j < mesh.primitives.size(); j++)
			{
				Primitive& primitive = mesh.primitives[j];
				if (!visible(MV, p_camera->projectionMatrix, primitive.min_pos, primitive.max_pos)) continue;

				const MeshStandardMaterial* material = material_lst[primitive.material_idx];
				if (pass == Pass::Opaque)
				{
					if (material->alphaMode == AlphaMode::Blend) continue;
				}
				else if (pass == Pass::Alpha || pass == Pass::Highlight)
				{
					if (material->alphaMode != AlphaMode::Blend) continue;
				}

				SimpleRoutine::RenderParams params;
				params.tex_list = tex_lst.data();
				params.material_list = material_lst.data();
				params.constant_camera = &p_camera->m_constant;
				params.constant_model = mesh.model_constant.get();
				params.primitive = &primitive;
				params.lights = &lights;
				params.tex_lightmap = nullptr;
				if (model->lightmap != nullptr)
				{
					params.tex_lightmap = model->lightmap->lightmap.get();
				}

				if (fog != nullptr)
				{
					params.constant_fog = &fog->m_constant;
				}
				else
				{
					params.constant_fog = nullptr;
				}
				render_primitive_simple(params, pass);
			}
		}

	}
}



DirectionalShadowCast* GLRenderer::get_shadow_caster(const DirectionalShadowCast::Options& options)
{
	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(DirectionalShadowCast::Options));
	auto iter = directional_shadow_caster_map.find(hash);
	if (iter == directional_shadow_caster_map.end())
	{
		directional_shadow_caster_map[hash] = std::unique_ptr<DirectionalShadowCast>(new DirectionalShadowCast(options));
	}
	return directional_shadow_caster_map[hash].get();
}

void GLRenderer::render_shadow_primitive(const DirectionalShadowCast::RenderParams& params)
{		
	const MeshStandardMaterial* material = params.material_list[params.primitive->material_idx];

	DirectionalShadowCast::Options options;
	options.alpha_mode = material->alphaMode;
	options.has_color = params.primitive->color_buf != nullptr;
	options.has_color_texture = material->tex_idx_map >= 0;
	DirectionalShadowCast* shadow_caster = get_shadow_caster(options);
	shadow_caster->render(params);
}

void GLRenderer::render_shadow_primitives(const DirectionalShadowCast::RenderParams& params, const std::vector<void*>& offset_lst, const std::vector<int>& count_lst)
{
	const MeshStandardMaterial* material = params.material_list[params.primitive->material_idx];

	DirectionalShadowCast::Options options;
	options.alpha_mode = material->alphaMode;
	options.has_color = params.primitive->color_buf != nullptr;
	options.has_color_texture = material->tex_idx_map >= 0;
	DirectionalShadowCast* shadow_caster = get_shadow_caster(options);
	shadow_caster->render_batched(params, offset_lst, count_lst);
}

void GLRenderer::render_shadow_model(DirectionalLightShadow* shadow, SimpleModel* model)
{
	glm::mat4 view_matrix = glm::inverse(shadow->m_light->matrixWorld);
	if (!visible(view_matrix * model->matrixWorld, shadow->m_light_proj_matrix, model->geometry.min_pos, model->geometry.max_pos)) return;

	const GLTexture2D* tex = &model->texture;
	if (model->repl_texture != nullptr)
	{
		tex = model->repl_texture;
	}

	const MeshStandardMaterial* material = &model->material;

	DirectionalShadowCast::RenderParams params;
	params.force_cull = shadow->m_force_cull;
	params.tex_list = &tex;
	params.material_list = &material;
	params.constant_shadow = &shadow->constant_shadow;
	params.constant_model = &model->m_constant;
	params.primitive = &model->geometry;
	render_shadow_primitive(params);
}

void GLRenderer::render_shadow_model(DirectionalLightShadow* shadow, GLTFModel* model)
{
	glm::mat4 view_matrix = glm::inverse(shadow->m_light->matrixWorld);
	
	std::vector<const GLTexture2D*> tex_lst(model->m_textures.size());
	for (size_t i = 0; i < tex_lst.size(); i++)
	{
		auto iter = model->m_repl_textures.find(i);
		if (iter != model->m_repl_textures.end())
		{
			tex_lst[i] = iter->second;
		}
		else
		{
			tex_lst[i] = model->m_textures[i].get();
		}
	}	

	std::vector<const MeshStandardMaterial*> material_lst(model->m_materials.size());
	for (size_t i = 0; i < material_lst.size(); i++)
		material_lst[i] = model->m_materials[i].get();

	if (model->batched_mesh != nullptr)
	{
		std::vector<std::vector<void*>> offset_lists(material_lst.size());
		std::vector<std::vector<int>> count_lists(material_lst.size());
		for (size_t i = 0; i < model->m_meshs.size(); i++)
		{
			Mesh& mesh = model->m_meshs[i];
			glm::mat4 matrix = model->matrixWorld;
			if (mesh.node_id >= 0 && mesh.skin_id < 0)
			{
				Node& node = model->m_nodes[mesh.node_id];
				matrix *= node.g_trans;
			}
			glm::mat4 MV = view_matrix * matrix;

			for (size_t j = 0; j < mesh.primitives.size(); j++)
			{
				Primitive& primitive = mesh.primitives[j];
				if (!visible(MV, shadow->m_light_proj_matrix, primitive.min_pos, primitive.max_pos)) continue;				

				int idx_list = primitive.material_idx;
				offset_lists[idx_list].push_back((void*)(model->batch_map[i][j]));
				count_lists[idx_list].push_back(primitive.num_face * 3);
			}
		}

		{
			Mesh& mesh = *model->batched_mesh;
			for (size_t j = 0; j < mesh.primitives.size(); j++)
			{
				Primitive& primitive = mesh.primitives[j];
				int idx_material = primitive.material_idx;

				std::vector<void*>& material_offsets = offset_lists[idx_material];
				std::vector<int>& material_counts = count_lists[idx_material];
				if (material_offsets.size() < 1) continue;

				const MeshStandardMaterial* material = material_lst[primitive.material_idx];

				DirectionalShadowCast::RenderParams params;
				params.force_cull = shadow->m_force_cull;
				params.tex_list = tex_lst.data();
				params.material_list = material_lst.data();
				params.constant_shadow = &shadow->constant_shadow;
				params.constant_model = mesh.model_constant.get();
				params.primitive = &primitive;
				render_shadow_primitives(params, material_offsets, material_counts);
			}
		}

	}
	else
	{
		for (size_t i = 0; i < model->m_meshs.size(); i++)
		{
			Mesh& mesh = model->m_meshs[i];
			glm::mat4 matrix = model->matrixWorld;
			if (mesh.node_id >= 0 && mesh.skin_id < 0)
			{
				Node& node = model->m_nodes[mesh.node_id];
				matrix *= node.g_trans;
			}
			glm::mat4 MV = view_matrix * matrix;

			for (size_t j = 0; j < mesh.primitives.size(); j++)
			{
				Primitive& primitive = mesh.primitives[j];
				if (!visible(MV, shadow->m_light_proj_matrix, primitive.min_pos, primitive.max_pos)) continue;

				const MeshStandardMaterial* material = material_lst[primitive.material_idx];

				DirectionalShadowCast::RenderParams params;
				params.force_cull = shadow->m_force_cull;
				params.tex_list = tex_lst.data();
				params.material_list = material_lst.data();
				params.constant_shadow = &shadow->constant_shadow;
				params.constant_model = mesh.model_constant.get();
				params.primitive = &primitive;
				render_shadow_primitive(params);
			}
		}
	}
}

void GLRenderer::render_shadow_model(DirectionalLightShadow* shadow, VolumeIsosurfaceModel* model)
{
	if (isosurface_directional_shadow == nullptr)
	{
		isosurface_directional_shadow = std::unique_ptr<IsosurfaceDirectionalShadow>(new IsosurfaceDirectionalShadow);
	}
	IsosurfaceDirectionalShadow* shadow_caster = isosurface_directional_shadow.get();

	IsosurfaceDirectionalShadow::RenderParams params;
	params.model = model;
	params.shadow = shadow;
	shadow_caster->render(params);
}

Picking* GLRenderer::get_picking(const Picking::Options& options)
{
	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(Picking::Options));
	auto iter = picking_map.find(hash);
	if (iter == picking_map.end())
	{
		picking_map[hash] = std::unique_ptr<Picking>(new Picking(options));
	}
	return picking_map[hash].get();
}

void GLRenderer::render_picking_primitive(const Picking::RenderParams& params)
{
	const MeshStandardMaterial* material = params.material_list[params.primitive->material_idx];

	Picking::Options options;
	options.alpha_mode = material->alphaMode;
	options.has_color = params.primitive->color_buf != nullptr;
	options.has_color_texture = material->tex_idx_map >= 0;
	Picking* picking_renderer = get_picking(options);
	picking_renderer->render(params);
}

void GLRenderer::render_picking_model(Camera* p_camera, SimpleModel* model, GLPickingTarget& target)
{
	glm::mat4 view_matrix = p_camera->matrixWorldInverse;
	if (!visible(view_matrix * model->matrixWorld, p_camera->projectionMatrix, model->geometry.min_pos, model->geometry.max_pos)) return;

	const GLTexture2D* tex = &model->texture;
	if (model->repl_texture != nullptr)
	{
		tex = model->repl_texture;
	}

	const MeshStandardMaterial* material = &model->material;

	GLPickingTarget::IdxInfo idx_info;
	idx_info.obj = model;
	idx_info.primitive_idx = 0;
	int idx = (int)target.m_idx_info.size();
	target.m_idx_info.push_back(idx_info);

	Picking::RenderParams params;
	params.tex_list = &tex;
	params.material_list = &material;
	params.constant_camera = &p_camera->m_constant;
	params.constant_model = &model->m_constant;
	params.primitive = &model->geometry;
	params.idx = idx;
	render_picking_primitive(params);
}

void GLRenderer::render_picking_model(Camera* p_camera, GLTFModel* model, GLPickingTarget& target)
{
	glm::mat4 view_matrix = p_camera->matrixWorldInverse;

	std::vector<const GLTexture2D*> tex_lst(model->m_textures.size());
	for (size_t i = 0; i < tex_lst.size(); i++)
	{
		auto iter = model->m_repl_textures.find(i);
		if (iter != model->m_repl_textures.end())
		{
			tex_lst[i] = iter->second;
		}
		else
		{
			tex_lst[i] = model->m_textures[i].get();
		}
	}

	std::vector<const MeshStandardMaterial*> material_lst(model->m_materials.size());
	for (size_t i = 0; i < material_lst.size(); i++)
		material_lst[i] = model->m_materials[i].get();

	int primitive_idx = 0;
	for (size_t i = 0; i < model->m_meshs.size(); i++)
	{
		Mesh& mesh = model->m_meshs[i];
		glm::mat4 matrix = model->matrixWorld;
		if (mesh.node_id >= 0 && mesh.skin_id < 0)
		{
			Node& node = model->m_nodes[mesh.node_id];
			matrix *= node.g_trans;
		}
		glm::mat4 MV = view_matrix * matrix;

		for (size_t j = 0; j < mesh.primitives.size(); j++, primitive_idx++)
		{
			Primitive& primitive = mesh.primitives[j];
			if (!visible(MV, p_camera->projectionMatrix, primitive.min_pos, primitive.max_pos)) continue;

			const MeshStandardMaterial* material = material_lst[primitive.material_idx];

			GLPickingTarget::IdxInfo idx_info;
			idx_info.obj = model;
			idx_info.primitive_idx = primitive_idx;
			int idx = (int)target.m_idx_info.size();
			target.m_idx_info.push_back(idx_info);

			Picking::RenderParams params;
			params.tex_list = tex_lst.data();
			params.material_list = material_lst.data();
			params.constant_camera = &p_camera->m_constant;
			params.constant_model = mesh.model_constant.get();
			params.primitive = &primitive;
			params.idx = idx;
			render_picking_primitive(params);
		}
	}
}

void GLRenderer::_render_fog(const Camera& camera, const Lights& lights, const Fog& fog, GLRenderTarget& target)
{
	DrawFog::Options options;
	options.msaa = target.msaa();
	options.has_ambient_light = lights.ambient_light != nullptr;
	options.has_hemisphere_light = lights.hemisphere_light != nullptr;
	options.has_environment_map = lights.environment_map != nullptr;

	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(DrawFog::Options));
	auto iter = fog_draw_map.find(hash);
	if (iter == fog_draw_map.end())
	{
		fog_draw_map[hash] = std::unique_ptr<DrawFog>(new DrawFog(options));
	}
	DrawFog* fog_draw = fog_draw_map[hash].get();

	DrawFog::RenderParams params;
	params.tex_depth = target.m_tex_depth.get();
	params.constant_camera = &camera.m_constant;
	params.constant_fog = &fog.m_constant;
	params.lights = &lights;

	fog_draw->render(params);
}

void GLRenderer::_render_fog_rm(const Camera& camera, DirectionalLight& light, const Fog& fog, GLRenderTarget& target)
{
	bool msaa = target.msaa();
	int idx = msaa ? 1 : 0;
	if (fog_ray_march[idx] == nullptr)
	{
		fog_ray_march[idx] = std::unique_ptr<FogRayMarching>(new FogRayMarching(msaa));
	}
	FogRayMarching* fog_rm = fog_ray_march[idx].get();

	light.updateConstant();
	FogRayMarching::RenderParams params;
	params.tex_depth = target.m_tex_depth.get();
	params.camera = &camera;
	params.fog = &fog;
	params.constant_diretional_light = &light.m_constant;
	if (light.shadow != nullptr)
	{
		params.constant_diretional_shadow = &light.shadow->constant_shadow;
		params.tex_shadow = light.shadow->m_lightTex;
	}
	else
	{
		params.constant_diretional_shadow = nullptr;
		params.tex_shadow = -1;
	}
	fog_rm->render(params);
}

void GLRenderer::_render_fog_rm_env(const Camera& camera, const Lights& lights, const Fog& fog, GLRenderTarget& target)
{
	FogRayMarchingEnv::Options options;
	options.msaa = target.msaa();
	options.has_probe_grid = lights.probe_grid != nullptr;
	if (options.has_probe_grid)
	{
		options.probe_reference_recorded = lights.probe_grid->record_references;
	}
	options.has_lod_probe_grid = lights.lod_probe_grid != nullptr;

	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(FogRayMarchingEnv::Options));
	auto iter = fog_ray_march_map.find(hash);
	if (iter == fog_ray_march_map.end())
	{
		fog_ray_march_map[hash] = std::unique_ptr<FogRayMarchingEnv>(new FogRayMarchingEnv(options));
	}
	FogRayMarchingEnv* fog_draw = fog_ray_march_map[hash].get();

	FogRayMarchingEnv::RenderParams params;
	params.tex_depth = target.m_tex_depth.get();
	params.constant_camera = &camera.m_constant;
	params.constant_fog = &fog.m_constant;
	params.lights = &lights;

	fog_draw->render(params);

}


void GLRenderer::_render_fog_rm_simple(const Camera& camera, DirectionalLight& light, const Fog& fog, GLRenderTarget& target)
{
	bool msaa = target.msaa();
	int idx = msaa ? 1 : 0;
	if (fog_ray_march_simple[idx] == nullptr)
	{
		fog_ray_march_simple[idx] = std::unique_ptr<SimpleFogRayMarching>(new SimpleFogRayMarching(msaa));
	}
	SimpleFogRayMarching* fog_rm = fog_ray_march_simple[idx].get();

	light.updateConstant();
	SimpleFogRayMarching::RenderParams params;
	params.tex_depth = target.m_tex_depth.get();
	params.camera = &camera;
	params.fog = &fog;
	params.constant_diretional_light = &light.m_constant;
	if (light.shadow != nullptr)
	{
		params.constant_diretional_shadow = &light.shadow->constant_shadow;
		params.tex_shadow = light.shadow->m_lightTex;
	}
	else
	{
		params.constant_diretional_shadow = nullptr;
		params.tex_shadow = -1;
	}
	fog_rm->render(params);
}

void GLRenderer::_render_fog_rm_env_simple(const Camera& camera, const Lights& lights, const Fog& fog, GLRenderTarget& target)
{
	SimpleFogRayMarchingEnv::Options options;
	options.msaa = target.msaa();
	options.has_probe_grid = lights.probe_grid != nullptr;
	if (options.has_probe_grid)
	{
		options.probe_reference_recorded = lights.probe_grid->record_references;
	}
	options.has_lod_probe_grid = lights.lod_probe_grid != nullptr;

	uint64_t hash = crc64(0, (const unsigned char*)&options, sizeof(SimpleFogRayMarchingEnv::Options));
	auto iter = fog_ray_march_map_simple.find(hash);
	if (iter == fog_ray_march_map_simple.end())
	{
		fog_ray_march_map_simple[hash] = std::unique_ptr<SimpleFogRayMarchingEnv>(new SimpleFogRayMarchingEnv(options));
	}
	SimpleFogRayMarchingEnv* fog_draw = fog_ray_march_map_simple[hash].get();

	SimpleFogRayMarchingEnv::RenderParams params;
	params.tex_depth = target.m_tex_depth.get();
	params.constant_camera = &camera.m_constant;
	params.constant_fog = &fog.m_constant;
	params.lights = &lights;

	fog_draw->render(params);

}

void GLRenderer::_pre_render(Scene& scene)
{
	scene.clear_lists();

	auto* p_scene = &scene;
	scene.traverse([p_scene](Object3D* obj) {
		do
		{
			{
				SimpleModel* model = dynamic_cast<SimpleModel*>(obj);
				if (model)
				{
					p_scene->simple_models.push_back(model);
					break;
				}
			}
			{
				GLTFModel* model = dynamic_cast<GLTFModel*>(obj);
				if (model)
				{
					p_scene->gltf_models.push_back(model);
					break;
				}
			}
			{
				VolumeIsosurfaceModel* model = dynamic_cast<VolumeIsosurfaceModel*>(obj);
				if (model)
				{
					p_scene->volume_isosurface_models.push_back(model);
					break;
				}
			}
			{
				DirectionalLight* light = dynamic_cast<DirectionalLight*>(obj);
				if (light)
				{
					light->lookAtTarget();
					p_scene->directional_lights.push_back(light);
					break;
				}
			}
		} while (false);

		obj->updateWorldMatrix(false, false);
	});

	// update models
	for (size_t i = 0; i < scene.simple_models.size(); i++)
	{
		SimpleModel* model = scene.simple_models[i];
		update_model(model);
	}

	for (size_t i = 0; i < scene.gltf_models.size(); i++)
	{
		GLTFModel* model = scene.gltf_models[i];
		update_model(model);
	}

	for (size_t i = 0; i < scene.volume_isosurface_models.size(); i++)
	{
		VolumeIsosurfaceModel* model = scene.volume_isosurface_models[i];
		update_model(model);
	}

	// pre-render background-scene
	if (scene.background != nullptr)
	{
		BackgroundScene* bg = dynamic_cast<BackgroundScene*>(scene.background);
		if (bg != nullptr && bg->scene!=nullptr)
		{
			_pre_render(*bg->scene);
		}
	}

	// update lights
	bool update_building = false;
	std::unordered_set<int> cur_set;
	for (size_t j = 0; j < scene.simple_models.size(); j++)
	{
		SimpleModel* model = scene.simple_models[j];
		if (model->is_building)
		{			
			int id = model->id;
			cur_set.insert(id);
			auto iter = scene.building_set.find(id);
			if (iter == scene.building_set.end())
			{
				update_building = true;
				scene.building_set.insert(id);
			}
			if (model->moved)
			{
				update_building = true;
				model->moved = false;
			}
		}
	}

	for (size_t j = 0; j < scene.gltf_models.size(); j++)
	{
		GLTFModel* model = scene.gltf_models[j];
		if (model->is_building)
		{
			int id = model->id;
			cur_set.insert(id);
			auto iter = scene.building_set.find(id);
			if (iter == scene.building_set.end())
			{
				update_building = true;
				scene.building_set.insert(id);
			}
			if (model->moved)
			{
				update_building = true;
				model->moved = false;
			}
		}
	}

	for (size_t j = 0; j < scene.volume_isosurface_models.size(); j++)
	{
		VolumeIsosurfaceModel* model = scene.volume_isosurface_models[j];
		if (model->is_building)
		{
			int id = model->id;
			cur_set.insert(id);
			auto iter = scene.building_set.find(id);
			if (iter == scene.building_set.end())
			{
				update_building = true;
				scene.building_set.insert(id);
			}
			if (model->moved)
			{
				update_building = true;
				model->moved = false;
			}
		}
	}

	if (cur_set.size() < scene.building_set.size())
	{
		update_building = true;
		scene.building_set = cur_set;
	}

	for (size_t i = 0; i < scene.directional_lights.size(); i++)
	{
		DirectionalLight* light = scene.directional_lights[i];
		if (light->shadow != nullptr)
		{
			light->shadow->updateMatrices();

			bool update_building_this = update_building;
			if (light->moved)
			{
				update_building_this = true;
				light->moved = false;
			}

			if (update_building_this)
			{
				light->shadow->update_building_map(light->shadow->m_map_width, light->shadow->m_map_height);
				glBindFramebuffer(GL_FRAMEBUFFER, light->shadow->m_lightFBO_building);
				glViewport(0, 0, light->shadow->m_building_map_width, light->shadow->m_building_map_height);
				const float one = 1.0f;
				glDepthMask(GL_TRUE);
				glClearBufferfv(GL_DEPTH, 0, &one);

				for (size_t j = 0; j < scene.simple_models.size(); j++)
				{
					SimpleModel* model = scene.simple_models[j];
					if (model->is_building)
					{
						render_shadow_model(light->shadow.get(), model);
					}
				}

				for (size_t j = 0; j < scene.gltf_models.size(); j++)
				{
					GLTFModel* model = scene.gltf_models[j];
					if (model->is_building)
					{
						render_shadow_model(light->shadow.get(), model);
					}
				}


				for (size_t j = 0; j < scene.volume_isosurface_models.size(); j++)
				{
					VolumeIsosurfaceModel* model = scene.volume_isosurface_models[j];
					if (model->is_building)
					{
						render_shadow_model(light->shadow.get(), model);
					}
				}

			}

			glBindFramebuffer(GL_FRAMEBUFFER, light->shadow->m_lightFBO);
			glViewport(0, 0, light->shadow->m_map_width, light->shadow->m_map_height);
			
			if (light->shadow->m_lightTex_building == (unsigned)(-1))
			{
				const float one = 1.0f;
				glDepthMask(GL_TRUE);
				glClearBufferfv(GL_DEPTH, 0, &one);
			}
			else
			{
				if (copy_shadow == nullptr)
				{
					copy_shadow = std::unique_ptr<CopyDepth>(new CopyDepth);
				}
				copy_shadow->render(light->shadow->m_lightTex_building);
			}		

			for (size_t j = 0; j < scene.simple_models.size(); j++)
			{
				SimpleModel* model = scene.simple_models[j];
				if (!model->is_building)
				{
					render_shadow_model(light->shadow.get(), model);
				}
			}

			for (size_t j = 0; j < scene.gltf_models.size(); j++)
			{
				GLTFModel* model = scene.gltf_models[j];
				if (!model->is_building)
				{
					render_shadow_model(light->shadow.get(), model);
				}
			}


			for (size_t j = 0; j < scene.volume_isosurface_models.size(); j++)
			{
				VolumeIsosurfaceModel* model = scene.volume_isosurface_models[j];
				if (!model->is_building)
				{
					render_shadow_model(light->shadow.get(), model);
				}
			}
		}
	}


	// update light constants
	Lights& lights = scene.lights;
	lights.directional_shadow_texs.clear();

	std::vector<ConstDirectionalLight> const_directional_lights(scene.directional_lights.size());
	std::vector<ConstDirectionalShadow> const_directional_shadows;
	for (size_t i = 0; i < scene.directional_lights.size(); i++)
	{
		DirectionalLight* light = scene.directional_lights[i];
		ConstDirectionalLight& const_light = const_directional_lights[i];
		light->makeConst(const_light);

		if (light->shadow != nullptr)
		{
			lights.directional_shadow_texs.push_back(light->shadow->m_lightTex);
			ConstDirectionalShadow constShadow;
			light->shadow->makeConst(constShadow);
			const_directional_shadows.push_back(std::move(constShadow));
		}
	}

	{
		if (lights.num_directional_lights != (int)const_directional_lights.size())
		{
			lights.num_directional_lights = (int)const_directional_lights.size();
			lights.constant_directional_lights = nullptr;
			if (lights.num_directional_lights > 0)
			{
				lights.constant_directional_lights = std::unique_ptr<GLBuffer>(new GLBuffer(const_directional_lights.size() * sizeof(ConstDirectionalLight), GL_UNIFORM_BUFFER));
			}
			lights.hash_directional_lights = 0;

		}
		if (lights.num_directional_lights > 0)
		{
			uint64_t hash = crc64(0, (unsigned char*)const_directional_lights.data(), const_directional_lights.size() * sizeof(ConstDirectionalLight));
			if (hash != lights.hash_directional_lights)
			{
				lights.hash_directional_lights = hash;
				lights.constant_directional_lights->upload(const_directional_lights.data());
			}
		}
	}

	{
		if (lights.num_directional_shadows != (int)const_directional_shadows.size())
		{
			lights.num_directional_shadows = (int)const_directional_shadows.size();
			lights.constant_directional_shadows = nullptr;
			if (lights.num_directional_shadows > 0)
			{
				lights.constant_directional_shadows = std::unique_ptr<GLBuffer>(new GLBuffer(const_directional_shadows.size() * sizeof(ConstDirectionalShadow), GL_UNIFORM_BUFFER));
			}
			lights.hash_directional_shadows = 0;
		}
		if (lights.num_directional_shadows > 0)
		{
			uint64_t hash = crc64(0, (unsigned char*)const_directional_shadows.data(), const_directional_shadows.size() * sizeof(ConstDirectionalShadow));
			if (hash != lights.hash_directional_shadows)
			{
				lights.hash_directional_shadows = hash;
				lights.constant_directional_shadows->upload(const_directional_shadows.data());
			}
		}
	}

	lights.reflection_map = nullptr;
	lights.environment_map = nullptr;
	lights.probe_grid = nullptr;
	lights.lod_probe_grid = nullptr;
	lights.ambient_light = nullptr;
	lights.hemisphere_light = nullptr;

	while (scene.indirectLight)
	{
		lights.reflection_map = scene.indirectLight->reflection.get();
		{
			EnvironmentMap* envMap = dynamic_cast<EnvironmentMap*>(scene.indirectLight);
			if (envMap != nullptr)
			{
				envMap->updateConstant();
				lights.environment_map = envMap;
				break;
			}
		}
		{
			ProbeGrid* probeGrid = dynamic_cast<ProbeGrid*>(scene.indirectLight);
			if (probeGrid != nullptr)
			{
				probeGrid->updateConstant();
				lights.probe_grid = probeGrid;
				break;
			}

		}

		{
			LODProbeGrid* probeGrid = dynamic_cast<LODProbeGrid*>(scene.indirectLight);
			if (probeGrid != nullptr)
			{
				probeGrid->updateConstant();
				lights.lod_probe_grid = probeGrid;
				break;
			}

		}

		{
			AmbientLight* ambientLight = dynamic_cast<AmbientLight*>(scene.indirectLight);
			if (ambientLight != nullptr)
			{
				ambientLight->updateConstant();
				lights.ambient_light = ambientLight;
				break;
			}
		}

		{
			HemisphereLight* hemisphereLight = dynamic_cast<HemisphereLight*>(scene.indirectLight);
			if (hemisphereLight != nullptr)
			{
				hemisphereLight->updateConstant();
				lights.hemisphere_light = hemisphereLight;
				break;
			}
		}
		break;
	}

	if (lights.probe_grid != nullptr && lights.probe_grid->per_primitive)
	{
		for (size_t i_model = 0; i_model < scene.simple_models.size(); i_model++)
		{
			SimpleModel* model = scene.simple_models[i_model];
			Primitive* prim = &model->geometry;
			if (prim->envMap == nullptr)
			{
				prim->envMap = std::unique_ptr<EnvironmentMap>(new EnvironmentMap);
			}

			glm::vec3 position = (prim->min_pos + prim->max_pos) * 0.5f;
			position = glm::vec3(model->matrixWorld * glm::vec4(position, 1.0f));
			lights.probe_grid->get_probe(position, *prim->envMap);			
		}

		for (size_t i_model = 0; i_model < scene.gltf_models.size(); i_model++)
		{
			GLTFModel* model = scene.gltf_models[i_model];
			if (model->batched_mesh != nullptr) continue;

			for (size_t i = 0; i < model->m_meshs.size(); i++)
			{
				Mesh& mesh = model->m_meshs[i];
				glm::mat4 matrix = model->matrixWorld;
				if (mesh.node_id >= 0 && mesh.skin_id < 0)
				{
					Node& node = model->m_nodes[mesh.node_id];
					matrix *= node.g_trans;
				}

				for (size_t j = 0; j < mesh.primitives.size(); j++)
				{
					Primitive& primitive = mesh.primitives[j];
					if (primitive.envMap == nullptr)
					{
						primitive.envMap = std::unique_ptr<EnvironmentMap>(new EnvironmentMap);
					}
					glm::vec3 position = (primitive.min_pos + primitive.max_pos) * 0.5f;
					position = glm::vec3(matrix * glm::vec4(position, 1.0f));
					lights.probe_grid->get_probe(position, *primitive.envMap);

				}

			}

		}


	}

	if (lights.lod_probe_grid != nullptr && lights.lod_probe_grid->per_primitive)
	{
		for (size_t i_model = 0; i_model < scene.simple_models.size(); i_model++)
		{
			SimpleModel* model = scene.simple_models[i_model];
			Primitive* prim = &model->geometry;
			if (prim->envMap == nullptr)
			{
				prim->envMap = std::unique_ptr<EnvironmentMap>(new EnvironmentMap);
			}

			glm::vec3 position = (prim->min_pos + prim->max_pos) * 0.5f;
			position = glm::vec3(model->matrixWorld * glm::vec4(position, 1.0f));
			lights.lod_probe_grid->get_probe(position, *prim->envMap);
		}

		for (size_t i_model = 0; i_model < scene.gltf_models.size(); i_model++)
		{
			GLTFModel* model = scene.gltf_models[i_model];
			if (model->batched_mesh != nullptr) continue;

			for (size_t i = 0; i < model->m_meshs.size(); i++)
			{
				Mesh& mesh = model->m_meshs[i];
				glm::mat4 matrix = model->matrixWorld;
				if (mesh.node_id >= 0 && mesh.skin_id < 0)
				{
					Node& node = model->m_nodes[mesh.node_id];
					matrix *= node.g_trans;
				}

				for (size_t j = 0; j < mesh.primitives.size(); j++)
				{
					Primitive& primitive = mesh.primitives[j];
					if (primitive.envMap == nullptr)
					{
						primitive.envMap = std::unique_ptr<EnvironmentMap>(new EnvironmentMap);
					}
					glm::vec3 position = (primitive.min_pos + primitive.max_pos) * 0.5f;
					position = glm::vec3(matrix * glm::vec4(position, 1.0f));
					lights.lod_probe_grid->get_probe(position, *primitive.envMap);

				}
			}
		}
	}
}


void GLRenderer::render_depth_primitive(const DepthOnly::RenderParams& params)
{
	if (DepthRenderer == nullptr)
	{
		DepthRenderer = std::unique_ptr<DepthOnly>(new DepthOnly);
	}
	DepthRenderer->render(params);
}

void GLRenderer::render_depth_primitives(const DepthOnly::RenderParams& params, const std::vector<void*>& offset_lst, const std::vector<int>& count_lst)
{
	if (DepthRenderer == nullptr)
	{
		DepthRenderer = std::unique_ptr<DepthOnly>(new DepthOnly);
	}
	DepthRenderer->render_batched(params, offset_lst, count_lst);

}

void GLRenderer::render_depth_model(Camera* p_camera, SimpleModel* model)
{
	const MeshStandardMaterial* material = &model->material;
	if (material->alphaMode != AlphaMode::Opaque) return;

	DepthOnly::RenderParams params;
	params.material_list = &material;
	params.constant_camera = &p_camera->m_constant;
	params.constant_model = &model->m_constant;
	params.primitive = &model->geometry;	
	render_depth_primitive(params);
}

void GLRenderer::render_depth_model(Camera* p_camera, GLTFModel* model)
{
	glm::mat4 view_matrix = p_camera->matrixWorldInverse;

	std::vector<const MeshStandardMaterial*> material_lst(model->m_materials.size());
	for (size_t i = 0; i < material_lst.size(); i++)
		material_lst[i] = model->m_materials[i].get();

	if (model->batched_mesh != nullptr)
	{
		std::vector<std::vector<void*>> offset_lists(material_lst.size());
		std::vector<std::vector<int>> count_lists(material_lst.size());
		for (size_t i = 0; i < model->m_meshs.size(); i++)
		{
			Mesh& mesh = model->m_meshs[i];
			glm::mat4 matrix = model->matrixWorld;
			if (mesh.node_id >= 0 && mesh.skin_id < 0)
			{
				Node& node = model->m_nodes[mesh.node_id];
				matrix *= node.g_trans;
			}
			glm::mat4 MV = p_camera->matrixWorldInverse * matrix;

			for (size_t j = 0; j < mesh.primitives.size(); j++)
			{
				Primitive& primitive = mesh.primitives[j];
				if (!visible(MV, p_camera->projectionMatrix, primitive.min_pos, primitive.max_pos)) continue;

				int idx_list = primitive.material_idx;
				offset_lists[idx_list].push_back((void*)(model->batch_map[i][j]));
				count_lists[idx_list].push_back(primitive.num_face * 3);
			}
		}

		{
			Mesh& mesh = *model->batched_mesh;
			for (size_t j = 0; j < mesh.primitives.size(); j++)
			{
				Primitive& primitive = mesh.primitives[j];
				int idx_material = primitive.material_idx;

				std::vector<void*>& material_offsets = offset_lists[idx_material];
				std::vector<int>& material_counts = count_lists[idx_material];
				if (material_offsets.size() < 1) continue;

				const MeshStandardMaterial* material = material_lst[idx_material];
				if (material->alphaMode != AlphaMode::Opaque) continue;

				DepthOnly::RenderParams params;
				params.material_list = material_lst.data();
				params.constant_camera = &p_camera->m_constant;
				params.constant_model = mesh.model_constant.get();
				params.primitive = &primitive;
				render_depth_primitives(params, material_offsets, material_counts);

			}

		}
	}
	else
	{

		for (size_t i = 0; i < model->m_meshs.size(); i++)
		{
			Mesh& mesh = model->m_meshs[i];
			glm::mat4 matrix = model->matrixWorld;
			if (mesh.node_id >= 0 && mesh.skin_id < 0)
			{
				Node& node = model->m_nodes[mesh.node_id];
				matrix *= node.g_trans;
			}
			glm::mat4 MV = view_matrix * matrix;

			for (size_t j = 0; j < mesh.primitives.size(); j++)
			{
				Primitive& primitive = mesh.primitives[j];
				if (!visible(MV, p_camera->projectionMatrix, primitive.min_pos, primitive.max_pos)) continue;

				const MeshStandardMaterial* material = material_lst[primitive.material_idx];
				if (material->alphaMode != AlphaMode::Opaque) continue;

				DepthOnly::RenderParams params;
				params.material_list = material_lst.data();
				params.constant_camera = &p_camera->m_constant;
				params.constant_model = mesh.model_constant.get();
				params.primitive = &primitive;
				render_depth_primitive(params);
			}
		}
	}
}

void GLRenderer::probe_space_center(Scene& scene, Camera& camera, GLSpaceProbeTarget& target, int width, int height, glm::vec3& sum, float& sum_weight)
{
	if (width < 2 || height < 2) return;

	camera.updateMatrixWorld(false);
	camera.updateConstant();

	target.update_framebuffers(width, height);
	target.bind_buffer();
	glViewport(0, 0, target.m_width, target.m_height);

	glDepthMask(GL_TRUE);
	glClearDepth(1.0f);	
	glClear(GL_DEPTH_BUFFER_BIT);

	for (size_t j = 0; j < scene.simple_models.size(); j++)
	{
		SimpleModel* model = scene.simple_models[j];
		render_depth_model(&camera, model);
	}

	for (size_t j = 0; j < scene.gltf_models.size(); j++)
	{
		GLTFModel* model = scene.gltf_models[j];
		render_depth_model(&camera, model);
	}

	std::vector<float> buf(target.m_width * target.m_height);
	glReadPixels(0, 0, target.m_width, target.m_height, GL_DEPTH_COMPONENT, GL_FLOAT, buf.data());

	glm::vec3 view_sum(0.0f);
	float view_sum_weight = 0.0f;
	for (int j = 0; j < target.m_height; j++)
	{
		for (int i = 0; i < target.m_width; i++)
		{
			float depth = buf[(size_t)target.m_width * (size_t)j + (size_t)i];
			if (depth == 1.0f) continue;
			glm::vec4 pos_clip;
			pos_clip.x = ((float)i + 0.5f) / (float)target.m_width * 2.0f - 1.0f;
			pos_clip.y = ((float)j + 0.5f) / (float)target.m_height * 2.0f - 1.0f;
			pos_clip.z = depth * 2.0f - 1.0f;
			pos_clip.w = 1.0f;

			glm::vec4 pos_view = camera.projectionMatrixInverse* pos_clip;
			pos_view /= pos_view.w;			

			float dis = glm::length(glm::vec3(pos_view));
			glm::vec3 abs_dir = glm::abs(pos_view);
			{
				float max_comp = abs_dir.x;
				if (abs_dir.y > max_comp); max_comp = abs_dir.y;
				if (abs_dir.z > max_comp); max_comp = abs_dir.z;
				abs_dir /= max_comp;
			}
			float lengthSq = glm::dot(abs_dir, abs_dir);
			float weight = (dis *dis*dis)/ (sqrtf(lengthSq));

			view_sum += glm::vec3(pos_view) * 3.0f/4.0f * weight;
			view_sum_weight += weight;
		}
	}

	if (view_sum_weight > 0.0f)
	{
		glm::vec3 view_ave = view_sum / view_sum_weight;
		sum += glm::vec3(camera.matrixWorld * glm::vec4(view_ave, 1.0f)) * view_sum_weight;
		sum_weight += view_sum_weight;
	}
}

glm::vec3 GLRenderer::probe_space_center_cube(Scene& scene, const glm::vec3& position, float zNear, float zFar, IndirectLight& light)
{
	GLSpaceProbeTarget& target = *light.probe_target;
	PerspectiveCamera& camera = *light.probe_camera;
	camera.fov = 90.0f;
	camera.aspect = 1.0f;
	camera.z_near = zNear;
	camera.z_far = zFar;
	camera.updateProjectionMatrix();

	glm::vec3 sum = glm::vec3(0.0f);
	float sum_weight = 0.0f;
	{
		camera.position = position;
		camera.up = { 0.0f, -1.0f, 0.0f };
		camera.lookAt(position + glm::vec3(1.0f, 0.0f, 0.0f));
		probe_space_center(scene, camera, target, 128, 128, sum, sum_weight);
	}

	{
		camera.position = position;
		camera.up = { 0.0f, -1.0f, 0.0f };
		camera.lookAt(position + glm::vec3(-1.0f, 0.0f, 0.0f));
		probe_space_center(scene, camera, target, 128, 128, sum, sum_weight);
	}

	{		
		camera.position = position;
		camera.up = { 0.0f, 0.0f, 1.0f };
		camera.lookAt(position + glm::vec3(0.0f, 1.0f, 0.0f));
		probe_space_center(scene, camera, target, 128, 128, sum, sum_weight);
	}

	{		
		camera.position = position;
		camera.up = { 0.0f, 0.0f, -1.0f };
		camera.lookAt(position + glm::vec3(0.0f, -1.0f, 0.0f));
		probe_space_center(scene, camera, target, 128, 128, sum, sum_weight);
	}

	{		
		camera.position = position;
		camera.up = { 0.0f, -1.0f, 0.0f };
		camera.lookAt(position + glm::vec3(0.0f, 0.0f, 1.0f));
		probe_space_center(scene, camera, target, 128, 128, sum, sum_weight);
	}

	{		
		camera.position = position;
		camera.up = { 0.0f, -1.0f, 0.0f };
		camera.lookAt(position + glm::vec3(0.0f, 0.0f, -1.0f));
		probe_space_center(scene, camera, target, 128, 128, sum, sum_weight);
	}

	return sum / sum_weight;
}

void GLRenderer::_ssao(const Camera& camera, GLRenderTarget& target)
{
	bool msaa = target.msaa();
	int idx = msaa ? 1 : 0;
	if (SSAORenderer[idx] == nullptr)
	{
		SSAORenderer[idx] = std::unique_ptr<SSAO>(new SSAO(msaa));
	}
	SSAO* ssao = SSAORenderer[idx].get();

	target.update_ssao_buffers();

	SSAO::RenderParams params;
	params.buffers = target.m_ssao_buffers.get();
	params.depth_in = target.m_tex_depth.get();
	params.constant_camera = &camera.m_constant;	
	ssao->render(params);
}

void GLRenderer::_render_scene(Scene& scene, Camera& camera, GLRenderTarget& target, bool widgets)
{	
	camera.updateMatrixWorld(false);
	camera.updateConstant();

	// model culling

	std::vector<SimpleModel*> simple_models = scene.simple_models;
	std::vector<GLTFModel*> gltf_models = scene.gltf_models;	

	bool has_alpha = false;
	bool has_opaque = false;

	for (size_t i = 0; i < simple_models.size(); i++)
	{
		SimpleModel* model = simple_models[i];
		if (!visible(camera.matrixWorldInverse * model->matrixWorld, camera.projectionMatrix, model->geometry.min_pos, model->geometry.max_pos))
		{
			simple_models.erase(simple_models.begin() + i);
			i--;
		}
		else
		{
			const MeshStandardMaterial* material = &model->material;
			if (material->alphaMode == AlphaMode::Blend)
			{
				has_alpha = true;
			}
			else
			{
				has_opaque = true;
			}
		}
	}

	for (size_t i = 0; i < gltf_models.size(); i++)
	{
		GLTFModel* model = gltf_models[i];
		if (!visible(camera.matrixWorldInverse * model->matrixWorld, camera.projectionMatrix, model->m_min_pos, model->m_max_pos))
		{
			gltf_models.erase(gltf_models.begin() + i);
			i--;
		}
		else
		{
			size_t num_materials = model->m_materials.size();
			for (size_t i = 0; i < num_materials; i++)
			{
				const MeshStandardMaterial* material = model->m_materials[i].get();
				if (material->alphaMode == AlphaMode::Blend)
				{
					has_alpha = true;
				}
				else
				{
					has_opaque = true;
				}
			}
		}
	}

	// render scene
	target.bind_buffer();
	glEnable(GL_FRAMEBUFFER_SRGB);
	glViewport(0, 0, target.m_width, target.m_height);

	while (scene.background != nullptr)
	{
		{
			ColorBackground* bg = dynamic_cast<ColorBackground*>(scene.background);
			if (bg != nullptr)
			{
				glClearColor(bg->color.r, bg->color.g, bg->color.b, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				break;
			}
		}
		{
			CubeBackground* bg = dynamic_cast<CubeBackground*>(scene.background);
			if (bg != nullptr)
			{
				if (SkyBoxDraw == nullptr)
				{
					SkyBoxDraw = std::unique_ptr<DrawSkyBox>(new DrawSkyBox);
				}
				SkyBoxDraw->render(&camera.m_constant, &bg->cubemap);
				break;
			}
		}
		{
			HemisphereBackground* bg = dynamic_cast<HemisphereBackground*>(scene.background);
			if (bg != nullptr)
			{
				bg->updateConstant();
				if (HemisphereDraw == nullptr)
				{
					HemisphereDraw = std::unique_ptr<DrawHemisphere>(new DrawHemisphere);
				}
				HemisphereDraw->render(&camera.m_constant, &bg->m_constant);
				break;
			}
		}
		{
			BackgroundScene* bg = dynamic_cast<BackgroundScene*>(scene.background);
			PerspectiveCamera* ref_cam =  dynamic_cast<PerspectiveCamera*>(&camera);
			if (bg != nullptr && bg->scene!=nullptr &&  ref_cam != nullptr)
			{
				BackgroundScene::Camera cam(bg, ref_cam);
				_render_scene(*bg->scene, cam, target);
			}
			
		}
		break;
	}

	Lights& lights = scene.lights;

	Fog* fog = nullptr;
	if (scene.fog != nullptr && scene.fog->density > 0)
	{
		fog = scene.fog;
	}

	glDepthMask(GL_TRUE);
	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);

	if (has_opaque)
	{
		glDisable(GL_BLEND);	

		// depth-prepass
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		for (size_t i = 0; i < simple_models.size(); i++)
		{
			SimpleModel* model = simple_models[i];
			render_depth_model(&camera, model);
		}

		for (size_t i = 0; i < gltf_models.size(); i++)
		{
			GLTFModel* model = gltf_models[i];
			render_depth_model(&camera, model);
		}
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}	

	// SSAO
	if (m_use_ssao)
	{
		_ssao(camera, target);
	}

	if (has_opaque)
	{
		// opaque
		for (size_t i = 0; i < simple_models.size(); i++)
		{
			SimpleModel* model = simple_models[i];
			render_model(&camera, lights, fog, model, target, Pass::Opaque);
		}

		for (size_t i = 0; i < gltf_models.size(); i++)
		{
			GLTFModel* model = gltf_models[i];
			render_model(&camera, lights, fog, model, target, Pass::Opaque);
		}

		for (size_t i = 0; i < scene.volume_isosurface_models.size(); i++)
		{
			VolumeIsosurfaceModel* model = scene.volume_isosurface_models[i];
			render_model(&camera, lights, fog, model, target, Pass::Opaque);
		}
	}

	if (widgets)
	{
		for (size_t i = 0; i < scene.widgets.size(); i++)
		{
			Object3D* obj = scene.widgets[i];
			obj->updateWorldMatrix(false, false);
			do
			{
				{
					SimpleModel* model = dynamic_cast<SimpleModel*>(obj);
					if (model != nullptr &&
						visible(camera.matrixWorldInverse * model->matrixWorld, camera.projectionMatrix, model->geometry.min_pos, model->geometry.max_pos))
					{
						update_model(model);
						render_model(&camera, lights, fog, model, target, Pass::Opaque);
						break;
					}
				}
				{
					DirectionalLight* light = dynamic_cast<DirectionalLight*>(obj);
					if (light != nullptr)
					{
						render_widget(&camera, light);
						break;
					}
				}
				{
					ProbeGridWidget* widget = dynamic_cast<ProbeGridWidget*>(obj);
					if (widget != nullptr)
					{
						render_widget(&camera, widget);
						break;
					}
				}
				{
					LODProbeGridWidget* widget = dynamic_cast<LODProbeGridWidget*>(obj);
					if (widget != nullptr)
					{
						render_widget(&camera, widget);
						break;
					}
				}

			} while (false);
		}
	}

	glDepthMask(GL_FALSE);

	if (has_alpha)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		for (size_t i = 0; i < simple_models.size(); i++)
		{
			SimpleModel* model = simple_models[i];
			render_model(&camera, lights, fog, model, target, Pass::Highlight);
		}

		for (size_t i = 0; i < gltf_models.size(); i++)
		{
			GLTFModel* model = gltf_models[i];
			render_model(&camera, lights, fog, model, target, Pass::Highlight);
		}

		target.update_oit_buffers();
		if (!target.msaa())
		{
			if (oit_resolvers[0] == nullptr)
			{
				oit_resolvers[0] = std::unique_ptr<WeightedOIT>(new WeightedOIT(false));
			}
			oit_resolvers[0]->PreDraw(target.m_OITBuffers);
		}
		else
		{
			if (oit_resolvers[1] == nullptr)
			{
				oit_resolvers[1] = std::unique_ptr<WeightedOIT>(new WeightedOIT(true));
			}
			oit_resolvers[1]->PreDraw(target.m_OITBuffers);
		}

		for (size_t i = 0; i < simple_models.size(); i++)
		{
			SimpleModel* model = simple_models[i];
			render_model(&camera, lights, fog, model, target, Pass::Alpha);
		}

		for (size_t i = 0; i < gltf_models.size(); i++)
		{
			GLTFModel* model = gltf_models[i];
			render_model(&camera, lights, fog, model, target, Pass::Alpha);
		}

		target.bind_buffer();

		if (!target.msaa())
		{
			oit_resolvers[0]->PostDraw(target.m_OITBuffers);
		}
		else
		{
			oit_resolvers[1]->PostDraw(target.m_OITBuffers);
		}
	}

}

void GLRenderer::_render(Scene& scene, Camera& camera, GLRenderTarget& target, bool widgets)
{	
	_render_scene(scene, camera, target, widgets);

	Lights& lights = scene.lights;

	Fog* fog = nullptr;
	if (scene.fog != nullptr && scene.fog->density > 0)
	{
		fog = scene.fog;
	}

	if (target.msaa())
	{
		target.resolve_msaa();
	}

	if (fog!=nullptr)
	{
		fog->updateConstant();

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);

		_render_fog(camera, lights, *fog, target);

		glBlendFunc(GL_ONE, GL_ONE);

		for (size_t i = 0; i < scene.directional_lights.size(); i++)
		{
			DirectionalLight* light = scene.directional_lights[i];
			_render_fog_rm(camera, *light, *fog, target);
		}

		if (lights.probe_grid != nullptr || lights.lod_probe_grid != nullptr)
		{
			_render_fog_rm_env(camera, lights, *fog, target);
		}
	}
}

void GLRenderer::_render_scene_simple(Scene& scene, Camera& camera, GLRenderTarget& target)
{
	camera.updateMatrixWorld(false);
	camera.updateConstant();

	// model culling

	std::vector<SimpleModel*> simple_models = scene.simple_models;
	std::vector<GLTFModel*> gltf_models = scene.gltf_models;

	bool has_alpha = false;
	bool has_opaque = false;

	for (size_t i = 0; i < simple_models.size(); i++)
	{
		SimpleModel* model = simple_models[i];
		if (!visible(camera.matrixWorldInverse * model->matrixWorld, camera.projectionMatrix, model->geometry.min_pos, model->geometry.max_pos))
		{
			simple_models.erase(simple_models.begin() + i);
			i--;
		}
		else
		{
			const MeshStandardMaterial* material = &model->material;
			if (material->alphaMode == AlphaMode::Blend)
			{
				has_alpha = true;
			}
			else
			{
				has_opaque = true;
			}
		}
	}

	for (size_t i = 0; i < gltf_models.size(); i++)
	{
		GLTFModel* model = gltf_models[i];
		if (!visible(camera.matrixWorldInverse * model->matrixWorld, camera.projectionMatrix, model->m_min_pos, model->m_max_pos))
		{
			gltf_models.erase(gltf_models.begin() + i);
			i--;
		}
		else
		{
			size_t num_materials = model->m_materials.size();
			for (size_t i = 0; i < num_materials; i++)
			{
				const MeshStandardMaterial* material = model->m_materials[i].get();
				if (material->alphaMode == AlphaMode::Blend)
				{
					has_alpha = true;
				}
				else
				{
					has_opaque = true;
				}
			}
		}
	}

	// render scene
	target.bind_buffer();
	glEnable(GL_FRAMEBUFFER_SRGB);
	glViewport(0, 0, target.m_width, target.m_height);

	while (scene.background != nullptr)
	{
		{
			ColorBackground* bg = dynamic_cast<ColorBackground*>(scene.background);
			if (bg != nullptr)
			{
				glClearColor(bg->color.r, bg->color.g, bg->color.b, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				break;
			}
		}
		{
			CubeBackground* bg = dynamic_cast<CubeBackground*>(scene.background);
			if (bg != nullptr)
			{
				if (SkyBoxDraw == nullptr)
				{
					SkyBoxDraw = std::unique_ptr<DrawSkyBox>(new DrawSkyBox);
				}
				SkyBoxDraw->render(&camera.m_constant, &bg->cubemap);
				break;
			}
		}
		{
			HemisphereBackground* bg = dynamic_cast<HemisphereBackground*>(scene.background);
			if (bg != nullptr)
			{
				bg->updateConstant();
				if (HemisphereDraw == nullptr)
				{
					HemisphereDraw = std::unique_ptr<DrawHemisphere>(new DrawHemisphere);
				}
				HemisphereDraw->render(&camera.m_constant, &bg->m_constant);
				break;
			}
		}
		{
			BackgroundScene* bg = dynamic_cast<BackgroundScene*>(scene.background);
			PerspectiveCamera* ref_cam = dynamic_cast<PerspectiveCamera*>(&camera);
			if (bg != nullptr && bg->scene != nullptr && ref_cam != nullptr)
			{
				BackgroundScene::Camera cam(bg, ref_cam);
				_render_scene_simple(*bg->scene, cam, target);
			}

		}
		break;
	}

	Lights& lights = scene.lights;

	Fog* fog = nullptr;
	if (scene.fog != nullptr && scene.fog->density > 0)
	{
		fog = scene.fog;
	}

	glDepthMask(GL_TRUE);
	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);

	if (has_opaque)
	{
		glDisable(GL_BLEND);
		// depth-prepass
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		for (size_t i = 0; i < simple_models.size(); i++)
		{
			SimpleModel* model = simple_models[i];
			render_depth_model(&camera, model);
		}

		for (size_t i = 0; i < gltf_models.size(); i++)
		{
			GLTFModel* model = gltf_models[i];
			render_depth_model(&camera, model);
		}
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	}	

	if (has_opaque)
	{
		// opaque
		for (size_t i = 0; i < simple_models.size(); i++)
		{
			SimpleModel* model = simple_models[i];
			render_model_simple(&camera, lights, fog, model, Pass::Opaque);
		}

		for (size_t i = 0; i < gltf_models.size(); i++)
		{
			GLTFModel* model = gltf_models[i];
			render_model_simple(&camera, lights, fog, model, Pass::Opaque);
		}		
	}

	glDepthMask(GL_FALSE);

	if (has_alpha)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		for (size_t i = 0; i < simple_models.size(); i++)
		{
			SimpleModel* model = simple_models[i];
			render_model_simple(&camera, lights, fog, model, Pass::Highlight);
		}

		for (size_t i = 0; i < gltf_models.size(); i++)
		{
			GLTFModel* model = gltf_models[i];
			render_model_simple(&camera, lights, fog, model, Pass::Highlight);
		}

		target.update_oit_buffers();
		if (!target.msaa())
		{
			if (oit_resolvers[0] == nullptr)
			{
				oit_resolvers[0] = std::unique_ptr<WeightedOIT>(new WeightedOIT(false));
			}
			oit_resolvers[0]->PreDraw(target.m_OITBuffers);
		}
		else
		{
			if (oit_resolvers[1] == nullptr)
			{
				oit_resolvers[1] = std::unique_ptr<WeightedOIT>(new WeightedOIT(true));
			}
			oit_resolvers[1]->PreDraw(target.m_OITBuffers);
		}

		for (size_t i = 0; i < simple_models.size(); i++)
		{
			SimpleModel* model = simple_models[i];
			render_model_simple(&camera, lights, fog, model, Pass::Alpha);
		}

		for (size_t i = 0; i < gltf_models.size(); i++)
		{
			GLTFModel* model = gltf_models[i];
			render_model_simple(&camera, lights, fog, model, Pass::Alpha);
		}

		target.bind_buffer();

		if (!target.msaa())
		{
			oit_resolvers[0]->PostDraw(target.m_OITBuffers);
		}
		else
		{
			oit_resolvers[1]->PostDraw(target.m_OITBuffers);
		}
	}
}

void GLRenderer::_render_simple(Scene& scene, Camera& camera, GLRenderTarget& target)
{
	_render_scene_simple(scene, camera, target);

	Lights& lights = scene.lights;

	Fog* fog = nullptr;
	if (scene.fog != nullptr && scene.fog->density > 0)
	{
		fog = scene.fog;
	}

	if (target.msaa())
	{
		target.resolve_msaa();
	}

	if (fog != nullptr)
	{
		fog->updateConstant();

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);

		_render_fog(camera, lights, *fog, target);

		glBlendFunc(GL_ONE, GL_ONE);

		for (size_t i = 0; i < scene.directional_lights.size(); i++)
		{
			DirectionalLight* light = scene.directional_lights[i];
			_render_fog_rm_simple(camera, *light, *fog, target);
		}

		if (lights.probe_grid != nullptr || lights.lod_probe_grid !=nullptr)
		{
			_render_fog_rm_env_simple(camera, lights, *fog, target);
		}
	}

}

void GLRenderer::_render_cube(Scene& scene, CubeRenderTarget& target, const glm::vec3& position, float zNear, float zFar, const glm::quat& rotation)
{
	{
		PerspectiveCamera camera(90.0f, 1.0f, zNear, zFar);
		camera.position = position;
		camera.up = { 0.0f, -1.0f, 0.0f };
		camera.lookAt(position + glm::vec3(1.0f, 0.0f, 0.0f));
		camera.applyQuaternion(rotation);
		_render_simple(scene, camera, *target.m_faces[0]);
	}

	{
		PerspectiveCamera camera(90.0f, 1.0f, zNear, zFar);
		camera.position = position;
		camera.up = { 0.0f, -1.0f, 0.0f };
		camera.lookAt(position + glm::vec3(-1.0f, 0.0f, 0.0f));
		camera.applyQuaternion(rotation);
		_render_simple(scene, camera, *target.m_faces[1]);
	}

	{
		PerspectiveCamera camera(90.0f, 1.0f, zNear, zFar);
		camera.position = position;
		camera.up = { 0.0f, 0.0f, 1.0f };
		camera.lookAt(position + glm::vec3(0.0f, 1.0f, 0.0f));
		camera.applyQuaternion(rotation);
		_render_simple(scene, camera, *target.m_faces[2]);
	}

	{
		PerspectiveCamera camera(90.0f, 1.0f, zNear, zFar);
		camera.position = position;
		camera.up = { 0.0f, 0.0f, -1.0f };
		camera.lookAt(position + glm::vec3(0.0f, -1.0f, 0.0f));
		camera.applyQuaternion(rotation);
		_render_simple(scene, camera, *target.m_faces[3]);
	}

	{
		PerspectiveCamera camera(90.0f, 1.0f, zNear, zFar);
		camera.position = position;
		camera.up = { 0.0f, -1.0f, 0.0f };
		camera.lookAt(position + glm::vec3(0.0f, 0.0f, 1.0f));
		camera.applyQuaternion(rotation);
		_render_simple(scene, camera, *target.m_faces[4]);
	}

	{
		PerspectiveCamera camera(90.0f, 1.0f, zNear, zFar);
		camera.position = position;
		camera.up = { 0.0f, -1.0f, 0.0f };
		camera.lookAt(position + glm::vec3(0.0f, 0.0f, -1.0f));
		camera.applyQuaternion(rotation);
		_render_simple(scene, camera, *target.m_faces[5]);
	}
}

#define REFLECTION_RAY_MARCHING 0

void GLRenderer::render(Scene& scene, Camera& camera, GLRenderTarget& target)
{
	if (scene.indirectLight != nullptr && scene.indirectLight->dynamic_map)
	{
		IndirectLight* light = scene.indirectLight;
#if REFLECTION_RAY_MARCHING
		if (light->reflection == nullptr || light->reflection->tex_id_dis == (unsigned)(-1))
		{
			light->reflection = std::unique_ptr<ReflectionMap>(new ReflectionMap(true));
		}
#else
		if (light->reflection == nullptr)
		{
			light->reflection = std::unique_ptr<ReflectionMap>(new ReflectionMap(false));
		}
#endif

	}
	_pre_render(scene);

	if (scene.indirectLight != nullptr && scene.indirectLight->dynamic_map)
	{
		IndirectLight* light = scene.indirectLight;
		PerspectiveCamera* p_cam = (PerspectiveCamera*)(&camera);

		camera.updateMatrixWorld(false);
		glm::vec3 cam_pos = camera.getWorldPosition();
		float delta = glm::length(cam_pos - light->camera_position);
		if (delta > 0.0f)
		{
			light->camera_position = cam_pos;

#if REFLECTION_RAY_MARCHING
			light->probe_position = cam_pos;

#else
			//glm::vec3 pos = probe_space_center_cube(scene, camera.getWorldPosition(), p_cam->z_near, p_cam->z_far, *light);

			glm::vec3 sum = glm::vec3(0.0f);
			float sum_weight = 0.0f;
			probe_space_center(scene, camera, *light->probe_target, target.m_width, target.m_height, sum, sum_weight);
			glm::vec3 pos = sum / sum_weight;

			pos.x = 0.5f * pos.x + 0.5f * cam_pos.x;
			pos.y = cam_pos.y;
			//pos.y = 0.5f * pos.y + 0.5f * cam_pos.y;
			pos.z = 0.5f * pos.z + 0.5f * cam_pos.z;
			//light->probe_position = 0.8f * light->probe_position + 0.2f * pos;
			light->probe_position = pos;
#endif			

			_render_cube(scene, *light->cube_target, light->probe_position, p_cam->z_near, p_cam->z_far);

			if (EnvCreator == nullptr)
			{
				EnvCreator = std::unique_ptr<EnvironmentMapCreator>(new EnvironmentMapCreator);
			}

			EnvCreator->CreateReflection(*light->reflection, light->cube_target->m_cube_map.get());

#if REFLECTION_RAY_MARCHING
			if (ReflDisCreator == nullptr)
			{
				ReflDisCreator = std::unique_ptr<ReflectionDistanceCreator>(new ReflectionDistanceCreator);
			}
			ReflDisCreator->Create(light->cube_target.get(), light->reflection.get(), p_cam->z_near, p_cam->z_far);
#endif
		}
	
	}
	_render(scene, camera, target, true);
}

void GLRenderer::render_picking(Scene& scene, Camera& camera, GLPickingTarget& target)
{	
	target.bind_buffer();
	glViewport(0, 0, target.m_width, target.m_height);

	glDepthMask(GL_TRUE);
	glClearDepth(1.0f);
	glClearColorIuiEXT(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

	target.m_idx_info.clear();

	GLPickingTarget::IdxInfo bg_info;
	bg_info.obj = nullptr;
	bg_info.primitive_idx = 0;
	target.m_idx_info.push_back(bg_info);

	for (size_t j = 0; j < scene.simple_models.size(); j++)
	{
		SimpleModel* model = scene.simple_models[j];
		render_picking_model(&camera, model, target);
	}

	for (size_t j = 0; j < scene.gltf_models.size(); j++)
	{
		GLTFModel* model = scene.gltf_models[j];
		render_picking_model(&camera, model, target);
	}

	for (size_t i = 0; i < scene.widgets.size(); i++)
	{
		Object3D* obj = scene.widgets[i];
		SimpleModel* model = dynamic_cast<SimpleModel*>(obj);
		if (model != nullptr &&
			visible(camera.matrixWorldInverse * model->matrixWorld, camera.projectionMatrix, model->geometry.min_pos, model->geometry.max_pos))
		{
			render_picking_model(&camera, model, target);			
		}		
	}
}

void GLRenderer::renderCube(Scene& scene, CubeRenderTarget& target, const glm::vec3& position, float zNear, float zFar, const glm::quat& rotation)
{
	_pre_render(scene);
	_render_cube(scene, target, position, zNear, zFar, rotation);
}

void GLRenderer::updateProbe(Scene& scene, CubeRenderTarget& target, ProbeGrid& probe_grid, glm::ivec3 idx, float zNear, float zFar, float k)
{
	glm::vec3 size_grid = probe_grid.coverage_max - probe_grid.coverage_min;
	glm::vec3 pos_normalized = (glm::vec3(idx) + 0.5f) / glm::vec3(probe_grid.divisions);
	pos_normalized.y = powf(pos_normalized.y, probe_grid.ypower);
	glm::vec3 pos = probe_grid.coverage_min + pos_normalized * size_grid;

	glm::vec3 axis = glm::sphericalRand(1.0f);
	float angle = randRad();
	glm::quat rotation = glm::angleAxis(angle, axis);
	renderCube(scene, target, pos, zNear, zFar, rotation);

	glm::vec4 coeffs[9];
	EnvironmentMapCreator::CreateSH(coeffs, target.m_cube_map->tex_id, target.m_width, rotation);

	int index = idx.x + (idx.y + (idx.z * probe_grid.divisions.y)) * probe_grid.divisions.x;
	glm::vec4* dest_coeffs = probe_grid.m_probe_data.data() + index * 9;
	for (int i = 0; i < 9; i++)
	{
		dest_coeffs[i] = (1.0f - k) * dest_coeffs[i] + k * coeffs[i];

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, probe_grid.m_probe_bufs[i]->m_id);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, index * sizeof(glm::vec4), sizeof(glm::vec4), &dest_coeffs[i]);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}	

	probe_grid.presample_probe(index);
}

void GLRenderer::updateProbe(Scene& scene, CubeRenderTarget& target, LODProbeGrid& probe_grid, int idx, float zNear, float zFar, float k)
{
	/*float vis = probe_grid.m_visibility_data[idx * 26];
	if (vis <= 0.0f) return;*/

	bool full_update = true;

	glm::vec4* dest_coeffs = &probe_grid.m_probe_data[idx * 10 + 1];

	/*if (idx < probe_grid.m_sub_index.size())
	{
		int sub_idx = probe_grid.m_sub_index[idx];
		if (sub_idx >= 0)
		{
			size_t base_offset = probe_grid.base_divisions.x * probe_grid.base_divisions.y * probe_grid.base_divisions.z;
			size_t sub_offset = base_offset + sub_idx * 8;

			float valid_count = 0.0f;
			glm::vec4 coeffs[9];
			memset(coeffs, 0, sizeof(glm::vec4) * 9);
			for (int i = 0; i < 8; i++)
			{
				size_t probe_offset = sub_offset + i;
				float vis_sub = probe_grid.m_visibility_data[probe_offset * 26];
				if (vis_sub)
				{
					glm::vec4* src_coeffs = &probe_grid.m_probe_data[probe_offset * 10 + 1];
					for (int j = 0; j < 9; j++)
					{
						coeffs[j] += src_coeffs[j];
					}
					valid_count += 1.0f;
				}
			}

			if (valid_count > 0.0f)
			{
				for (int j = 0; j < 9; j++)
				{
					dest_coeffs[j] = coeffs[j] / valid_count;
				}
				full_update = false;
			}			
		}

	}*/

	if (full_update)
	{
		glm::vec3 pos = probe_grid.m_probe_data[idx * 10];

		glm::vec3 axis = glm::sphericalRand(1.0f);
		float angle = randRad();
		glm::quat rotation = glm::angleAxis(angle, axis);
		renderCube(scene, target, pos, zNear, zFar, rotation);

		glm::vec4 coeffs[9];
		EnvironmentMapCreator::CreateSH(coeffs, target.m_cube_map->tex_id, target.m_width, rotation);
		
		for (int i = 0; i < 9; i++)
		{
			dest_coeffs[i] = (1.0f - k) * dest_coeffs[i] + k * coeffs[i];

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, probe_grid.m_probe_bufs[i]->m_id);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * idx, sizeof(glm::vec4), &dest_coeffs[i]);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		}
	}

	
	probe_grid.presample_probe(idx);

}

int GLRenderer::updateProbes(Scene& scene, ProbeGrid& probe_grid, int start_idx, int num_directions, float rate_vis, float rate_irr)
{
	int max_probes = (1 << 17) / num_directions;
	if (max_probes < 1) max_probes = 1;

	glm::ivec3 divs = probe_grid.divisions;
	int num_probes = divs.x * divs.y * divs.z;
	num_probes -= start_idx;
	if (num_probes > max_probes) num_probes = max_probes;

	BVHRenderTarget bvh_target;
	bvh_target.update(num_directions, num_probes);

	ProbeRayList prl(probe_grid, start_idx, start_idx + num_probes, num_directions);
	bvh_renderer.render_probe(scene, prl, bvh_target);

	bvh_renderer.update_probe_visibility(bvh_target, prl, probe_grid, start_idx, rate_vis);
	bvh_renderer.update_probe_irradiance(bvh_target, prl, probe_grid, start_idx, rate_irr);

	probe_grid.updated = true;
	
	return num_probes;
}

int GLRenderer::updateProbes(Scene& scene, LODProbeGrid& probe_grid, int start_idx, int num_directions, float rate_vis, float rate_irr)
{
	int max_probes = (1 << 17) / num_directions;
	int num_probes = probe_grid.getNumberOfProbes();
	num_probes -= start_idx;
	if (num_probes > max_probes) num_probes = max_probes;

	BVHRenderTarget bvh_target;
	bvh_target.update(num_directions, num_probes);

	ProbeRayList prl(probe_grid, start_idx, start_idx + num_probes, num_directions);
	bvh_renderer.render_probe(scene, prl, bvh_target);

	bvh_renderer.update_probe_visibility(bvh_target, prl, probe_grid, start_idx, rate_vis);
	bvh_renderer.update_probe_irradiance(bvh_target, prl, probe_grid, start_idx, rate_irr);

	probe_grid.updated = true;

	return num_probes;
}

void GLRenderer::renderTexture(GLTexture2D* tex, int x, int y, int width, int height, GLRenderTarget& target, bool flipY, float alpha)
{
	if (TextureDraw == nullptr)
	{
		TextureDraw = std::unique_ptr<DrawTexture>(new DrawTexture(false, flipY));
	}	
	glBindFramebuffer(GL_FRAMEBUFFER, target.m_fbo_video==0? target.m_fbo_default : target.m_fbo_video);
	TextureDraw->render(tex->tex_id, x, target.m_height - (y + height), width, height, alpha<1.0f, alpha);
}

void GLRenderer::scene_to_volume_primitive(const SceneToVolume::RenderParams& params)
{
	if (SceneVolumeConvert == nullptr)
	{
		SceneVolumeConvert = std::unique_ptr<SceneToVolume>(new SceneToVolume);
	}
	SceneVolumeConvert->render(params);
}

void GLRenderer::scene_to_volume_model(SimpleModel* model, SceneToVolume::RenderParams& params)
{	
	params.constant_model = &model->m_constant;
	params.primitive = &model->geometry;
	scene_to_volume_primitive(params);
}

void GLRenderer::scene_to_volume_model(GLTFModel* model, SceneToVolume::RenderParams& params)
{

	for (size_t i = 0; i < model->m_meshs.size(); i++)
	{
		Mesh& mesh = model->m_meshs[i];
		glm::mat4 matrix = model->matrixWorld;
		if (mesh.node_id >= 0 && mesh.skin_id < 0)
		{
			Node& node = model->m_nodes[mesh.node_id];
			matrix *= node.g_trans;
		}		

		for (size_t j = 0; j < mesh.primitives.size(); j++)
		{
			Primitive& primitive = mesh.primitives[j];
			params.constant_model = mesh.model_constant.get();
			params.primitive = &primitive;
			scene_to_volume_primitive(params);			
		}
	}
}

void GLRenderer::sceneToVolume(Scene& scene, unsigned tex_id_volume, const glm::vec3& coverage_min, const glm::vec3& coverage_max, const glm::ivec3& divisions)
{

	scene.clear_lists();

	auto* p_scene = &scene;
	scene.traverse([p_scene](Object3D* obj) {
		do
		{
			{
				SimpleModel* model = dynamic_cast<SimpleModel*>(obj);
				if (model)
				{
					p_scene->simple_models.push_back(model);
					break;
				}
			}
			{
				GLTFModel* model = dynamic_cast<GLTFModel*>(obj);
				if (model)
				{
					p_scene->gltf_models.push_back(model);
					break;
				}
			}			
		} while (false);

		obj->updateWorldMatrix(false, false);
	});

	// update models
	for (size_t i = 0; i < scene.simple_models.size(); i++)
	{
		SimpleModel* model = scene.simple_models[i];
		update_model(model);
	}

	for (size_t i = 0; i < scene.gltf_models.size(); i++)
	{
		GLTFModel* model = scene.gltf_models[i];
		update_model(model);
	}
		
	SceneToVolume::RenderParams params;
	params.tex_id_volume = tex_id_volume;
	params.coverage_min = coverage_min;
	params.coverage_max = coverage_max;
	params.divisions = divisions;

	for (size_t i = 0; i < scene.simple_models.size(); i++)
	{
		SimpleModel* model = scene.simple_models[i];
		scene_to_volume_model(model, params);
	}

	for (size_t i = 0; i < scene.gltf_models.size(); i++)
	{
		GLTFModel* model = scene.gltf_models[i];
		scene_to_volume_model(model, params);
	}
}

void GLRenderer::rasterize_atlas_primitive(const RasterizeAtlas::RenderParams& params)
{
	const MeshStandardMaterial* material = params.material_list[params.primitive->material_idx];
	
	bool has_normal_map = material->tex_idx_normalMap >= 0;

	int idx = has_normal_map ? 1 : 0;
	if (atlas_rasterizer[idx] == nullptr)
	{
		atlas_rasterizer[idx] = std::unique_ptr<RasterizeAtlas>(new RasterizeAtlas(has_normal_map));
	}

	RasterizeAtlas* routine = atlas_rasterizer[idx].get();
	glLineWidth(2.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	routine->render(params);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	routine->render(params);
}


void GLRenderer::rasterize_atlas(GLTFModel* model)
{
	model->updateWorldMatrix(false, false);
	update_model(model);

	glBindFramebuffer(GL_FRAMEBUFFER, model->lightmap_target->m_fbo);

	const GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, drawBuffers);
	glViewport(0, 0, model->lightmap_target->m_width, model->lightmap_target->m_height);

	float zero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glClearBufferfv(GL_COLOR, 0, zero);
	glClearBufferfv(GL_COLOR, 1, zero);

	glDisable(GL_BLEND);

	std::vector<const GLTexture2D*> tex_lst(model->m_textures.size());
	for (size_t i = 0; i < tex_lst.size(); i++)
	{
		auto iter = model->m_repl_textures.find(i);
		if (iter != model->m_repl_textures.end())
		{
			tex_lst[i] = iter->second;
		}
		else
		{
			tex_lst[i] = model->m_textures[i].get();
		}
	}

	std::vector<const MeshStandardMaterial*> material_lst(model->m_materials.size());
	for (size_t i = 0; i < material_lst.size(); i++)
		material_lst[i] = model->m_materials[i].get();

	if (model->batched_mesh != nullptr)
	{
		Mesh& mesh = *model->batched_mesh;
		for (size_t j = 0; j < mesh.primitives.size(); j++)
		{
			Primitive& primitive = mesh.primitives[j];

			RasterizeAtlas::RenderParams params;
			params.tex_list = tex_lst.data();
			params.material_list = material_lst.data();
			params.constant_model = mesh.model_constant.get();
			params.primitive = &primitive;
			rasterize_atlas_primitive(params);
		}
	}
	else
	{
		for (size_t i = 0; i < model->m_meshs.size(); i++)
		{
			Mesh& mesh = model->m_meshs[i];
			for (size_t j = 0; j < mesh.primitives.size(); j++)
			{
				Primitive& primitive = mesh.primitives[j];

				RasterizeAtlas::RenderParams params;
				params.tex_list = tex_lst.data();
				params.material_list = material_lst.data();
				params.constant_model = mesh.model_constant.get();
				params.primitive = &primitive;
				rasterize_atlas_primitive(params);
			}
		}
	}

}

int GLRenderer::updateLightmap(Scene& scene, Lightmap& lm, LightmapRenderTarget& src, int start_texel, int num_directions, float k)
{
	int max_texels = (1 << 17) / num_directions;
	if (max_texels < 1) max_texels = 1;

	int num_texels = src.count_valid - start_texel;
	if (num_texels > max_texels) num_texels = max_texels;

	int width = 512;
	if (width < num_directions) width = num_directions;

	int texels_per_row = width / num_directions;

	int height = (num_texels + texels_per_row - 1) / texels_per_row;

	BVHRenderTarget bvh_target;
	bvh_target.update(width, height);

	LightmapRayList lmrl(&src, &bvh_target, start_texel, start_texel + num_texels, num_directions);
	bvh_renderer.render_lightmap(scene, lmrl, bvh_target);

	bvh_renderer.update_lightmap(bvh_target, lmrl, lm, start_texel, k);
	

	return num_texels;

}

void GLRenderer::filterLightmap(Lightmap& lm, LightmapRenderTarget& src, const glm::mat4& model_mat)
{
	bvh_renderer.filter_lightmap(src, lm, model_mat);
}

#if 0
bool GLRenderer::compressLightmap(Scene& scene, GLTFModel* model)
{
	if (model->lightmap == nullptr) return false;
	if (model->lightmap_target == nullptr)
	{
		model->init_lightmap_target(this);
	}

	Lights& lights = scene.lights;
	lights.probe_grid = nullptr;
	lights.lod_probe_grid = nullptr;

	while (scene.indirectLight)
	{
		{
			ProbeGrid* probeGrid = dynamic_cast<ProbeGrid*>(scene.indirectLight);
			if (probeGrid != nullptr)
			{
				probeGrid->updateConstant();
				lights.probe_grid = probeGrid;
				break;
			}

		}
		{
			LODProbeGrid* probeGrid = dynamic_cast<LODProbeGrid*>(scene.indirectLight);
			if (probeGrid != nullptr)
			{
				probeGrid->updateConstant();
				lights.lod_probe_grid = probeGrid;
				break;
			}

		}
		break;
	}
	bool has_probe_grid = lights.probe_grid != nullptr;
	bool has_lod_probe_grid = lights.lod_probe_grid != nullptr;
	if (!has_probe_grid && !has_lod_probe_grid) return false;

	int idx = has_lod_probe_grid ? 1 : 0;
	if (lightmap_compressor[idx] == nullptr)
	{
		lightmap_compressor[idx] = std::unique_ptr<CompressLightmap>(new CompressLightmap(has_probe_grid, has_lod_probe_grid));
	}

	if (model->lightmap_probe_vis == nullptr)
	{
		int width = model->lightmap->width;
		int height = model->lightmap->height;
		model->lightmap_probe_vis = std::unique_ptr<ProbeVisibilityMap>(new ProbeVisibilityMap(width, height));
	}

	uint8_t value = 255;
	glClearTexImage(model->lightmap_probe_vis->tex->tex_id, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, &value);

	CompressLightmap::RenderParams params;
	params.atlas = model->lightmap_target.get();
	params.light_map = model->lightmap->lightmap.get();
	params.probe_grid = lights.probe_grid;
	params.lod_probe_grid = lights.lod_probe_grid;
	params.probe_visibility_map = model->lightmap_probe_vis->tex.get();
	lightmap_compressor[idx]->compress(params);

	return true;
}

bool GLRenderer::decompressLightmap(Scene& scene, GLTFModel* model)
{
	if (model->lightmap == nullptr || model->lightmap_probe_vis == nullptr) return false;
	if (model->lightmap_target == nullptr)
	{
		model->init_lightmap_target(this);
	}

	Lights& lights = scene.lights;
	lights.probe_grid = nullptr;
	lights.lod_probe_grid = nullptr;

	while (scene.indirectLight)
	{
		{
			ProbeGrid* probeGrid = dynamic_cast<ProbeGrid*>(scene.indirectLight);
			if (probeGrid != nullptr)
			{
				probeGrid->updateConstant();
				lights.probe_grid = probeGrid;
				break;
			}

		}
		{
			LODProbeGrid* probeGrid = dynamic_cast<LODProbeGrid*>(scene.indirectLight);
			if (probeGrid != nullptr)
			{
				probeGrid->updateConstant();
				lights.lod_probe_grid = probeGrid;
				break;
			}

		}
		break;
	}
	bool has_probe_grid = lights.probe_grid != nullptr;
	bool has_lod_probe_grid = lights.lod_probe_grid != nullptr;
	if (!has_probe_grid && !has_lod_probe_grid) return false;

	int idx = has_lod_probe_grid ? 1 : 0;
	if (lightmap_decompressor[idx] == nullptr)
	{
		lightmap_decompressor[idx] = std::unique_ptr<DecompressLightmap>(new DecompressLightmap(has_probe_grid, has_lod_probe_grid));
	}

	glm::vec4 zero = { 0.0f, 0.0f, 0.0f, 0.0f };
	glClearTexImage(model->lightmap->lightmap->tex_id, 0, GL_RGBA, GL_FLOAT, &zero);

	DecompressLightmap::RenderParams params;
	params.atlas = model->lightmap_target.get();
	params.probe_visibility_map = model->lightmap_probe_vis->tex.get();	
	params.probe_grid = lights.probe_grid;
	params.lod_probe_grid = lights.lod_probe_grid;
	params.light_map = model->lightmap->lightmap.get();	
	lightmap_decompressor[idx]->decompress(params);

	//bvh_renderer.filter_lightmap(*model->lightmap_target, *model->lightmap, model->matrixWorld);
	
	return true;
}
#endif
