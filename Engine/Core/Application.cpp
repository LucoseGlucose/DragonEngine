#include "stdafx.h"
#include "Application.h"

#include "Rendering.h"
#include "TimeManager.h"
#include "SceneManager.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

void Application::Run(const std::function<Scene*(void)>& sceneCreateFunc)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(1280, 720, "DragonEngine", nullptr, nullptr);
	glfwSetWindowPos(window, 320, 180);

	Rendering::Init();
	double lastFrameTime = 0;

	SceneManager::AddScene(sceneCreateFunc());
	SceneManager::GetActiveScene()->OnStart();

	while (!glfwWindowShouldClose(window))
	{
		TimeManager::Update(glfwGetTime() - lastFrameTime);
		lastFrameTime = glfwGetTime();

		XMUINT2 preWindowSize = GetUnsignedFramebufferSize();
		glfwPollEvents();

		XMUINT2 postWindowSize = GetUnsignedFramebufferSize();
		if (postWindowSize.x != preWindowSize.x || postWindowSize.y != preWindowSize.y) Rendering::Resize(postWindowSize);

		SceneManager::GetActiveScene()->OnUpdate();
		
		Rendering::Render();
		glfwSetWindowTitle(window, std::to_string((int)std::round(1.0 / TimeManager::GetDeltaTime())).append(" FPS").c_str());
	}

	SceneManager::GetActiveScene()->OnEnd();
	Rendering::Cleanup();

	glfwDestroyWindow(window);
	glfwTerminate();
}

XMINT2 Application::GetFramebufferSize()
{
	XMINT2 size{};
	glfwGetFramebufferSize(window, &size.x, &size.y);
	return size;
}

XMUINT2 Application::GetUnsignedFramebufferSize()
{
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	return XMUINT2(width, height);
}

HWND Application::GetWindowHandle()
{
	return glfwGetWin32Window(window);
}

std::filesystem::path Application::GetApplicationPath()
{
	char* buffer = new char[128];
	DWORD size = GetModuleFileNameA(nullptr, buffer, 128);

	char* path = new char[size + 1];
	GetModuleFileNameA(nullptr, path, size + 1);

	std::filesystem::path str = std::filesystem::path(path);

	delete[] buffer;
	delete[] path;

	return str;
}
