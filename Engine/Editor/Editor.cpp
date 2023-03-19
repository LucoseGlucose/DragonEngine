#include "stdafx.h"
#include "Editor.h"

#include "Rendering.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_dx12.h>

void Editor::Init()
{
	ImGui::CreateContext();

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.NumDescriptors = 2;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	Utils::ThrowIfFailed(Rendering::device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&imGuiDescHeap)));

	ImGui_ImplGlfw_InitForOther(Application::window, true);
	ImGui_ImplDX12_Init(Rendering::device.Get(), Rendering::presentationBuffer->colorTextures.size(),
		Rendering::presentationBuffer->colorTextures[0]->format, imGuiDescHeap.Get(),
		imGuiDescHeap->GetCPUDescriptorHandleForHeapStart(), imGuiDescHeap->GetGPUDescriptorHandleForHeapStart());

	ImGuiIO& io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/calibri.ttf", 14.f);
	io.Fonts->Build();
}

void Editor::Update()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplGlfw_NewFrame();

	ImGui::NewFrame();
	ImGui::ShowDemoWindow();

	ImGui::Render();
}

void Editor::Render(CommandRecorder* recorder)
{
	ID3D12DescriptorHeap* heaps[1] = { imGuiDescHeap.Get() };
	recorder->list->SetDescriptorHeaps(1, heaps);

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), recorder->list.Get());
	ImGui::EndFrame();
}

void Editor::Close()
{
	Rendering::commandQueue->WaitForAllCommands();

	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplDX12_Shutdown();
}
