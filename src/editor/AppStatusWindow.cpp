#include "AppStatusWindow.h"

void Ainan::AppStatusWindow::DisplayGUI()
{
	auto flags = ImGuiWindowFlags_NoTitleBar       |
				 ImGuiWindowFlags_NoDecoration     |
				 ImGuiWindowFlags_NoScrollWithMouse|
				 ImGuiWindowFlags_NoScrollbar	   |
				 ImGuiWindowFlags_NoMove;

	ImGui::SetNextWindowSize(ImVec2(200, 200));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0 / 255.0f, 122 / 255.0f, 204 / 255.0f, 255 / 255.0f));
	ImGui::Begin("##AppStatusWindow", nullptr, flags);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.25f);
	ImGui::PopStyleColor();
	ImGui::DockBuilderGetNode(ImGui::GetWindowDockID())->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;

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
