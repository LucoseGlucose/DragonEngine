#include "stdafx.h"
#include "ViewportWindow.h"

#include "EditorLayer.h"
#include "Rendering.h"
#include "ImGuiRenderPass.h"

ViewportWindow::ViewportWindow(EditorLayer* el, EditorWindowIndex windowIndex) : EditorWindow(el, windowIndex, GetTitle(), ImVec2(256, 144))
{
	
}

void ViewportWindow::BeforeShow()
{
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
}

void ViewportWindow::OnGui()
{
	ImGui::Image(editorLayer->GetViewportTextureID(), lastWindowSize);
}

void ViewportWindow::AfterShow()
{
	ImGui::PopStyleVar();
}

void ViewportWindow::OnResizeWindow(ImVec2 newSize)
{
	Vector2 xmNewSize = Vector2(newSize.x, newSize.y);

	Rendering::commandQueue->WaitForAllCommands();
	Rendering::outputCam->SetSize(xmNewSize);

	for (size_t i = 0; i < Rendering::renderPasses.size(); i++)
	{
		if (dynamic_cast<ImGuiRenderPass*>(Rendering::renderPasses[i]) != nullptr) continue;

		Rendering::renderPasses[i]->Resize(i != 0 ? Rendering::renderPasses[i - 1]->outputFB : nullptr, xmNewSize);
	}
}
