#pragma once

#include <filesystem>

#include "SimpleMath.h"

using namespace DirectX::SimpleMath;

#define SETTING inline const

namespace Settings
{
	SETTING UINT8 numPresentationFrames = 2;
	SETTING std::filesystem::path skyboxImagePathFromProject = "Images/citrus_orchard_4k.hdr";
	SETTING UINT8 numCommandRecorders = 32;
}