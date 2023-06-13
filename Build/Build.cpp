#include "stdafx.h"

#include "Application.h"
#include "SceneManager.h"
#include "BuildLayer.h"
#include "SceneLayer.h"
#include "Game.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	Application::Init();
	Application::PushLayer(new BuildLayer());

	SceneManager::AddScene(Game::Init());

	Application::PushLayer(new SceneLayer());
	Application::Run();
}