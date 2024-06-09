#include "stdafx.h"
#include "ResolveRenderPass.h"

ResolveRenderPass::ResolveRenderPass()
{
	CD3DX12_CLEAR_VALUE rtClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black.f);
	CD3DX12_CLEAR_VALUE dsClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	Vector2 size = Application::GetFramebufferSize();

	outputFB = new Framebuffer(RenderTextureProfile(size, DXGI_FORMAT_D32_FLOAT, dsClear, 1),
		{ RenderTextureProfile(size, DXGI_FORMAT_R16G16B16A16_FLOAT, rtClear, 1) });
}

void ResolveRenderPass::Execute(Framebuffer* inputFB, CommandRecorder* recorder)
{
	inputFB->ResolveTo(recorder, outputFB);
}
