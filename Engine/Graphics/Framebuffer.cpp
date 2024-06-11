#include "stdafx.h"
#include "Framebuffer.h"

#include "Rendering.h"

Framebuffer::Framebuffer(RenderTextureProfile dsProfile, std::initializer_list<RenderTextureProfile> rtvProfiles)
	:
	pipelineProfile(),
	size(dsProfile.size),
	samples(dsProfile.samples),
	colorDescHeap(rtvProfiles.size(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE),
	depthDescHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE),
	depthTexture(new DepthTexture(dsProfile))
{
	for (size_t i = 0; i < rtvProfiles.size(); i++)
	{
		if (i >= 8) Utils::CrashWithMessage(L"Too many color textures - Maximum is 8");

		ColorTexture* tex = new ColorTexture(rtvProfiles.begin()[i]);
		if (tex->samples != samples) Utils::CrashWithMessage(L"Sample count must be equal for all attachments");

		colorTextures.push_back(tex);
		tex->CreateRTV(colorDescHeap.GetCPUHandleForIndex(i));
	}

	depthTexture->CreateDSV(depthDescHeap.GetCPUHandle());

	pipelineProfile.dsvFormat = depthTexture->format;
	pipelineProfile.sampleDesc.Count = samples;

	for (size_t i = 0; i < colorTextures.size(); i++)
	{
		pipelineProfile.rtvFormats.push_back(colorTextures[i]->format);
	}
}

