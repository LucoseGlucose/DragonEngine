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

D3D12_SAMPLER_DESC Utils::GetDefaultSampler()
{
	D3D12_SAMPLER_DESC sampler{};
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.MaxAnisotropy = 8;
	sampler.MinLOD = 0;
	sampler.MaxLOD = 16;

	return sampler;
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

uint32_t Utils::GetMipCount(uint32_t width, uint32_t height)
{
	uint32_t highBit;
	_BitScanReverse((unsigned long*)&highBit, width | height);
	return highBit + 1;
}

DXGI_FORMAT Utils::GetSRGBFormat(DXGI_FORMAT linear)
{
	switch (linear)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	case DXGI_FORMAT_BC1_UNORM:
		return DXGI_FORMAT_BC1_UNORM_SRGB;
	case DXGI_FORMAT_BC2_UNORM:
		return DXGI_FORMAT_BC2_UNORM_SRGB;
	case DXGI_FORMAT_BC3_UNORM:
		return DXGI_FORMAT_BC3_UNORM_SRGB;
	case DXGI_FORMAT_B8G8R8A8_UNORM:
		return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	case DXGI_FORMAT_B8G8R8X8_UNORM:
		return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
	case DXGI_FORMAT_BC7_UNORM:
		return DXGI_FORMAT_BC7_UNORM_SRGB;
	default:
		break;
	}

	return linear;
}

DXGI_FORMAT Utils::GetLinearFormat(DXGI_FORMAT srgb)
{
	switch (srgb)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case DXGI_FORMAT_BC1_UNORM_SRGB:
		return DXGI_FORMAT_BC1_UNORM;
	case DXGI_FORMAT_BC2_UNORM_SRGB:
		return DXGI_FORMAT_BC2_UNORM;
	case DXGI_FORMAT_BC3_UNORM_SRGB:
		return DXGI_FORMAT_BC3_UNORM;
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return DXGI_FORMAT_B8G8R8X8_UNORM;
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return DXGI_FORMAT_BC7_UNORM;
	default:
		break;
	}

	return srgb;
}

bool Utils::IsFormatSRGB(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return true;
	case DXGI_FORMAT_BC1_UNORM_SRGB:
		return true;
	case DXGI_FORMAT_BC2_UNORM_SRGB:
		return true;
	case DXGI_FORMAT_BC3_UNORM_SRGB:
		return true;
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		return true;
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return true;
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		return true;
	default:
		break;
	}

	return false;
}

std::function<void(RendererComponent*)> Utils::GetLitShaderParamFunc()
{
	return [](RendererComponent* renderer)
	{
		XMFLOAT4X4 projMat = Rendering::outputCam->GetProjectionMat();
		XMFLOAT4X4 viewMat = Rendering::outputCam->GetViewMat();
		XMFLOAT4X4 modelMat = renderer->GetTransform()->GetMatrix();

		XMMATRIX modelMatMat = DirectX::XMLoadFloat4x4(&modelMat);
		XMMATRIX multiplied = modelMatMat * DirectX::XMLoadFloat4x4(&viewMat) * DirectX::XMLoadFloat4x4(&projMat);

		XMFLOAT4X4 mvpMat;
		DirectX::XMStoreFloat4x4(&mvpMat, DirectX::XMMatrixTranspose(multiplied));

		renderer->material->SetParameter("p_mvpMat", &mvpMat, sizeof(mvpMat));

		XMFLOAT4X4 shaderModelMat;
		DirectX::XMStoreFloat4x4(&shaderModelMat, DirectX::XMMatrixTranspose(modelMatMat));

		renderer->material->SetParameter("p_modelMat", &shaderModelMat, sizeof(shaderModelMat));

		XMFLOAT3 cameraPosition = Rendering::outputCam->GetTransform()->GetPosition();
		renderer->material->SetParameter("p_cameraPosition", &cameraPosition, sizeof(XMFLOAT3));

		std::vector<LightComponent*> lights = *Rendering::lights;
		std::map<LightComponent*, float> distances{};

		for (size_t i = 0; i < lights.size(); i++)
		{
			distances[lights[i]] = lights[i]->GetTransform()->GetDistance(renderer->GetTransform(), true);
		}

		std::sort(lights.begin(), lights.end(), [distances](LightComponent* l1, LightComponent* l2)
			{ return distances.at(l1) < distances.at(l2); });

		LightData lightData[5]{};
		for (size_t i = 0; i < 5; i++)
		{
			LightData data{};

			if (i >= lights.size())
			{
				data.color = XMFLOAT3(0.f, 0.f, 0.f);

				lightData[i] = data;
				continue;
			}

			LightComponent* light = lights[i];
			light->GetLightData(&data);

			lightData[i] = data;
		}

		renderer->material->SetParameter("p_lights", lightData, sizeof(lightData));
	};
}
