#include "stdafx.h"
#include "SceneObject.h"

SceneObject::SceneObject(std::string name) : Object(name)
{
	transform = new Transform(this);
}

SceneObject::~SceneObject()
{
	for (size_t i = 0; i < components.size(); i++)
	{
		delete components[i];
	}
	delete transform;
}

Transform* SceneObject::GetTransform()
{
	return transform;
}

void SceneObject::SetTransform(Transform* transform)
{
	*this->transform = *transform;
}

void SceneObject::OnStart()
{
	for (size_t i = 0; i < components.size(); i++)
	{
		components[i]->OnStart();
	}
}

void SceneObject::OnUpdate()
{
	for (size_t i = 0; i < components.size(); i++)
	{
		components[i]->OnUpdate();
	}
}

void SceneObject::OnEnd()
{
	for (size_t i = 0; i < components.size(); i++)
	{
		components[i]->OnEnd();
	}
}

void SceneObject::RemoveComponent(Component* comp)
{
	Utils::RemoveFromVector(&components, comp);
	delete comp;
}
