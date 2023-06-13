#include "stdafx.h"
#include "EditorLayer.h"

#include "Rendering.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_glfw.h"
#include "ImGuiRenderPass.h"

void EditorLayer::OnPush()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_DockingEnable;

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = 2;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	Rendering::device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&descHeap));

	ImGui_ImplGlfw_InitForOther(Application::window, true);
	ImGui_ImplDX12_Init(Rendering::device.Get(), 2, DXGI_FORMAT_R16G16B16A16_FLOAT, descHeap.Get(),
		descHeap->GetCPUDescriptorHandleForHeapStart(), descHeap->GetGPUDescriptorHandleForHeapStart());

	ImGuiRenderPass* imRenderPass = new ImGuiRenderPass();

	Rendering::renderPasses.push_back(imRenderPass);
	Rendering::outputObj->material->SetTexture("t_inputTexture", imRenderPass->outputFB->colorTexture);
}

void EditorLayer::Update()
{
	Rendering::outputCam->OnUpdate();

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::DockSpaceOverViewport();
	ImGui::ShowDemoWindow();
}

void EditorLayer::Resize(XMUINT2 newSize)
{
	Rendering::Resize(newSize);
}

void EditorLayer::OnPop()
{
	Rendering::commandQueue->WaitForAllCommands();

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplGlfw_Shutdown();
}
