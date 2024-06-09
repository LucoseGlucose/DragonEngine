#include "stdafx.h"
#include "EditorSettings.h"

#include "Application.h"
#include "SceneManager.h"
#include "EditorLayer.h"
#include "SceneLayer.h"
#include "Game.h"
#include "Rendering.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	Application::Init(Vector2(1280, 720), Vector2(320, 180), "DragonEditor",
		"Engine/Images/DragonEngine Logo.png", true);

	Rendering::Init();
	Application::PushLayer(new EditorLayer());

	SceneManager::AddScene(Game::Init());
	Application::Run();
}