#include "stdafx.h"
#include "ProcessRenderPass.h"

#include "Rendering.h"

ProcessRenderPass::ProcessRenderPass(Material* material)
{
	this->material = material;

	CD3DX12_CLEAR_VALUE rtClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black.f);
	CD3DX12_CLEAR_VALUE dsClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	Vector2 size = Application::GetFramebufferSize();

	outputFB = new Framebuffer(RenderTextureProfile(size, DXGI_FORMAT_D32_FLOAT, dsClear, 1),
		{ RenderTextureProfile(size, DXGI_FORMAT_R16G16B16A16_FLOAT, rtClear, 1) });
}

ProcessRenderPass::~ProcessRenderPass()
{
	delete material;
}

void ProcessRenderPass::Execute(Framebuffer* inputFB, CommandRecorder* recorder)
{
	outputFB->Setup(recorder);
	material->SetTexture("t_inputTexture", inputFB->colorTextures.front());

	material->Bind(recorder, outputFB->pipelineProfile);
	Rendering::quadMesh->Draw(recorder);
}

void ProcessRenderPass::Resize(Framebuffer* inputFB, Vector2 newSize)
{
	outputFB->Resize(newSize);
	material->UpdateTexture("t_inputTexture", inputFB->colorTextures.front());
}
