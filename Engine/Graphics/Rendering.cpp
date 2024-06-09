#include "stdafx.h"
#include "Rendering.h"

#include "SceneManager.h"

void Rendering::WaitForNextFrame()
{
	uint8_t frameIndex = presentationBuffer->swapchain->GetCurrentBackBufferIndex();
	const UINT64 currentFenceValue = fenceValues[frameIndex];

	commandQueue->WaitToValue(currentFenceValue);
	fenceValues[frameIndex] = currentFenceValue + 1;
}

CommandRecorder* Rendering::GetRecorder()
{
	CommandRecorder* rec = cmdRecorders->front();
	cmdRecorders->pop();
	return rec;
}

void Rendering::RecycleRecorder(CommandRecorder* recorder)
{
	if (recorder->IsRecording()) recorder->Execute();
	cmdRecorders->push(recorder);
}

void Rendering::Init()
{
	UINT dxgiFactoryFlags = 0;

#ifdef _DEBUG
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif

	Utils::ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	int adapterIndex = 0;
	ComPtr<IDXGIAdapter1> tempAdapter;
	SIZE_T mostVideoMem = 0;

	while (factory->EnumAdapters1(adapterIndex, &tempAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 adapterDesc{};
		tempAdapter->GetDesc1(&adapterDesc);

		if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			adapterIndex++;
			continue;
		}

		HRESULT result = D3D12CreateDevice(tempAdapter.Get(), D3D_FEATURE_LEVEL_12_2, _uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(result))
		{
			if (adapterDesc.DedicatedVideoMemory > mostVideoMem)
			{
				mostVideoMem = adapterDesc.DedicatedVideoMemory;
				adapter = tempAdapter;
			}
		}

		adapterIndex++;
	}

	Utils::ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&device)));
	NAME_D3D_OBJECT(device);

	commandQueue = new CommandQueue();
	presentationBuffer = new PresentationBuffer();

	cmdRecorders = new std::queue<CommandRecorder*>();
	for (size_t i = 0; i < Settings::numCommandRecorders; i++)
	{
		CommandRecorder* rec = new CommandRecorder();
		cmdRecorders->push(rec);
	}

	Utils::ThrowIfFailed(factory->MakeWindowAssociation(Application::GetWindowHandle(), DXGI_MWA_NO_ALT_ENTER));
	Vector2 windowSize = Application::GetFramebufferSize();

	viewport.MaxDepth = 1.f;
	ResetViewportSize();

	quadMesh = new Mesh(Utils::GetPathFromProject("Models/Quad.fbx"));

	missingMaterial = new Material(ShaderProgram::Create(Utils::GetPathFromExe("UnlitV.cso"), Utils::GetPathFromExe("UnlitP.cso")));
	missingMaterial->SetParameter("p_albedo", Vector3(1.f, 0.f, 1.f));

	outputObj = (new SceneObject("Render Output"))->AddComponent<RendererComponent>();
	outputObj->SetMesh(quadMesh);

	outputObj->SetMaterial(new Material(ShaderProgram::Create(Utils::GetPathFromExe("OutputV.cso"), Utils::GetPathFromExe("OutputP.cso"))));

	scenePass = new SceneRenderPass();
	resolvePass = new ResolveRenderPass();

	tonemapPass = new ProcessRenderPass(new Material(ShaderProgram::Create(
		Utils::GetPathFromExe("OutputV.cso"), Utils::GetPathFromExe("TonemapP.cso"))));
	tonemapPass->material->SetParameter("p_tonemappingMode", 1);

	gammaPass = new ProcessRenderPass(new Material(ShaderProgram::Create(
		Utils::GetPathFromExe("OutputV.cso"), Utils::GetPathFromExe("GammaP.cso"))));

	renderPasses = std::vector<RenderPass*>{ scenePass, resolvePass, tonemapPass, gammaPass };
	outputObj->GetMaterial()->SetTexture("t_inputTexture", renderPasses.back()->outputFB->colorTextures.front());
}

