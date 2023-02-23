#include "stdafx.h"
#include "PresentationBuffer.h"

#include "Rendering.h"
#include <GLFW/glfw3.h>

PresentationBuffer::PresentationBuffer()
{
	XMUINT2 windowSize = Application::GetUnsignedFramebufferSize();

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
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swapChainDesc.Flags = 0;

	ComPtr<IDXGISwapChain1> tempSC;
	Utils::ThrowIfFailed(Rendering::factory->CreateSwapChainForHwnd(Rendering::commandQueue->commandQueue.Get(), Application::GetWindowHandle(),
		&swapChainDesc, &fsDesc, nullptr, tempSC.GetAddressOf()));

	tempSC.As<IDXGISwapChain3>(&swapchain);

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.NumDescriptors = 2;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	Utils::ThrowIfFailed(Rendering::device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescHeap)));
	NAME_D3D_OBJECT(rtvDescHeap);

	D3D12_DESCRIPTOR_HEAP_DESC dsHeapDesc{};
	dsHeapDesc.NumDescriptors = 1;
	dsHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	Utils::ThrowIfFailed(Rendering::device->CreateDescriptorHeap(&dsHeapDesc, IID_PPV_ARGS(&dsDescHeap)));
	NAME_D3D_OBJECT(dsDescHeap);

	View rtvView{};
	rtvView.rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvView.rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	ID3D12Resource* res;
	Utils::ThrowIfFailed(swapchain->GetBuffer(0, IID_PPV_ARGS(&res)));
	colorTextures[0] = new RenderTexture(&res, rtvView);

	Utils::ThrowIfFailed(swapchain->GetBuffer(1, IID_PPV_ARGS(&res)));
	colorTextures[1] = new RenderTexture(&res, rtvView);

	UINT rtvHandleSize = Rendering::device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvDescHeap->GetCPUDescriptorHandleForHeapStart());

	Rendering::device->CreateRenderTargetView(colorTextures[0]->textureBuffer.Get(), &colorTextures[0]->descriptor.rtvDesc, rtvDescHandle);
	rtvDescHandle.Offset(1, rtvHandleSize);
	Rendering::device->CreateRenderTargetView(colorTextures[1]->textureBuffer.Get(), &colorTextures[1]->descriptor.rtvDesc, rtvDescHandle);

	View dsView{};
	dsView.dsDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsView.dsDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsView.dsDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE dsClearValue{};
	dsClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	dsClearValue.DepthStencil.Depth = 1.0f;
	dsClearValue.DepthStencil.Stencil = 0;

	depthStencilTexture = new RenderTexture(Application::GetUnsignedFramebufferSize(), DXGI_FORMAT_D32_FLOAT,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, dsClearValue, dsView, D3D12_RESOURCE_STATE_DEPTH_WRITE, 1);

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvDescHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsDescHeap->GetCPUDescriptorHandleForHeapStart());
	Rendering::device->CreateDepthStencilView(depthStencilTexture->textureBuffer.Get(), &depthStencilTexture->descriptor.dsDesc, dsvDescHandle);
}

PresentationBuffer::~PresentationBuffer()
{
	delete colorTextures[0];
	delete colorTextures[1];
	delete depthStencilTexture;
}

void PresentationBuffer::Resize(XMUINT2 newSize)
{
	if (newSize.x == 0 || newSize.y == 0) return;

	colorTextures[0]->textureBuffer->Release();
	colorTextures[1]->textureBuffer->Release();

	colorTextures[0]->textureBuffer.Reset();
	colorTextures[1]->textureBuffer.Reset();

	swapchain->ResizeBuffers(2, newSize.x, newSize.y, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	ID3D12Resource* tempRes = nullptr;

	Utils::ThrowIfFailed(swapchain->GetBuffer(0, IID_PPV_ARGS(&tempRes)));
	colorTextures[0]->Resize(newSize, &tempRes);

	Utils::ThrowIfFailed(swapchain->GetBuffer(1, IID_PPV_ARGS(&tempRes)));
	colorTextures[1]->Resize(newSize, &tempRes);

	depthStencilTexture->Resize(newSize, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	UINT rtvHandleSize = Rendering::device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvDescHeap->GetCPUDescriptorHandleForHeapStart());

	Rendering::device->CreateRenderTargetView(colorTextures[0]->textureBuffer.Get(), &colorTextures[0]->descriptor.rtvDesc, rtvDescHandle);
	rtvDescHandle.Offset(1, rtvHandleSize);
	Rendering::device->CreateRenderTargetView(colorTextures[1]->textureBuffer.Get(), &colorTextures[1]->descriptor.rtvDesc, rtvDescHandle);

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvDescHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsDescHeap->GetCPUDescriptorHandleForHeapStart());
	Rendering::device->CreateDepthStencilView(depthStencilTexture->textureBuffer.Get(), &depthStencilTexture->descriptor.dsDesc, dsvDescHandle);
}

void PresentationBuffer::Setup(CommandRecorder* recorder)
{
	uint32_t frameIndex = swapchain->GetCurrentBackBufferIndex();

	recorder->list->RSSetViewports(1, &Rendering::viewport);
	recorder->list->RSSetScissorRects(1, &Rendering::scissorRect);
	recorder->list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	CD3DX12_RESOURCE_BARRIER transitionToRT = CD3DX12_RESOURCE_BARRIER::Transition(
		colorTextures[frameIndex]->textureBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	recorder->list->ResourceBarrier(1, &transitionToRT);

	UINT handleSize = Rendering::device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
		rtvDescHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, handleSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsDescHeap->GetCPUDescriptorHandleForHeapStart());

	recorder->list->OMSetRenderTargets(1, &rtvCPUHandle, false, &dsCPUHandle);
	recorder->list->ClearRenderTargetView(rtvCPUHandle, &clearColor.x, 0, nullptr);
	recorder->list->ClearDepthStencilView(dsCPUHandle, D3D12_CLEAR_FLAG_DEPTH, clearDepth, 0, 0, nullptr);
}

void PresentationBuffer::Present(CommandRecorder* recorder)
{
	uint32_t frameIndex = swapchain->GetCurrentBackBufferIndex();

	CD3DX12_RESOURCE_BARRIER transitionToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
		colorTextures[frameIndex]->textureBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	recorder->list->ResourceBarrier(1, &transitionToPresent);

	recorder->Execute();
	Utils::ThrowIfFailed(swapchain->Present(0, 0));
}
