#pragma once

#include <DirectXMath.h>

#ifdef _DEBUG
#define NAME_D3D_OBJECT(obj) Utils::ThrowIfFailed(obj->SetName(L#obj))
#else
#define NAME_D3D_OBJECT(obj)
#endif

class Utils
{
public:
	static void ThrowIfFailed(HRESULT result);
	static void ThrowIfFailed(HRESULT result, LPCWSTR message);
	static void CrashWithMessage(LPCWSTR message);

	static DirectX::XMFLOAT3 QuatToEulerAngles(DirectX::XMFLOAT4 quat);

	static bool SamplersEqual(D3D12_SAMPLER_DESC& s1, D3D12_SAMPLER_DESC& s2);
	static D3D12_SAMPLER_DESC GetDefaultSampler();

	static std::filesystem::path GetPathFromExe(std::filesystem::path path);
	static std::filesystem::path GetPathFromProject(std::filesystem::path path);
	static std::filesystem::path GetPathFromSolution(std::filesystem::path path);

	static uint32_t GetMipCount(uint32_t width, uint32_t height);

	static DXGI_FORMAT GetSRGBFormat(DXGI_FORMAT linear);
	static DXGI_FORMAT GetLinearFormat(DXGI_FORMAT srgb);
	static bool IsFormatSRGB(DXGI_FORMAT format);

	template<typename T>
	static void RemoveFromVector(std::vector<T>* vec, const T& item)
	{
		vec->erase(std::find(vec->begin(), vec->end(), item));
	}
};