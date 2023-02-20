#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <wrl.h>
#include <process.h>

#include <DirectXMath.h>
#include <DirectXColors.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12.h"

#include <exception>
#include <string>
#include <vector>
#include <array>
#include <filesystem>
#include <map>
#include <optional>
#include <tuple>
#include <queue>
#include <functional>
#include <random>

#include "Utils.h"
#include "Application.h"
#include "TimeManager.h"

using namespace Microsoft::WRL;
using namespace DirectX;