#include "stdafx.h"
#include "PresentationBuffer.h"

#include "Rendering.h"
#include <GLFW/glfw3.h>

PresentationBuffer::PresentationBuffer() : colorTextures(), pipelineProfile()//, depthTexture(nullptr)
{
	Vector2 windowSize = Application::GetFramebufferSize();

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsDesc{};
	fsDesc.Windowed = true;

	DXGI_MODE_DESC modeDesc{};
	modeDesc.Width = windowSize.x;
	modeDesc.Height = windowSize.y;
	modeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	DXGI_SAMPLE_DESC sampleDesc{};
	sampleDesc.Count = 1;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = windowSize.x;
	swapChainDesc.Height = windowSize.y;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = Settings::numPresentationFrames;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swapChainDesc.Flags = 0;

	ComPtr<IDXGISwapChain1> tempSC;
	Utils::ThrowIfFailed(Rendering::factory->CreateSwapChainForHwnd(Rendering::commandQueue->commandQueue.Get(), Application::GetWindowHandle(),
		&swapChainDesc, &fsDesc, nullptr, tempSC.GetAddressOf()));

	Utils::ThrowIfFailed(tempSC.As<IDXGISwapChain3>(&swapchain));

	rtvDescHeap = DescriptorHeap(Settings::numPresentationFrames, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
	//dsDescHeap = DescriptorHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

	for (size_t i = 0; i < Settings::numPresentationFrames; i++)
	{
		ID3D12Resource* res;
		Utils::ThrowIfFailed(swapchain->GetBuffer(i, IID_PPV_ARGS(&res)));

		colorTextures[i] = new ColorTexture(&res, CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, &clearColor.x));
		colorTextures[i]->currentState = D3D12_RESOURCE_STATE_PRESENT;

		Rendering::device->CreateRenderTargetView(colorTextures[i]->resourceBuffer.Get(),
			&colorTextures[i]->rtvDesc, rtvDescHeap.GetCPUHandleForIndex(i));
	}

	pipelineProfile.rtvFormats.push_back(DXGI_FORMAT_R8G8B8A8_UNORM);
	pipelineProfile.depthStencilState.DepthEnable = false;
}

PresentationBuffer::~PresentationBuffer()
{
	for (size_t i = 0; i < colorTextures.size(); i++)
	{
		delete colorTextures[i];
	}
}

void PresentationBuffer::Resize(Vector2 newSize)
{
	if (newSize.x == 0 || newSize.y == 0) return;

	for (size_t i = 0; i < colorTextures.size(); i++)
	{
		colorTextures[i]->resourceBuffer->Release();
		colorTextures[i]->resourceBuffer.Reset();
	}

	Utils::ThrowIfFailed(swapchain->ResizeBuffers(Settings::numPresentationFrames, newSize.x, newSize.y, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	ID3D12Resource* tempRes = nullptr;

	for (size_t i = 0; i < colorTextures.size(); i++)
	{
		Utils::ThrowIfFailed(swapchain->GetBuffer(i, IID_PPV_ARGS(&tempRes)));
		colorTextures[i]->Resize(newSize, &tempRes);

		Rendering::device->CreateRenderTargetView(colorTextures[i]->resourceBuffer.Get(),
			&colorTextures[i]->rtvDesc, rtvDescHeap.GetCPUHandleForIndex(i));
	}
}

void PresentationBuffer::Setup(CommandRecorder* recorder)
{
	UINT32 frameIndex = swapchain->GetCurrentBackBufferIndex();

	recorder->list->RSSetViewports(1, &Rendering::viewport);
	recorder->list->RSSetScissorRects(1, &Rendering::scissorRect);
	recorder->list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Rendering::RecordBarriers(recorder, { colorTextures[frameIndex]->TransitionToState(D3D12_RESOURCE_STATE_RENDER_TARGET) });

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvCPUHandle = rtvDescHeap.GetCPUHandleForIndex(frameIndex);

	recorder->list->OMSetRenderTargets(1, &rtvCPUHandle, false, nullptr);
	recorder->list->ClearRenderTargetView(rtvCPUHandle, &clearColor.x, 0, nullptr);
}

void PresentationBuffer::Present(CommandRecorder* recorder)
{
	UINT32 frameIndex = swapchain->GetCurrentBackBufferIndex();
	Rendering::RecordBarriers(recorder, { colorTextures[frameIndex]->TransitionToState(D3D12_RESOURCE_STATE_PRESENT) });

	recorder->Execute();
	Utils::ThrowIfFailed(swapchain->Present(0, 0));
}