Framebuffer::Framebuffer(std::initializer_list<RenderTextureProfile> rtvProfiles)
	:
	pipelineProfile(),
	size(rtvProfiles.begin()->size),
	samples(rtvProfiles.begin()->samples),
	colorDescHeap(rtvProfiles.size(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE),
	depthDescHeap(0, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE),
	depthTexture(nullptr)
{
	for (size_t i = 0; i < rtvProfiles.size(); i++)
	{
		if (i >= 8) Utils::CrashWithMessage(L"Too many color textures - Maximum is 8");

		ColorTexture* tex = new ColorTexture(rtvProfiles.begin()[i]);
		if (tex->samples != samples) Utils::CrashWithMessage(L"Sample count must be equal for all attachments");

		colorTextures.push_back(tex);
		tex->CreateRTV(colorDescHeap.GetCPUHandleForIndex(i));
	}

	pipelineProfile.sampleDesc.Count = samples;
	pipelineProfile.depthStencilState.DepthEnable = false;

	for (size_t i = 0; i < colorTextures.size(); i++)
	{
		pipelineProfile.rtvFormats.push_back(colorTextures[i]->format);
	}
}

Framebuffer::Framebuffer(RenderTextureProfile dsProfile)
	:
	pipelineProfile(),
	size(dsProfile.size),
	samples(dsProfile.samples),
	colorDescHeap(0, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE),
	depthDescHeap(1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE),
	depthTexture(new DepthTexture(dsProfile))
{
	depthTexture->CreateDSV(depthDescHeap.GetCPUHandle());

	pipelineProfile.dsvFormat = depthTexture->format;
	pipelineProfile.sampleDesc.Count = samples;
}

Framebuffer::~Framebuffer()
{
	for (size_t i = 0; i < colorTextures.size(); i++)
	{
		delete colorTextures[i];
	};

	delete depthTexture;
}

void Framebuffer::Resize(Vector2 newSize)
{
	if (newSize.x == 0 || newSize.y == 0) return;

	for (size_t i = 0; i < colorTextures.size(); i++)
	{
		colorTextures[i]->Resize(newSize);
		colorTextures[i]->CreateRTV(colorDescHeap.GetCPUHandleForIndex(i));
	}

	depthTexture->Resize(newSize);
	depthTexture->CreateDSV(depthDescHeap.GetCPUHandle());
}

void Framebuffer::Setup(CommandRecorder* recorder, UINT8 colorClearMask, bool clearDepth)
{
	recorder->list->RSSetViewports(1, &Rendering::viewport);
	recorder->list->RSSetScissorRects(1, &Rendering::scissorRect);
	recorder->list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUHandle{};
	D3D12_CPU_DESCRIPTOR_HANDLE dsCPUHandle{};

	if (colorTextures.size() > 0) rtvCPUHandle = colorDescHeap.GetCPUHandle();
	if (depthTexture != nullptr) dsCPUHandle = depthDescHeap.GetCPUHandle();

	recorder->list->OMSetRenderTargets(colorDescHeap.size, colorTextures.size() < 1 ? nullptr : &rtvCPUHandle,
		true, depthTexture == nullptr ? nullptr : &dsCPUHandle);
	Clear(recorder, colorClearMask, clearDepth);
}

void Framebuffer::Clear(CommandRecorder* recorder, UINT8 colorMask, bool depth)
{
	for (UINT8 i = 0; i < colorTextures.size(); i++)
	{
		if (Rendering::GetColorMaskForIndex(i) & colorMask)
		{
			recorder->list->ClearRenderTargetView(colorDescHeap.GetCPUHandleForIndex(i), colorTextures[i]->clearValue.Color, 0, nullptr);
		}
	}

	if (depth && depthTexture != nullptr)
	{
		recorder->list->ClearDepthStencilView(depthDescHeap.GetCPUHandle(), D3D12_CLEAR_FLAG_DEPTH,
			depthTexture->clearValue.DepthStencil.Depth, depthTexture->clearValue.DepthStencil.Stencil, 0, nullptr);
	}
}

void Framebuffer::BlitTo(CommandRecorder* recorder, Framebuffer* dest, UINT8 colorMask, bool depthStencil)
{
	if (samples < 2) CopyTo(recorder, dest, colorMask, depthStencil);
	else ResolveTo(recorder, dest, colorMask, depthStencil);
}

void Framebuffer::CopyTo(CommandRecorder* recorder, Framebuffer* dest, UINT8 colorMask, bool depthStencil)
{
	for (UINT8 i = 0; i < colorTextures.size(); i++)
	{
		if (Rendering::GetColorMaskForIndex(i) & colorMask)
		{
			Rendering::RecordBarriers(recorder, { colorTextures[i]->TransitionToState(D3D12_RESOURCE_STATE_COPY_SOURCE),
				dest->colorTextures[i]->TransitionToState(D3D12_RESOURCE_STATE_COPY_DEST) });

			recorder->list->CopyResource(dest->colorTextures[i]->resourceBuffer.Get(), colorTextures[i]->resourceBuffer.Get());

			Rendering::RecordBarriers(recorder, { colorTextures[i]->TransitionToState(D3D12_RESOURCE_STATE_RENDER_TARGET),
				dest->colorTextures[i]->TransitionToState(D3D12_RESOURCE_STATE_RENDER_TARGET) });
		}
	}

	if (!depthStencil) return;

	Rendering::RecordBarriers(recorder, { depthTexture->TransitionToState(D3D12_RESOURCE_STATE_COPY_SOURCE),
				dest->depthTexture->TransitionToState(D3D12_RESOURCE_STATE_COPY_DEST) });

	recorder->list->CopyResource(dest->depthTexture->resourceBuffer.Get(), depthTexture->resourceBuffer.Get());

	Rendering::RecordBarriers(recorder, { depthTexture->TransitionToState(D3D12_RESOURCE_STATE_DEPTH_WRITE),
		dest->depthTexture->TransitionToState(D3D12_RESOURCE_STATE_RENDER_TARGET) });
}

void Framebuffer::ResolveTo(CommandRecorder* recorder, Framebuffer* dest, UINT8 colorMask, bool depthStencil)
{
	for (UINT8 i = 0; i < colorTextures.size(); i++)
	{
		if (Rendering::GetColorMaskForIndex(i) & colorMask)
		{
			Rendering::RecordBarriers(recorder, { colorTextures[i]->TransitionToState(D3D12_RESOURCE_STATE_RESOLVE_SOURCE),
				dest->colorTextures[i]->TransitionToState(D3D12_RESOURCE_STATE_RESOLVE_DEST) });

			recorder->list->ResolveSubresource(dest->colorTextures[i]->resourceBuffer.Get(),
				0, colorTextures[i]->resourceBuffer.Get(), 0, dest->colorTextures[i]->format);

			Rendering::RecordBarriers(recorder, { colorTextures[i]->TransitionToState(D3D12_RESOURCE_STATE_RENDER_TARGET),
				dest->colorTextures[i]->TransitionToState(D3D12_RESOURCE_STATE_RENDER_TARGET) });
		}
	}

	if (!depthStencil) return;

	Rendering::RecordBarriers(recorder, { depthTexture->TransitionToState(D3D12_RESOURCE_STATE_COPY_SOURCE),
				dest->depthTexture->TransitionToState(D3D12_RESOURCE_STATE_COPY_DEST) });

	recorder->list->ResolveSubresource(dest->depthTexture->resourceBuffer.Get(),
		0, depthTexture->resourceBuffer.Get(), 0, Rendering::GetResolveFormatForDepth(dest->depthTexture->format));

	Rendering::RecordBarriers(recorder, { depthTexture->TransitionToState(D3D12_RESOURCE_STATE_DEPTH_WRITE),
		dest->depthTexture->TransitionToState(D3D12_RESOURCE_STATE_DEPTH_WRITE) });
}
