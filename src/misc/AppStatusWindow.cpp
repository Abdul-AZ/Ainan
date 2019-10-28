#include <pch.h>

#include "AppStatusWindow.h"

#include "imgui_internal.h"

void Ainan::AppStatusWindow::DisplayGUI(ImGuiID& viewportDockID)
{
	auto parentNode = ImGui::DockBuilderGetNode(viewportDockID);
	ImGuiID downNode = parentNode->ChildNodes[1]->ID;

	ImGui::DockBuilderGetNode(downNode)->LocalFlags |= ImGuiDockNodeFlags_HiddenTabBar;
	ImGui::DockBuilderGetNode(downNode)->SizeRef.y = ImGui::GetFontSize() + ImGui::GetFrameHeight() - 5.0f;

	ImGui::SetNextWindowDockID(downNode);
	auto flags = ImGuiWindowFlags_NoSavedSettings  |
				 ImGuiWindowFlags_NoTitleBar       |
				 ImGuiWindowFlags_NoDecoration     |
				 ImGuiWindowFlags_NoScrollWithMouse|
				 ImGuiWindowFlags_NoScrollbar	   |
				 ImGuiWindowFlags_NoMove;

	ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImVec4(0 / 255.0f, 122 / 255.0f, 204 / 255.0f, 255 / 255.0f));
	ImGui::Begin("##AppStatusWindow", nullptr, flags);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.25f);
	ImGui::PopStyleColor();

	ImGui::Text((m_DisplayTime <= 0.0f) ? "Ready" : m_Text.c_str());
	
	ImGui::End();

}

void Ainan::AppStatusWindow::Update(float deltaTime)
{
	m_DisplayTime -= deltaTime;
}

void Ainan::AppStatusWindow::SetText(const std::string& text, float time)
{
	m_Text = text;
	m_DisplayTime = time;
}
