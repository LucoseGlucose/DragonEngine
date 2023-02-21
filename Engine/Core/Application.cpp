#include "stdafx.h"
#include "Application.h"

#include "Rendering.h"
#include "TimeManager.h"
#include "SceneManager.h"
#include "Input.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

void Application::Run(const std::function<Scene*(void)>& sceneCreateFunc)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(1280, 720, "DragonEngine", nullptr, nullptr);
	glfwSetWindowPos(window, 320, 180);

	Input::Init();
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
		Input::Update();

		XMUINT2 postWindowSize = GetUnsignedFramebufferSize();
		if (postWindowSize.x != preWindowSize.x || postWindowSize.y != preWindowSize.y) Rendering::Resize(postWindowSize);

		if (Input::GetKeyDown(GLFW_KEY_F11)) SetFullscreen(!fullscreen);

		SceneManager::GetActiveScene()->OnUpdate();
		
		Rendering::Render();
	}

	SetFullscreen(false);

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

XMINT2 Application::GetWindowSize()
{
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	return XMINT2(width, height);
}

XMINT2 Application::GetWindowPosition()
{
	int x, y;
	glfwGetWindowPos(window, &x, &y);
	return XMINT2(x, y);
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

bool Application::GetFullscreen()
{
	return fullscreen;
}

void Application::SetFullscreen(bool fs)
{
	if (fs && !fullscreen)
	{
		lastWindowedSize = GetWindowSize();
		lastWindowedPos = GetWindowPosition();

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* vidMode = glfwGetVideoMode(monitor);

		glfwSetWindowMonitor(window, monitor, 0, 0, vidMode->width, vidMode->height, GLFW_DONT_CARE);

		Rendering::Resize(GetUnsignedFramebufferSize());
		fullscreen = true;
	}
	else if (!fs && fullscreen)
	{
		glfwSetWindowMonitor(window, nullptr, lastWindowedPos.x, lastWindowedPos.y, lastWindowedSize.x, lastWindowedSize.y, GLFW_DONT_CARE);

		Rendering::Resize(GetUnsignedFramebufferSize());
		fullscreen = false;
	}
}
