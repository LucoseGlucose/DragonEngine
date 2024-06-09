#include "stdafx.h"
#include "Utils.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Rendering.h"
#include "RendererComponent.h"
#include "LightComponent.h"

void Utils::ThrowIfFailed(HRESULT result)
{
	if (FAILED(result))
	{
		if (result == DXGI_ERROR_DEVICE_REMOVED)
		{
			HRESULT reason = Rendering::device->GetDeviceRemovedReason();
			wchar_t outString[100];
			size_t size = 100;
			swprintf_s(outString, size, L"Device removed! DXGI_ERROR code: 0x%X\n", reason);
			OutputDebugStringW(outString);
		}

		throw std::exception();
	}
}

void Utils::ThrowIfFailed(HRESULT result, LPCWSTR message)
{
	if (FAILED(result))
	{
		CrashWithMessage(message);
	}
}

void Utils::CrashWithMessage(LPCWSTR message)
{
	MessageBeep(MB_ICONSTOP);
	MessageBox(Application::GetWindowHandle(), message, L"Error", MB_ICONERROR | MB_OK);
	throw std::exception();
}

std::filesystem::path Utils::GetPathFromExe(std::filesystem::path path)
{
	std::filesystem::path exePath = Application::GetApplicationPath();
	exePath.remove_filename();

	return std::filesystem::canonical(exePath.append(path.string()));
}

std::filesystem::path Utils::GetPathFromProject(std::filesystem::path path)
{
	std::filesystem::path exePath = Application::GetApplicationPath();
	exePath.remove_filename();

	std::filesystem::path projPath = std::filesystem::canonical(exePath.append("../../Engine/"));
	return std::filesystem::canonical(projPath.append(path.string()));
}

std::filesystem::path Utils::GetPathFromSolution(std::filesystem::path path)
{
	std::filesystem::path exePath = Application::GetApplicationPath();
	exePath.remove_filename();

	std::filesystem::path projPath = std::filesystem::canonical(exePath.append("../../"));
	return std::filesystem::canonical(projPath.append(path.string()));
}
