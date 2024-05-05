#pragma once

#include "SceneObject.h"
#include <string>
#include <functional>

class Scene : public Object
{
	std::vector<SceneObject*> objects;

public:
	Scene(std::string name);
	~Scene();

	void OnStart();
	void OnUpdate();
	void OnEnd();

	SceneObject* AddObject(SceneObject* object);
	void DestroyObject(SceneObject* object);

	template<typename T>
	T* FindObject() requires std::is_base_of_v<SceneObject, T>
	{
		for (size_t i = 0; i < objects.size(); i++)
		{
			T* casted = dynamic_cast<T*>(objects[i]);
			if (casted != nullptr) return casted;
		}

		return nullptr;
	}

	template<typename T>
	bool TryFindObject(T** out) requires std::is_base_of_v<SceneObject, T>
	{
		*out = FindObject<T>();
		return out == nullptr;
	}

	template<typename T>
	std::vector<T*> FindObjects() requires std::is_base_of_v<SceneObject, T>
	{
		std::vector<T*> vec = std::vector<T*>();

		for (size_t i = 0; i < objects.size(); i++)
		{
			T* casted = dynamic_cast<T*>(objects[i]);
			if (casted != nullptr) vec.push_back(casted);
		}

		return vec;
	}

	template<typename T>
	bool TryFindObjects(std::vector<T*>* out) requires std::is_base_of_v<SceneObject, T>
	{
		*out = FindObjects<T>();
		return !(*out->empty());
	}

	template<typename T>
	T* FindComponent() requires std::is_base_of_v<Component, T>
	{
		for (size_t i = 0; i < objects.size(); i++)
		{
			T* ptr = nullptr;
			if (objects[i]->TryGetComponent(&ptr)) return ptr;
		}

		return nullptr;
	}

	template<typename T>
	bool TryFindComponent(T** out) requires std::is_base_of_v<Component, T>
	{
		*out = FindComponent<T>();
		return out == nullptr;
	}

	template<typename T>
	std::vector<T*> FindComponents() requires std::is_base_of_v<Component, T>
	{
		std::vector<T*> vec = std::vector<T*>();

		for (size_t o = 0; o < objects.size(); o++)
		{
			std::vector<T*> tempVec = objects[o]->GetComponents<T>();
			for (size_t i = 0; i < tempVec.size(); i++)
			{
				vec.push_back(tempVec.at(i));
			}
		}

		return vec;
	}

	template<typename T>
	bool TryFindComponents(std::vector<T*>* out) requires std::is_base_of_v<Component, T>
	{
		*out = FindComponents<T>();
		return !(*out)->empty();
	}
};