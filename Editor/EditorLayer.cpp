#include "stdafx.h"
#include "EditorLayer.h"

#include "imgui_impl_dx12.h"
#include "imgui_impl_glfw.h"

#include "Rendering.h"
#include "ImGuiRenderPass.h"
#include "IconsMaterialDesign.h"

#include "ViewportWindow.h"
#include "SceneWindow.h"
#include "StatsWindow.h"

ImTextureID EditorLayer::GetViewportTextureID()
{
	D3D12_GPU_DESCRIPTOR_HANDLE texDescStartHnd = EditorLayer::descHeap->GetGPUDescriptorHandleForHeapStart();
	UINT incrementSize = Rendering::device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	CD3DX12_GPU_DESCRIPTOR_HANDLE texDescHnd = CD3DX12_GPU_DESCRIPTOR_HANDLE(texDescStartHnd, 1, incrementSize);

	return (ImTextureID)texDescHnd.ptr;
}

void EditorLayer::OpenEditorWindow(EditorWindowIndex windowIndex)
{
	EditorWindowData* data = &availableWindows[(int)windowIndex];

	data->open = true;
	windows.push_back(data->creationFunc(this));
}

void EditorLayer::CloseEditorWindow(EditorWindow* window)
{
	availableWindows[(int)window->windowIndex].open = false;

	Utils::RemoveFromVector(&windows, window);
	delete window;
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

	for (size_t i = 0; i < availableWindows.size(); i++)
	{
		EditorWindowData data = availableWindows[i];
		if (data.open) OpenEditorWindow(data.index);
	}
}

void EditorLayer::Update()
{
	if (Application::GetFramebufferSize().x == 0 || Application::GetFramebufferSize().y == 0) return;

	Rendering::outputCam->OnUpdate();
	
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("Windows"))
	{
		for (size_t i = 0; i < availableWindows.size(); i++)
		{
			EditorWindowData* data = &availableWindows[i];

			bool open = data->open;
			ImGui::MenuItem(data->title.c_str(), nullptr, &open);

			if (open != data->open)
			{
				if (open) OpenEditorWindow(data->index);
				else
				{
					EditorWindow* w = GetWindowByTitle(data->title);
					if (w != nullptr) CloseEditorWindow(w);
				}
			}
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

void EditorLayer::Resize(Vector2 newSize)
{
	Rendering::Resize(newSize);
}

void EditorLayer::OnPop()
{
	Rendering::commandQueue->WaitForAllCommands();

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplGlfw_Shutdown();
}

EditorWindow* EditorLayer::GetWindowByTitle(std::string title)
{
	auto it = std::find_if(windows.begin(), windows.end(), [=](EditorWindow* w) -> bool { return w->title == title; });

	if (it != windows.end()) return it[0];
	return nullptr;
}

bool EditorLayer::TryGetWindowByTitle(std::string title, EditorWindow** out)
{
	*out = GetWindowByTitle(title);
	return *out == nullptr;
}
