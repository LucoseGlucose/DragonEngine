#pragma once

#include "Object.h"
#include "Transform.h"

class SceneObject;

class Component : public Object
{
	SceneObject* owner;

public:
	Component(SceneObject* owner);

	SceneObject* GetOwner();

	Transform* GetTransform();
	void SetTransform(Transform* transform);

	virtual void OnStart();
	virtual void OnUpdate();
	virtual void OnEnd();

	template<typename T>
	T* GetOwner() requires std::is_base_of_v<SceneObject, T>
	{
		T* casted = dynamic_cast<T*>(owner);
		return casted;
	}
};