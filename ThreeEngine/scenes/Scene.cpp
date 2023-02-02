#include "Scene.h"

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

