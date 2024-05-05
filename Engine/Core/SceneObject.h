#pragma once

#include "Component.h"
#include "Transform.h"

class SceneObject : public Object
{
	std::vector<Component*> components;
	Transform* transform;

public:
	SceneObject(std::string name);
	~SceneObject();

	Transform* GetTransform();
	void SetTransform(Transform* transform);

	virtual void OnStart();
	virtual void OnUpdate();
	virtual void OnEnd();

	void RemoveComponent(Component* comp);

	template<typename T>
	T* AddComponent() requires std::is_base_of_v<Component, T>
	{
		T* ptr = new T(this);
		Component* pComp = dynamic_cast<Component*>(ptr);

		if (pComp == nullptr)
		{
			delete ptr;
			return nullptr;
		}

		components.push_back(pComp);
		return ptr;
	}

	template<typename T>
	T* GetComponent() requires std::is_base_of_v<Component, T>
	{
		for (size_t i = 0; i < components.size(); i++)
		{
			T* casted = dynamic_cast<T*>(components[i]);
			if (casted != nullptr) return casted;
		}

		return nullptr;
	}

	template<typename T>
	bool TryGetComponent(T** out) requires std::is_base_of_v<Component, T>
	{
		*out = GetComponent<T>();
		return *out == nullptr;
	}

	template<typename T>
	std::vector<T*> GetComponents() requires std::is_base_of_v<Component, T>
	{
		std::vector<T*> vec = std::vector<T*>();

		for (size_t i = 0; i < components.size(); i++)
		{
			Component* pComp = components[i];
			T* casted = dynamic_cast<T*>(pComp);

			if (casted != nullptr) vec.push_back(casted);
		}

		return vec;
	}

	template<typename T>
	bool TryGetComponents(std::vector<T*>* out) requires std::is_base_of_v<Component, T>
	{
		*out = GetComponents<T>();
		return !(*out)->empty();
	}
};