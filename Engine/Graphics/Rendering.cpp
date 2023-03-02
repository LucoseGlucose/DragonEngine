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
	CommandRecorder* rec = cmdRecorders.front();
	cmdRecorders.pop();
	return rec;
}

void Rendering::RecycleRecorder(CommandRecorder* recorder)
{
	if (recorder->IsRecording()) recorder->Execute();
	cmdRecorders.push(recorder);
}

void Rendering::Init()
{
	UINT dxgiFactoryFlags = 0;

#if _DEBUG
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
	bool adapterFound = false;

	while (factory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 adapterDesc{};
		adapter->GetDesc1(&adapterDesc);

		if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			adapterIndex++;
			continue;
		}

		HRESULT result = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(result))
		{
			adapterFound = true;
			break;
		}

		adapterIndex++;
	}

	Utils::ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));
	NAME_D3D_OBJECT(device);

	commandQueue = new CommandQueue();
	presentationBuffer = new PresentationBuffer();

	CD3DX12_CLEAR_VALUE rtClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black.f);
	CD3DX12_CLEAR_VALUE dsClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	sceneFB = new Framebuffer(Application::GetUnsignedFramebufferSize(), DXGI_FORMAT_R16G16B16A16_FLOAT,
		DXGI_FORMAT_D32_FLOAT, rtClear, dsClear, 4);

	postFB = new Framebuffer(Application::GetUnsignedFramebufferSize(), DXGI_FORMAT_R16G16B16A16_FLOAT,
		DXGI_FORMAT_D32_FLOAT, rtClear, dsClear, 1);

	for (size_t i = 0; i < 32; i++)
	{
		CommandRecorder* rec = new CommandRecorder();
		cmdRecorders.push(rec);
	}

	Utils::ThrowIfFailed(factory->MakeWindowAssociation(Application::GetWindowHandle(), DXGI_MWA_NO_ALT_ENTER));
	XMUINT2 windowSize = Application::GetUnsignedFramebufferSize();

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = windowSize.x;
	viewport.Height = windowSize.y;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = windowSize.x;
	scissorRect.bottom = windowSize.y;

	quadMesh = new Mesh(Utils::GetPathFromProject("Models/Quad.fbx"));

	outputObj = (new SceneObject("Render Output"))->AddComponent<RendererComponent>();
	outputObj->mesh = quadMesh;

	outputObj->material = new Material(ShaderProgram::Create(Utils::GetPathFromExe("OutputVertex.cso"),
		Utils::GetPathFromExe("OutputPixel.cso"), 1, DXGI_FORMAT_R8G8B8A8_UNORM));

	outputObj->material->SetTexture("t_sceneTexture", postFB->colorTexture);
}

void Rendering::Render()
{
	XMUINT2 windowSize = Application::GetUnsignedFramebufferSize();
	if (windowSize.x == 0 || windowSize.y == 0) return;

	CommandRecorder* recorder = GetRecorder();
	recorder->StartRecording();

	sceneFB->Setup(recorder);

	std::vector<RendererComponent*>* renderers = SceneManager::GetActiveScene()->FindComponents<RendererComponent>();
	lights = SceneManager::GetActiveScene()->FindComponents<LightComponent>();

	for (size_t i = 0; i < renderers->size(); i++)
	{
		RendererComponent* renderer = renderers->at(i);
		renderer->Render(recorder);
	}
	delete renderers;

	sceneFB->Blit(recorder, postFB, true, DXGI_FORMAT_R16G16B16A16_FLOAT, false, DXGI_FORMAT_R32_FLOAT);

	recorder->StopRecording();
	commandQueue->Execute(recorder->list.Get());

	RecycleRecorder(recorder);
	recorder = GetRecorder();
	recorder->StartRecording();

	presentationBuffer->Setup(recorder);
	outputObj->Render(recorder);
	presentationBuffer->Present(recorder);

	RecycleRecorder(recorder);
	WaitForNextFrame();
}

void Rendering::Cleanup()
{
	commandQueue->WaitForAllCommands();

	while (!cmdRecorders.empty())
	{
		delete cmdRecorders.front();
		cmdRecorders.pop();
	}

	delete sceneFB;
	delete postFB;

	delete presentationBuffer;
	delete commandQueue;
}

void Rendering::Resize(XMUINT2 newSize)
{
	commandQueue->WaitForAllCommands();

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = newSize.x;
	viewport.Height = newSize.y;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = newSize.x;
	scissorRect.bottom = newSize.y;

	sceneFB->Resize(newSize);
	postFB->Resize(newSize);
	presentationBuffer->Resize(newSize);

	outputObj->material->SetTexture("t_sceneTexture", postFB->colorTexture);
	outputCam->CalculateProjection();
}

void Rendering::SetViewportSize(XMUINT2 size)
{
	viewport.Width = size.x;
	viewport.Height = size.y;

	scissorRect.right = size.x;
	scissorRect.bottom = size.y;
}

void Rendering::ResetViewportSize()
{
	XMUINT2 size = Application::GetUnsignedFramebufferSize();

	viewport.Width = size.x;
	viewport.Height = size.y;

	scissorRect.right = size.x;
	scissorRect.bottom = size.y;
}