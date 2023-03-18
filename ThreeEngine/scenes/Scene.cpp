#include "Scene.h"
#include "models/SimpleModel.h"
#include "models/GLTFModel.h"

void Scene::add_widget(Object3D* object)
{
	auto iter = std::find(widgets.begin(), widgets.end(), object);
	if (iter == widgets.end())
	{
		widgets.push_back(object);
	}
}

void Scene::remove_widget(Object3D* object)
{
	auto iter = std::find(widgets.begin(), widgets.end(), object);
	if (iter != widgets.end())
	{
		widgets.erase(iter);
	}
}

void Scene::clear_widgets()
{	
	widgets.clear();
}

void Scene::get_bounding_box(glm::vec3& min_pos, glm::vec3& max_pos, const glm::mat4& view_matrix)
{
	simple_models.clear();
	gltf_models.clear();

	auto* p_scene = this;
	traverse([p_scene](Object3D* obj) {
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

	min_pos = { FLT_MAX, FLT_MAX, FLT_MAX };
	max_pos = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

	for (size_t i = 0; i < simple_models.size(); i++)
	{
		SimpleModel* model = simple_models[i];

		glm::vec3 model_min_pos = model->geometry.min_pos;
		glm::vec3 model_max_pos = model->geometry.max_pos;
		glm::mat4 model_matrix = model->matrixWorld;
		glm::mat4 MV = view_matrix * model_matrix;

		glm::vec4 world_pos[8];
		world_pos[0] = MV * glm::vec4(model_min_pos.x, model_min_pos.y, model_min_pos.z, 1.0f);
		world_pos[1] = MV * glm::vec4(model_max_pos.x, model_min_pos.y, model_min_pos.z, 1.0f);
		world_pos[2] = MV * glm::vec4(model_min_pos.x, model_max_pos.y, model_min_pos.z, 1.0f);
		world_pos[3] = MV * glm::vec4(model_max_pos.x, model_max_pos.y, model_min_pos.z, 1.0f);
		world_pos[4] = MV * glm::vec4(model_min_pos.x, model_min_pos.y, model_max_pos.z, 1.0f);
		world_pos[5] = MV * glm::vec4(model_max_pos.x, model_min_pos.y, model_max_pos.z, 1.0f);
		world_pos[6] = MV * glm::vec4(model_min_pos.x, model_max_pos.y, model_max_pos.z, 1.0f);
		world_pos[7] = MV * glm::vec4(model_max_pos.x, model_max_pos.y, model_max_pos.z, 1.0f);

		for (int j = 0; j < 8; j++)
		{
			glm::vec4 pos = world_pos[j];
			if (pos.x < min_pos.x) min_pos.x = pos.x;
			if (pos.x > max_pos.x) max_pos.x = pos.x;
			if (pos.y < min_pos.y) min_pos.y = pos.y;
			if (pos.y > max_pos.y) max_pos.y = pos.y;
			if (pos.z < min_pos.z) min_pos.z = pos.z;
			if (pos.z > max_pos.z) max_pos.z = pos.z;
		}
	}

	for (size_t i = 0; i < gltf_models.size(); i++)
	{
		GLTFModel* model = gltf_models[i];
		glm::vec3 model_min_pos = model->m_min_pos;
		glm::vec3 model_max_pos = model->m_max_pos;
		glm::mat4 model_matrix = model->matrixWorld;
		glm::mat4 MV = view_matrix * model_matrix;

		glm::vec4 world_pos[8];
		world_pos[0] = MV * glm::vec4(model_min_pos.x, model_min_pos.y, model_min_pos.z, 1.0f);
		world_pos[1] = MV * glm::vec4(model_max_pos.x, model_min_pos.y, model_min_pos.z, 1.0f);
		world_pos[2] = MV * glm::vec4(model_min_pos.x, model_max_pos.y, model_min_pos.z, 1.0f);
		world_pos[3] = MV * glm::vec4(model_max_pos.x, model_max_pos.y, model_min_pos.z, 1.0f);
		world_pos[4] = MV * glm::vec4(model_min_pos.x, model_min_pos.y, model_max_pos.z, 1.0f);
		world_pos[5] = MV * glm::vec4(model_max_pos.x, model_min_pos.y, model_max_pos.z, 1.0f);
		world_pos[6] = MV * glm::vec4(model_min_pos.x, model_max_pos.y, model_max_pos.z, 1.0f);
		world_pos[7] = MV * glm::vec4(model_max_pos.x, model_max_pos.y, model_max_pos.z, 1.0f);

		for (int j = 0; j < 8; j++)
		{
			glm::vec4 pos = world_pos[j];
			if (pos.x < min_pos.x) min_pos.x = pos.x;
			if (pos.x > max_pos.x) max_pos.x = pos.x;
			if (pos.y < min_pos.y) min_pos.y = pos.y;
			if (pos.y > max_pos.y) max_pos.y = pos.y;
			if (pos.z < min_pos.z) min_pos.z = pos.z;
			if (pos.z > max_pos.z) max_pos.z = pos.z;
		}
	}
}

