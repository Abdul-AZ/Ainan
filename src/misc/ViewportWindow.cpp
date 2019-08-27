#include <pch.h>

#include "ViewportWindow.h"

namespace ALZ {

	void ViewportWindow::DisplayGUI()
	{
		ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 10.0f);
		ImGui::Begin("Viewport",nullptr ,ImGuiWindowFlags_NoBackground);
		ImGui::PopStyleVar();

		RenderViewport.x = ImGui::GetWindowPos().x;

		RenderViewport.y = Window::FramebufferSize.y - (ImGui::GetWindowPos().y + ImGui::GetWindowContentRegionMax().y + ImGui::GetFrameHeightWithSpacing());

		RenderViewport.width = ImGui::GetWindowSize().x;
		RenderViewport.height = ImGui::GetWindowSize().y;

		RenderViewport.x -= Window::Position.x;
		RenderViewport.y += Window::Position.y; //negative because y axis is inverted in screen coordinates

		ImGui::End();
	}

}