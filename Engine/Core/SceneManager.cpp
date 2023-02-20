#include "stdafx.h"
#include "SceneManager.h"

Scene* SceneManager::GetActiveScene()
{
	return scenes[activeIndex];
}

void SceneManager::SetActiveScene(Scene* scene)
{
	std::vector<Scene*>::iterator ptr = std::find(scenes.begin(), scenes.end(), scene);
	if (ptr == scenes.end()) return;

	uint32_t index = std::distance(scenes.begin(), ptr);

	if (activeIndex == index) return;
	GetActiveScene()->OnEnd();

	activeIndex = index;
	GetActiveScene()->OnStart();
}

void SceneManager::SetActiveScene(uint32_t scene)
{
	uint32_t index = std::clamp<uint32_t>(scene, 0, scenes.size() - 1);
	if (activeIndex == index) return;

	GetActiveScene()->OnEnd();

	activeIndex = index;
	GetActiveScene()->OnStart();
}

void SceneManager::AddScene(Scene* scene)
{
	scenes.push_back(scene);
}

void SceneManager::RemoveScene(Scene* scene)
{
	if (GetActiveScene() == scene) Utils::CrashWithMessage(L"Cannot remove active scene!");
	Utils::RemoveFromVector(&scenes, scene);
}

uint32_t SceneManager::GetSceneCount()
{
	return scenes.size();
}
