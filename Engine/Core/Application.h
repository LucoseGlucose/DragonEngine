#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <GLFW/glfw3.h>
#include <DirectXMath.h>
#include <functional>
#include "Scene.h"

using namespace DirectX;

class Application
{
public:
	static void Run(const std::function<Scene* (void)>& sceneCreateFunc);
	static XMINT2 GetFramebufferSize();
	static XMUINT2 GetUnsignedFramebufferSize();
	static HWND GetWindowHandle();
	static std::filesystem::path GetApplicationPath();

	static inline GLFWwindow* window{};
};