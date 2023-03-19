#include "stdafx.h"
#include "Editor.h"

#include "Rendering.h"
#include "EditorViewport.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_dx12.h>

void Editor::Init()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

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

	windows.push_back(new EditorViewport("Viewport", std::string(), true));
}

void Editor::Update()
{
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplDX12_NewFrame();

	ImGui::NewFrame();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Windows"))
		{
			for (size_t i = 0; i < windows.size(); i++)
			{
				ImGui::MenuItem(windows[i]->title.c_str(), windows[i]->shortcut.c_str(), &windows[i]->open);
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	ImGui::DockSpaceOverViewport();

	for (size_t i = 0; i < windows.size(); i++)
	{
		windows[i]->Update();
	}
}

void Editor::Render(CommandRecorder* recorder)
{
	ImGui::Render();

	ID3D12DescriptorHeap* heaps[1] = { imGuiDescHeap.Get() };
	recorder->list->SetDescriptorHeaps(1, heaps);

	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), recorder->list.Get());
	ImGui::EndFrame();
}

void Editor::Close()
{
	for (size_t i = 0; i < windows.size(); i++)
	{
		delete windows[i];
	}

	Rendering::commandQueue->WaitForAllCommands();

	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplDX12_Shutdown();
}
