#include "stdafx.h"
#include "EditorViewport.h"

#include "Rendering.h"
#include "Editor.h"

EditorViewport::EditorViewport(const std::string& title, const std::string& shortcut, bool open)
	: EditorWindow(title, shortcut, open)
{
	viewportSize = ImVec2(Application::GetFramebufferSize().x, Application::GetFramebufferSize().y);

	Rendering::device->CreateShaderResourceView(Rendering::postFB->colorTexture->textureBuffer.Get(),
		&Rendering::postFB->colorTexture->srvDesc, CD3DX12_CPU_DESCRIPTOR_HANDLE(Editor::imGuiDescHeap->GetCPUDescriptorHandleForHeapStart(),
			1, Rendering::device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)));
}

void EditorViewport::ShowWindow()
{
	ImGui::Image((ImTextureID)CD3DX12_GPU_DESCRIPTOR_HANDLE(Editor::imGuiDescHeap->GetGPUDescriptorHandleForHeapStart(), 1,
		Rendering::device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)).ptr, viewportSize);
}
