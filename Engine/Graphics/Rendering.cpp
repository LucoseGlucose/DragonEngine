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
	//std::lock_guard<std::mutex> lock = std::lock_guard<std::mutex>(recorderMutex);

	CommandRecorder* rec = cmdRecorders->front();
	cmdRecorders->pop();
	return rec;
}

void Rendering::RecycleRecorder(CommandRecorder* recorder)
{
	//std::lock_guard<std::mutex> lock = std::lock_guard<std::mutex>(recorderMutex);

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

	cmdRecorders = new std::queue<CommandRecorder*>();
	for (size_t i = 0; i < 32; i++)
	{
		CommandRecorder* rec = new CommandRecorder();
		cmdRecorders->push(rec);
	}

	Utils::ThrowIfFailed(factory->MakeWindowAssociation(Application::GetWindowHandle(), DXGI_MWA_NO_ALT_ENTER));
	XMUINT2 windowSize = Application::GetUnsignedFramebufferSize();

	viewport.MaxDepth = 1.f;
	ResetViewportSize();

	quadMesh = new Mesh(Utils::GetPathFromProject("Models/Quad.fbx"));

	outputObj = (new SceneObject("Render Output"))->AddComponent<RendererComponent>();
	outputObj->mesh = quadMesh;

	outputObj->material = new Material(ShaderProgram::Create(Utils::GetPathFromExe("OutputV.cso"),
		Utils::GetPathFromExe("OutputP.cso"), 1, DXGI_FORMAT_R8G8B8A8_UNORM));

	scenePass = new SceneRenderPass();
	resolvePass = new ResolveRenderPass();
	tonemapPass = new ProcessRenderPass(new Material(ShaderProgram::Create(Utils::GetPathFromExe("OutputV.cso"),
		Utils::GetPathFromExe("TonemapP.cso"), 1, DXGI_FORMAT_R16G16B16A16_FLOAT)));
	gammaPass = new ProcessRenderPass(new Material(ShaderProgram::Create(Utils::GetPathFromExe("OutputV.cso"),
		Utils::GetPathFromExe("GammaP.cso"), 1, DXGI_FORMAT_R16G16B16A16_FLOAT)));

	renderPasses = std::vector<RenderPass*>{ scenePass, resolvePass, tonemapPass, gammaPass };
	outputObj->material->SetTexture("t_inputTexture", renderPasses.back()->outputFB->colorTexture);
}

void Rendering::Render()
{
	XMUINT2 windowSize = Application::GetUnsignedFramebufferSize();
	if (windowSize.x == 0 || windowSize.y == 0) return;

	for (size_t i = 0; i < renderPasses.size(); i++)
	{
		renderPasses[i]->Render(i != 0 ? renderPasses[i - 1]->outputFB : nullptr);
	}

	CommandRecorder* recorder = GetRecorder();
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

	while (!cmdRecorders->empty())
	{
		delete cmdRecorders->front();
		cmdRecorders->pop();
	}

	delete presentationBuffer;
	delete commandQueue;
}

void Rendering::Resize(XMUINT2 newSize)
{
	commandQueue->WaitForAllCommands();

	SetViewportSize(newSize);
	outputCam->CalculateProjection();

	for (size_t i = 0; i < renderPasses.size(); i++)
	{
		renderPasses[i]->Resize(i != 0 ? renderPasses[i - 1]->outputFB : nullptr, newSize);
	}

	outputObj->material->UpdateTexture("t_inputTexture", renderPasses.back()->outputFB->colorTexture);
	presentationBuffer->Resize(newSize);
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