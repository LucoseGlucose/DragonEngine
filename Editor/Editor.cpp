#include "stdafx.h"

#include "Application.h"
#include "SceneManager.h"
#include "EditorLayer.h"
#include "SceneLayer.h"
#include "Game.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	Application::Init();
	Application::PushLayer(new EditorLayer());

	SceneManager::AddScene(Game::Init());
	Application::Run();
}