void Rendering::Render()
{
	Vector2 windowSize = Application::GetFramebufferSize();
	if (windowSize.x == 0 || windowSize.y == 0) return;

	for (size_t i = 0; i < renderPasses.size(); i++)
	{
		renderPasses[i]->Render(i != 0 ? renderPasses[i - 1]->outputFB : nullptr);
	}

	CommandRecorder* recorder = GetRecorder();
	recorder->StartRecording();

	ResetViewportSize();
	presentationBuffer->Setup(recorder);

	outputObj->Render(recorder, presentationBuffer->pipelineProfile);
	presentationBuffer->Present(recorder);

	RecycleRecorder(recorder);
	WaitForNextFrame();
}

void Rendering::Cleanup()
{
	commandQueue->WaitForAllCommands();

	while (!cmdRecorders->empty())
	{
		delete cmdRecorders->front();
		cmdRecorders->pop();
	}

	delete presentationBuffer;
	delete commandQueue;
}

void Rendering::Resize(Vector2 newSize)
{
	commandQueue->WaitForAllCommands();

	SetViewportSize(newSize);
	outputCam->SetSize(newSize);

	for (size_t i = 0; i < renderPasses.size(); i++)
	{
		renderPasses[i]->Resize(i != 0 ? renderPasses[i - 1]->outputFB : nullptr, newSize);
	}

	outputObj->GetMaterial()->UpdateTexture("t_inputTexture", renderPasses.back()->outputFB->colorTextures.front());
	presentationBuffer->Resize(newSize);
}

void Rendering::SetViewportSize(Vector2 size)
{
	viewport.Width = size.x;
	viewport.Height = size.y;

	scissorRect.right = size.x;
	scissorRect.bottom = size.y;
}

void Rendering::ResetViewportSize()
{
	SetViewportSize(Application::GetFramebufferSize());
}

D3D12_SAMPLER_DESC Rendering::GetDefaultSampler()
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

D3D12_SAMPLER_DESC Rendering::GetBRDFSampler()
{
	D3D12_SAMPLER_DESC sampler{};
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.MaxAnisotropy = 0;
	sampler.MinLOD = 0;
	sampler.MaxLOD = 0;

	return sampler;
}

UINT32 Rendering::GetMipCount(UINT32 width, UINT32 height)
{
	UINT32 highBit;
	_BitScanReverse((unsigned long*)&highBit, width | height);
	return highBit + 1;
}

DXGI_FORMAT Rendering::GetSRGBFormat(DXGI_FORMAT linear)
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

DXGI_FORMAT Rendering::GetLinearFormat(DXGI_FORMAT srgb)
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

bool Rendering::IsFormatSRGB(DXGI_FORMAT format)
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

UINT8 Rendering::GetColorMaskForIndex(UINT8 index)
{
	return 0b10000000 >> index;
}

DXGI_FORMAT Rendering::GetResolveFormatForDepth(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_D32_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	default:
		break;
	}
}

void Rendering::RecordBarriers(CommandRecorder* recorder, std::initializer_list<CD3DX12_RESOURCE_BARRIER> barriers)
{
	recorder->list->ResourceBarrier(barriers.size(), barriers.begin());
}

CD3DX12_BLEND_DESC Rendering::GetDefaultBlendState()
{
	return CD3DX12_BLEND_DESC(D3D12_DEFAULT);
}

CD3DX12_RASTERIZER_DESC Rendering::GetDefaultRasterizerState()
{
	CD3DX12_RASTERIZER_DESC desc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	desc.MultisampleEnable = true;

	return desc;
}

CD3DX12_DEPTH_STENCIL_DESC Rendering::GetDefaultDepthStencilState()
{
	CD3DX12_DEPTH_STENCIL_DESC desc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	return desc;
}

DXGI_SAMPLE_DESC Rendering::GetDefaultSampleDesc()
{
	return { 1, 0 };
}
