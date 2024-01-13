#include "stdafx.h"
#include "EditorLayer.h"

#include "imgui_impl_dx12.h"
#include "imgui_impl_glfw.h"

#include "Rendering.h"
#include "ImGuiRenderPass.h"
#include "IconsMaterialDesign.h"

#include "ViewportWindow.h"
#include "SceneWindow.h"

ImTextureID EditorLayer::GetViewportTextureID()
{
	D3D12_GPU_DESCRIPTOR_HANDLE texDescStartHnd = EditorLayer::descHeap->GetGPUDescriptorHandleForHeapStart();
	UINT incrementSize = Rendering::device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_GPU_DESCRIPTOR_HANDLE texDescHnd = CD3DX12_GPU_DESCRIPTOR_HANDLE(texDescStartHnd, 1, incrementSize);

	return (ImTextureID)texDescHnd.ptr;
}

void EditorLayer::OnPush()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_::ImGuiConfigFlags_DockingEnable;

	ImFontConfig fontConfig{};
	io.Fonts->AddFontFromFileTTF(Utils::GetPathFromSolution("Editor/Fonts/Rubik-Regular.ttf").string().c_str(), 14.f);

	static const ImWchar iconRanges[] = { ICON_MIN_MD, ICON_MAX_MD, 0 };
	fontConfig.MergeMode = true;
	fontConfig.GlyphOffset.y = 4.f;
	io.Fonts->AddFontFromFileTTF(Utils::GetPathFromSolution("Editor/Fonts/MaterialIcons-Regular.ttf")
		.string().c_str(), 16.f, &fontConfig, iconRanges);

	ImGuiStyle& style = ImGui::GetStyle();

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

	viewport = new ViewportWindow();
	windows.push_back(viewport);
	windows.push_back(new SceneWindow());
}

void EditorLayer::Update()
{
	Rendering::outputCam->OnUpdate();

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("Windows"))
	{
		for (size_t i = 0; i < windows.size(); i++)
		{
			ImGui::MenuItem(windows[i]->title.c_str(), nullptr, &windows[i]->open);
		}
		ImGui::EndMenu();
	}
	ImGui::EndMainMenuBar();

	ImGui::DockSpaceOverViewport();

	for (size_t i = 0; i < windows.size(); i++)
	{
		windows[i]->Show();
	}
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

XMUINT2 EditorLayer::GetViewportSize()
{
	return XMUINT2(viewport->lastWindowSize.x, viewport->lastWindowSize.y);
}
