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
	Input::Update();
	SceneManager::GetActiveScene()->OnUpdate();
}

void SceneLayer::Resize(Vector2 newSize)
{

}

void SceneLayer::OnPop()
{
	Input::CleanUp();
	SceneManager::GetActiveScene()->OnEnd();
}