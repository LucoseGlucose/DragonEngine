#include "stdafx.h"
#include "Application.h"

#include "Rendering.h"
#include "SceneManager.h"
#include "Input.h"
#include "stb_image.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

void Application::Init(Vector2 windowStartSize, Vector2 windowStartPos, const char* windowTitle,
	const std::filesystem::path& windowIconPathFromSolution, bool windowStartMaximized)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(windowStartSize.x, windowStartSize.y, windowTitle, nullptr, nullptr);
	glfwSetWindowPos(window, windowStartPos.x, windowStartPos.y);

	SetWindowIcon(Utils::GetPathFromSolution(windowIconPathFromSolution));

	glfwSetWindowAttrib(window, GLFW_ICONIFIED, windowStartMaximized);
}

void Application::Run()
{
	while (!glfwWindowShouldClose(window))
	{
		Vector2 preWindowSize = GetFramebufferSize();
		glfwPollEvents();

		if (targetFrameRate != 0)
		{
			double targetDeltaTime = 1.0 / targetFrameRate;
			if (deltaTime < targetDeltaTime) Sleep((targetDeltaTime - deltaTime) * 1000);
		}

		deltaTime = glfwGetTime() - lastFrameTime;

		lastFrameTime = glfwGetTime();
		totalTime += deltaTime;

		Vector2 postWindowSize = GetFramebufferSize();
		if (postWindowSize.x != preWindowSize.x || postWindowSize.y != preWindowSize.y)
		{
			for (Layer*& l : layers)
			{
				l->Resize(postWindowSize);
			}
		}

		for (Layer*& l : layers)
		{
			l->Update();
		}

		if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) == GLFW_FALSE) Rendering::Render();
	}

	for (Layer*& l : layers)
	{
		l->OnPop();
		delete l;
	}
	layers.clear();

	Rendering::Cleanup();

	glfwDestroyWindow(window);
	glfwTerminate();
}

Vector2 Application::GetFramebufferSize()
{
	int x;
	int y;
	glfwGetFramebufferSize(window, &x, &y);

	return Vector2(x, y);
}

Vector2 Application::GetWindowSize()
{
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	return Vector2(width, height);
}

Vector2 Application::GetWindowPosition()
{
	int x, y;
	glfwGetWindowPos(window, &x, &y);
	return Vector2(x, y);
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

void Application::SetWindowIcon(const std::filesystem::path& path)
{
	int width;
	int height;
	int channels;
	unsigned char* pixels = stbi_load(path.string().c_str(), &width, &height, &channels, 4);

	GLFWimage icon;
	icon.pixels = pixels;
	icon.width = width;
	icon.height = height;

	glfwSetWindowIcon(window, 1, &icon);
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

		Rendering::Resize(GetFramebufferSize());
		fullscreen = true;
	}
	else if (!fs && fullscreen)
	{
		glfwSetWindowMonitor(window, nullptr, lastWindowedPos.x, lastWindowedPos.y, lastWindowedSize.x, lastWindowedSize.y, GLFW_DONT_CARE);

		Rendering::Resize(GetFramebufferSize());
		fullscreen = false;
	}
}

void Application::PushLayer(Layer* layer)
{
	layers.push_back(layer);
	layer->OnPush();
}

void Application::PopLayer()
{
	layers.back()->OnPop();
	layers.erase(layers.end());
}

bool Application::Closing()
{
	return glfwWindowShouldClose(window);
}
