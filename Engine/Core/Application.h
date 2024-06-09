#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <GLFW/glfw3.h>
#include <DirectXMath.h>
#include <functional>
#include <list>

#include "Layer.h"
#include "Scene.h"
#include "Utils.h"

using namespace DirectX;

class Application
{
	STATIC(bool fullscreen);
	STATIC(Vector2 lastWindowedSize);
	STATIC(Vector2 lastWindowedPos);

	STATIC(std::list<Layer*> layers);

	STATIC(double lastFrameTime);
	STATIC(double deltaTime);
	STATIC(double totalTime);

public:

	STATIC(float targetFrameRate);

	static void Init(Vector2 windowStartSize, Vector2 windowStartPos, const char* windowTitle,
		const std::filesystem::path& windowIconPathFromSolution, bool windowStartMaximized);
	static void Run();

	static Vector2 GetFramebufferSize();

	static Vector2 GetWindowSize();
	static Vector2 GetWindowPosition();

	static GETTER(GetDeltaTime, deltaTime);
	static GETTER(GetTotalTime, totalTime);

	static HWND GetWindowHandle();
	static std::filesystem::path GetApplicationPath();
	static void SetWindowIcon(const std::filesystem::path& path);

	static GETTER(GetFullscreen, fullscreen);
	static void SetFullscreen(bool fs);

	static void PushLayer(Layer* layer);
	static void PopLayer();

	static bool Closing();

	static inline GLFWwindow* window{};
};