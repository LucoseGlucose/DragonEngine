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
	static inline bool fullscreen{};
	static inline XMINT2 lastWindowedSize{};
	static inline XMINT2 lastWindowedPos{};

public:
	static void Run(const std::function<Scene* (void)>& sceneCreateFunc);

	static XMINT2 GetFramebufferSize();
	static XMUINT2 GetUnsignedFramebufferSize();

	static XMINT2 GetWindowSize();
	static XMINT2 GetWindowPosition();

	static HWND GetWindowHandle();
	static std::filesystem::path GetApplicationPath();

	static bool GetFullscreen();
	static void SetFullscreen(bool fs);

	static inline GLFWwindow* window{};
};