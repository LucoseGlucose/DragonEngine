#include "stdafx.h"
#include "Scene.h"

Scene::Scene(std::string name) : Object(name), objects()
{

}

Scene::~Scene()
{
	for (size_t i = 0; i < objects.size(); i++)
	{
		delete objects[i];
	}
}

void Scene::OnStart()
{
	for (size_t i = 0; i < objects.size(); i++)
	{
		objects[i]->OnStart();
	}
}

void Scene::OnUpdate()
{
	for (size_t i = 0; i < objects.size(); i++)
	{
		objects[i]->OnUpdate();
	}
}

void Scene::OnEnd()
{
	for (size_t i = 0; i < objects.size(); i++)
	{
		objects[i]->OnEnd();
	}
}

SceneObject* Scene::AddObject(SceneObject* object)
{
	objects.push_back(object);
	return object;
}

void Scene::DestroyObject(SceneObject* object)
{
	Utils::RemoveFromVector(&objects, object);
	delete object;
}
