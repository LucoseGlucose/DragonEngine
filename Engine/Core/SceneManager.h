#pragma once

#include "Utils.h"
#include "Scene.h"

class SceneManager
{
	STATIC(std::vector<Scene*> scenes);
	STATIC(uint32_t activeIndex);

public:

	static Scene* GetActiveScene();
	static void SetActiveScene(Scene* scene);
	static void SetActiveScene(uint32_t scene);

	static void AddScene(Scene* scene);
	static void RemoveScene(Scene* scene);

	static uint32_t GetSceneCount();
};