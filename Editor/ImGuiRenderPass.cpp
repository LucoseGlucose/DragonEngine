#include "stdafx.h"
#include "ImGuiRenderPass.h"

#include "imgui_impl_dx12.h"
#include "EditorLayer.h"
#include "Rendering.h"

ImGuiRenderPass::ImGuiRenderPass()
{
	CD3DX12_CLEAR_VALUE rtClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black.f);
	CD3DX12_CLEAR_VALUE dsClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	outputFB = new Framebuffer(RenderTextureProfile(Application::GetFramebufferSize(), DXGI_FORMAT_D32_FLOAT, dsClear, 1),
		{ RenderTextureProfile(Application::GetFramebufferSize(), DXGI_FORMAT_R16G16B16A16_FLOAT, rtClear, 1) });

	Rendering::outputObj->GetMaterial()->SetTexture("t_inputTexture", outputFB->colorTextures.front());
}

void ImGuiRenderPass::Execute(Framebuffer* inputFB, CommandRecorder* recorder)
{
	outputFB->Setup(recorder);

	D3D12_CPU_DESCRIPTOR_HANDLE texDescStartHnd = EditorLayer::descHeap->GetCPUDescriptorHandleForHeapStart();
	UINT incrementSize = Rendering::device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE texDescHnd = CD3DX12_CPU_DESCRIPTOR_HANDLE(texDescStartHnd, 1, incrementSize);

	Rendering::device->CreateShaderResourceView(inputFB->colorTextures.front()->resourceBuffer.Get(),
		&inputFB->colorTextures.front()->srvDesc, texDescHnd);

	ImGui::Render();

	ID3D12DescriptorHeap* descHeaps[] = { EditorLayer::descHeap.Get() };

	recorder->list->SetDescriptorHeaps(1, descHeaps);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), recorder->list.Get());
}
