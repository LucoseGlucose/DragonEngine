#include "stdafx.h"
#include "Framebuffer.h"

#include "Rendering.h"

Framebuffer::Framebuffer(XMUINT2 size, DXGI_FORMAT rtvFormat, DXGI_FORMAT dsFormat,
	D3D12_CLEAR_VALUE rtvClear, D3D12_CLEAR_VALUE dsClear, uint32_t samples)
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.NumDescriptors = 1;
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
	rtvView.rtvDesc.Format = rtvFormat;
	rtvView.rtvDesc.ViewDimension = samples < 2 ? D3D12_RTV_DIMENSION_TEXTURE2D : D3D12_RTV_DIMENSION_TEXTURE2DMS;

	colorTexture = new RenderTexture(size, rtvFormat,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, rtvClear, rtvView, D3D12_RESOURCE_STATE_RENDER_TARGET, samples);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvDescHeap->GetCPUDescriptorHandleForHeapStart());
	Rendering::device->CreateRenderTargetView(colorTexture->textureBuffer.Get(), &colorTexture->descriptor.rtvDesc, rtvDescHandle);

	View dsView{};
	dsView.dsDesc.Format = dsFormat;
	dsView.dsDesc.ViewDimension = samples < 2 ? D3D12_DSV_DIMENSION_TEXTURE2D : D3D12_DSV_DIMENSION_TEXTURE2DMS;
	dsView.dsDesc.Flags = D3D12_DSV_FLAG_NONE;

	depthStencilTexture = new RenderTexture(size, dsFormat,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, dsClear, dsView, D3D12_RESOURCE_STATE_DEPTH_WRITE, samples);

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvDescHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsDescHeap->GetCPUDescriptorHandleForHeapStart());
	Rendering::device->CreateDepthStencilView(depthStencilTexture->textureBuffer.Get(), &depthStencilTexture->descriptor.dsDesc, dsvDescHandle);
}

Framebuffer::~Framebuffer()
{
	delete colorTexture;
	delete depthStencilTexture;
}

void Framebuffer::Resize(XMUINT2 newSize)
{
	if (newSize.x == 0 || newSize.y == 0) return;

	colorTexture->Resize(newSize, D3D12_RESOURCE_STATE_RENDER_TARGET);
	depthStencilTexture->Resize(newSize, D3D12_RESOURCE_STATE_DEPTH_WRITE);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvDescHeap->GetCPUDescriptorHandleForHeapStart());
	Rendering::device->CreateRenderTargetView(colorTexture->textureBuffer.Get(), &colorTexture->descriptor.rtvDesc, rtvDescHandle);

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvDescHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(dsDescHeap->GetCPUDescriptorHandleForHeapStart());
	Rendering::device->CreateDepthStencilView(depthStencilTexture->textureBuffer.Get(), &depthStencilTexture->descriptor.dsDesc, dsvDescHandle);
}

void Framebuffer::Setup(CommandRecorder* recorder, bool clearTargets)
{
	recorder->list->RSSetViewports(1, &Rendering::viewport);
	recorder->list->RSSetScissorRects(1, &Rendering::scissorRect);
	recorder->list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUHandle = rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE dsCPUHandle = dsDescHeap->GetCPUDescriptorHandleForHeapStart();

	recorder->list->OMSetRenderTargets(1, &rtvCPUHandle, false, &dsCPUHandle);
	if (clearTargets) Clear(recorder, true, true);
}

