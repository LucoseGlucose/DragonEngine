#include "stdafx.h"
#include "ResolveRenderPass.h"

ResolveRenderPass::ResolveRenderPass()
{
	CD3DX12_CLEAR_VALUE rtClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black.f);
	CD3DX12_CLEAR_VALUE dsClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	outputFB = new Framebuffer(Application::GetUnsignedFramebufferSize(), DXGI_FORMAT_R16G16B16A16_FLOAT,
		DXGI_FORMAT_D32_FLOAT, rtClear, dsClear, 1);
}

void ResolveRenderPass::Execute(Framebuffer* inputFB, CommandRecorder* recorder)
{
	inputFB->Blit(recorder, outputFB, true, outputFB->colorTexture->format, false, DXGI_FORMAT_R32_FLOAT);
}
