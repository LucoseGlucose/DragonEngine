#pragma once

#include <DirectXMath.h>

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

	template<typename T>
	static void RemoveFromVector(std::vector<T>* vec, const T& item)
	{
		vec->erase(std::find(vec->begin(), vec->end(), item));
	}
};