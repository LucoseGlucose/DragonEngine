#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void Utils::ThrowIfFailed(HRESULT result)
{
	if (FAILED(result))
	{
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

XMFLOAT3 Utils::QuatToEulerAngles(XMFLOAT4 quat)
{
	float pitch = (float)std::asin(2 * quat.x * quat.y + 2 * quat.z * quat.w);
	float yaw = (float)std::atan2(2 * quat.y * quat.w - 2 * quat.x * quat.z, 1 - 2 * quat.y * quat.y - 2 * quat.z * quat.z);
	float roll = (float)std::atan2(2 * quat.x * quat.w - 2 * quat.y * quat.z, 1 - 2 * quat.x * quat.x - 2 * quat.z * quat.z);

	return XMFLOAT3(XMConvertToDegrees(pitch), XMConvertToDegrees(yaw), XMConvertToDegrees(roll));
}

bool Utils::SamplersEqual(D3D12_SAMPLER_DESC& s1, D3D12_SAMPLER_DESC& s2)
{
	return (int)s1.AddressU == (int)s2.AddressU && (int)s1.AddressV == (int)s2.AddressV && (int)s1.AddressW == (int)s2.AddressW
		&& s1.BorderColor == s2.BorderColor && (int)s1.ComparisonFunc == (int)s2.ComparisonFunc && (int)s1.Filter == (int)s2.Filter
		&& s1.MaxAnisotropy == s2.MaxAnisotropy && s1.MaxLOD == s2.MaxLOD && s1.MinLOD == s2.MinLOD && s1.MipLODBias == s2.MipLODBias;
}

D3D12_SAMPLER_DESC Utils::GetDefaultSampler()
{
	D3D12_SAMPLER_DESC sampler{};
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	sampler.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	sampler.MaxAnisotropy = 8;
	sampler.MinLOD = 0;
	sampler.MaxLOD = 0;

	return sampler;
}

std::filesystem::path Utils::GetPathFromExe(std::filesystem::path path)
{
	std::filesystem::path exePath = Application::GetApplicationPath();
	exePath.remove_filename();

	return exePath.append(path.string());
}

std::filesystem::path Utils::GetPathFromProject(std::filesystem::path path)
{
	std::filesystem::path exePath = Application::GetApplicationPath();
	exePath.remove_filename();

	std::filesystem::path projPath = std::filesystem::canonical(exePath.append("../../Engine/"));
	return projPath.append(path.string());
}

std::filesystem::path Utils::GetPathFromSolution(std::filesystem::path path)
{
	std::filesystem::path exePath = Application::GetApplicationPath();
	exePath.remove_filename();

	std::filesystem::path projPath = std::filesystem::canonical(exePath.append("../../"));
	return projPath.append(path.string());
}
