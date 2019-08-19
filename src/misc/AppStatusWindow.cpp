#include <pch.h>

#include "AppStatusWindow.h"

#include "imgui_internal.h"

void ALZ::AppStatusWindow::DisplayGUI(ImGuiID& viewportDockID)
{
	auto parentNode = ImGui::DockBuilderGetNode(viewportDockID);
	ImGuiID downNode = parentNode->ChildNodes[1]->ID;

	ImGui::DockBuilderGetNode(downNode)->LocalFlags |= ImGuiDockNodeFlags_HiddenTabBar | ImGuiDockNodeFlags_NoResize;

	ImGui::SetNextWindowDockID(downNode);
	auto flags = ImGuiWindowFlags_NoSavedSettings |
				 ImGuiWindowFlags_NoTitleBar      |
				 ImGuiWindowFlags_NoDecoration    |
				 ImGuiWindowFlags_NoMove;


	ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImVec4(0 / 255.0f, 122 / 255.0f, 204 / 255.0f, 255 / 255.0f));
	ImGui::Begin("##AppStatusWindow", nullptr, flags);
	ImGui::PopStyleColor();
	

	ImGui::Text(Text.c_str());
	
	ImGui::End();

}