#include "stdafx.h"
#include "SceneLayer.h"

#include "Input.h"
#include "SceneManager.h"

void SceneLayer::OnPush()
{
	Input::Init();
	SceneManager::GetActiveScene()->OnStart();
}

void SceneLayer::Update()
{
	TimeManager::Update(glfwGetTime() - lastFrameTime);
	lastFrameTime = glfwGetTime();

	Input::Update();
	SceneManager::GetActiveScene()->OnUpdate();
}

void SceneLayer::Resize(XMUINT2 newSize)
{

}

void SceneLayer::OnPop()
{
	Input::CleanUp();
	SceneManager::GetActiveScene()->OnEnd();
}