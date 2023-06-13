#include "stdafx.h"
#include "ImGuiRenderPass.h"

#include "imgui_impl_dx12.h"
#include "EditorLayer.h"

ImGuiRenderPass::ImGuiRenderPass()
{
	CD3DX12_CLEAR_VALUE rtClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R16G16B16A16_FLOAT, Colors::Black.f);
	CD3DX12_CLEAR_VALUE dsClear = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	outputFB = new Framebuffer(Application::GetUnsignedFramebufferSize(), DXGI_FORMAT_R16G16B16A16_FLOAT,
		DXGI_FORMAT_D32_FLOAT, rtClear, dsClear, 1);
}

void ImGuiRenderPass::Execute(Framebuffer* inputFB, CommandRecorder* recorder)
{
	outputFB->Setup(recorder, true);

	ImGui::Render();
	ImGui::RenderPlatformWindowsDefault();

	ID3D12DescriptorHeap* descHeaps[] = { EditorLayer::descHeap.Get() };

	recorder->list->SetDescriptorHeaps(1, descHeaps);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), recorder->list.Get());
}

void ImGuiRenderPass::Resize(Framebuffer* inputFB, XMUINT2 newSize)
{

}
