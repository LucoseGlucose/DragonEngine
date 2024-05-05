#include "stdafx.h"
#include "ViewportWindow.h"

#include "IconsMaterialDesign.h"
#include "EditorLayer.h"
#include "Rendering.h"
#include "ImGuiRenderPass.h"

ViewportWindow::ViewportWindow() : EditorWindow(ICON_MD_TV" Viewport", ImVec2(256, 144))
{
	
}

void ViewportWindow::BeforeShow()
{
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
}

void ViewportWindow::OnGui()
{
	ImGui::Image(EditorLayer::GetViewportTextureID(), lastWindowSize);
}

void ViewportWindow::AfterShow()
{
	ImGui::PopStyleVar();
}

void ViewportWindow::OnResizeWindow(ImVec2 newSize)
{
	Rendering::commandQueue->WaitForAllCommands();
	Rendering::outputCam->CalculateProjection();

	XMUINT2 xmNewSize = XMUINT2(newSize.x, newSize.y);

	for (size_t i = 0; i < Rendering::renderPasses.size(); i++)
	{
		if (dynamic_cast<ImGuiRenderPass*>(Rendering::renderPasses[i]) != nullptr) continue;

		Rendering::renderPasses[i]->Resize(i != 0 ? Rendering::renderPasses[i - 1]->outputFB : nullptr, xmNewSize);
	}
}
