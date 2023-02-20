#pragma once

#include "Object.h"

class SceneObject;

class Component : public Object
{
	SceneObject* owner;

public:
	Component(SceneObject* owner);

	SceneObject* GetOwner();

	virtual void OnStart();
	virtual void OnUpdate();
	virtual void OnEnd();
};