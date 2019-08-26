#include <pch.h>

#include "ViewportWindow.h"

//TEMPORARY
#include <GLFW/glfw3.h>

namespace ALZ {

	void ViewportWindow::DisplayGUI()
	{
		ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 10.0f);
		ImGui::Begin("Viewport",nullptr ,ImGuiWindowFlags_NoBackground);
		ImGui::PopStyleVar();

		RenderViewport.x = ImGui::GetWindowPos().x;

		int height;
		glfwGetWindowSize(&Window::GetWindow(),nullptr, &height);

		RenderViewport.y = Window::FramebufferSize.y - (ImGui::GetWindowPos().y + ImGui::GetWindowContentRegionMax().y);

		RenderViewport.width = ImGui::GetWindowSize().x;
		RenderViewport.height = ImGui::GetWindowSize().y;

		ImGui::End();
	}

}