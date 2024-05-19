#pragma once

#include <filesystem>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

#define SETTING inline const

namespace Settings
{
	SETTING std::filesystem::path iconPathFromSolution = "Engine/Images/DragonEngine Logo.png";
}