void Framebuffer::Blit(CommandRecorder* recorder, Framebuffer* dest, bool color, DXGI_FORMAT colorFormat, bool depthStencil, DXGI_FORMAT dsFormat)
{
	if (colorTexture->samples < 2)
	{
		if (color)
		{
			CD3DX12_RESOURCE_BARRIER toCopySrc = CD3DX12_RESOURCE_BARRIER::Transition(colorTexture->textureBuffer.Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);

			CD3DX12_RESOURCE_BARRIER toCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(dest->colorTexture->textureBuffer.Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);

			CD3DX12_RESOURCE_BARRIER initialBarriers[2] = { toCopySrc, toCopyDest };
			recorder->list->ResourceBarrier(2, initialBarriers);

			recorder->list->CopyResource(dest->colorTexture->textureBuffer.Get(), colorTexture->textureBuffer.Get());

			CD3DX12_RESOURCE_BARRIER fromSrcToRT = CD3DX12_RESOURCE_BARRIER::Transition(colorTexture->textureBuffer.Get(),
				D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

			CD3DX12_RESOURCE_BARRIER fromDestToRT = CD3DX12_RESOURCE_BARRIER::Transition(dest->colorTexture->textureBuffer.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);

			CD3DX12_RESOURCE_BARRIER afterBarriers[2] = { fromSrcToRT, fromDestToRT };
			recorder->list->ResourceBarrier(2, afterBarriers);
		}
		if (depthStencil)
		{
			CD3DX12_RESOURCE_BARRIER toCopySrc = CD3DX12_RESOURCE_BARRIER::Transition(depthStencilTexture->textureBuffer.Get(),
				D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_SOURCE);

			CD3DX12_RESOURCE_BARRIER toCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(dest->depthStencilTexture->textureBuffer.Get(),
				D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_COPY_DEST);

			CD3DX12_RESOURCE_BARRIER initialBarriers[2] = { toCopySrc, toCopyDest };
			recorder->list->ResourceBarrier(2, initialBarriers);

			recorder->list->CopyResource(dest->depthStencilTexture->textureBuffer.Get(),
				depthStencilTexture->textureBuffer.Get());

			CD3DX12_RESOURCE_BARRIER fromSrcToDW = CD3DX12_RESOURCE_BARRIER::Transition(depthStencilTexture->textureBuffer.Get(),
				D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			CD3DX12_RESOURCE_BARRIER fromDestToDW = CD3DX12_RESOURCE_BARRIER::Transition(dest->depthStencilTexture->textureBuffer.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			CD3DX12_RESOURCE_BARRIER afterBarriers[2] = { fromSrcToDW, fromDestToDW };
			recorder->list->ResourceBarrier(2, afterBarriers);
		}
	}
	else
	{
		if (color)
		{
			CD3DX12_RESOURCE_BARRIER toResolveSrc = CD3DX12_RESOURCE_BARRIER::Transition(colorTexture->textureBuffer.Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);

			CD3DX12_RESOURCE_BARRIER toResolveDest = CD3DX12_RESOURCE_BARRIER::Transition(dest->colorTexture->textureBuffer.Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_DEST);

			CD3DX12_RESOURCE_BARRIER initialBarriers[2] = { toResolveSrc, toResolveDest };
			recorder->list->ResourceBarrier(2, initialBarriers);

			recorder->list->ResolveSubresource(dest->colorTexture->textureBuffer.Get(),
				0, colorTexture->textureBuffer.Get(), 0, colorFormat);

			CD3DX12_RESOURCE_BARRIER fromSrcToRT = CD3DX12_RESOURCE_BARRIER::Transition(colorTexture->textureBuffer.Get(),
				D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

			CD3DX12_RESOURCE_BARRIER fromDestToRT = CD3DX12_RESOURCE_BARRIER::Transition(dest->colorTexture->textureBuffer.Get(),
				D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);

			CD3DX12_RESOURCE_BARRIER afterBarriers[2] = { fromSrcToRT, fromDestToRT };
			recorder->list->ResourceBarrier(2, afterBarriers);
		}
		if (depthStencil)
		{
			CD3DX12_RESOURCE_BARRIER toResolveSrc = CD3DX12_RESOURCE_BARRIER::Transition(depthStencilTexture->textureBuffer.Get(),
				D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);

			CD3DX12_RESOURCE_BARRIER toResolveDest = CD3DX12_RESOURCE_BARRIER::Transition(dest->depthStencilTexture->textureBuffer.Get(),
				D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_RESOLVE_DEST);

			CD3DX12_RESOURCE_BARRIER initialBarriers[2] = { toResolveSrc, toResolveDest };
			recorder->list->ResourceBarrier(2, initialBarriers);

			recorder->list->ResolveSubresource(dest->depthStencilTexture->textureBuffer.Get(),
				0, depthStencilTexture->textureBuffer.Get(), 0, dsFormat);

			CD3DX12_RESOURCE_BARRIER fromSrcToDW = CD3DX12_RESOURCE_BARRIER::Transition(depthStencilTexture->textureBuffer.Get(),
				D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			CD3DX12_RESOURCE_BARRIER fromDestToDW = CD3DX12_RESOURCE_BARRIER::Transition(dest->depthStencilTexture->textureBuffer.Get(),
				D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			CD3DX12_RESOURCE_BARRIER afterBarriers[2] = { fromSrcToDW, fromDestToDW };
			recorder->list->ResourceBarrier(2, afterBarriers);
		}
	}
}

void Framebuffer::Clear(CommandRecorder* recorder, bool color, bool depth)
{
	if (color) recorder->list->ClearRenderTargetView(rtvDescHeap->GetCPUDescriptorHandleForHeapStart(), &clearColor.x, 0, nullptr);

	if (depth) recorder->list->ClearDepthStencilView(
		dsDescHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, clearDepth, 0, 0, nullptr);
}
