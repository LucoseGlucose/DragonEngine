#include "stdafx.h"
#include "ProcessRenderPass.h"

#include "Rendering.h"

ProcessRenderPass::ProcessRenderPass(Material* material)
{
	this->material = material;

	CD3DX12_CLEAR_VALUE rtClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black.f);
	CD3DX12_CLEAR_VALUE dsClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	outputFB = new Framebuffer(Application::GetUnsignedFramebufferSize(), DXGI_FORMAT_R16G16B16A16_FLOAT,
		DXGI_FORMAT_D32_FLOAT, rtClear, dsClear, 1);
}

ProcessRenderPass::~ProcessRenderPass()
{
	delete material;
}

void ProcessRenderPass::Execute(Framebuffer* inputFB, CommandRecorder* recorder)
{
	outputFB->Setup(recorder, true);
	material->SetTexture("t_inputTexture", inputFB->colorTexture);

	material->Bind(recorder);
	Rendering::quadMesh->Draw(recorder);
}

void ProcessRenderPass::Resize(Framebuffer* inputFB, XMUINT2 newSize)
{
	outputFB->Resize(newSize);
	material->UpdateTexture("t_inputTexture", inputFB->colorTexture);
}
