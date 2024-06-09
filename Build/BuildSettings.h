#pragma once

#include <filesystem>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

#define SETTING inline const

namespace Settings
{
	SETTING std::filesystem::path windowIconPathFromSolution = "Engine/Images/DragonEngine Logo.png";
	SETTING Vector2 windowStartSize = Vector2(1280, 720);
	SETTING Vector2 windowStartPos = Vector2(320, 180);
	SETTING char* windowTitle = "DragonEngine";
	SETTING bool windowStartMaximized = false;
}