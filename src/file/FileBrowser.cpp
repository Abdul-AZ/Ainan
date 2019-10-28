#include <pch.h>

#include "FileBrowser.h"

namespace Ainan {

	namespace fs = std::filesystem;

	FileBrowser::FileBrowser(const std::string& startingFolder, const std::string& windowName) :
		m_CurrentFolder(startingFolder),
		m_WindowName(windowName),
		m_InputFolder(startingFolder)
	{}

	void FileBrowser::OpenWindow()
	{
		m_WindowOpen = true;
		m_LastWindowState = true;
	}

	void FileBrowser::CloseWindow()
	{
		m_WindowOpen = false;
	}

	void FileBrowser::DisplayGUI(const std::function<void(const std::string&)>& func)
	{
		if (!m_WindowOpen)
			return;

		ImGui::SetNextWindowSizeConstraints(ImVec2(BROWSER_WINDOW_WIDTH, BROWSER_MIN_WINDOW_HEIGHT), ImVec2(1000, 1000), BrowserWindowSizeCallback);

		ImGui::Begin(m_WindowName.c_str(), &m_WindowOpen);

		ImGui::Text("Current Directory :");
		ImGui::SameLine();
		auto flags = ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue;
		if (ImGui::InputText("##empty", &m_InputFolder, flags)) {
			if (fs::exists(m_InputFolder))
				m_CurrentFolder = m_InputFolder;
		}

		ImGui::Text("Current Chosen File :");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), m_CurrentselectedFilePath.c_str());
		ImGui::PushItemWidth(-1);
		if (ImGui::ListBoxHeader("##empty", ImVec2(-1, ImGui::GetWindowSize().y - 100))) {

			//check if we can go back
			if (std::count(m_CurrentFolder.begin(), m_CurrentFolder.end(), '\\') > 0) {
				//back button
				if (ImGui::Button("..")) {
					auto lastBackslashLoc = m_CurrentFolder.find_last_of('\\');
					if (std::count(m_CurrentFolder.begin(), m_CurrentFolder.end(), '\\') == 1)
						m_CurrentFolder.erase(lastBackslashLoc + 1, m_CurrentFolder.size() - lastBackslashLoc + 1);
					else
						m_CurrentFolder.erase(lastBackslashLoc, m_CurrentFolder.size() - lastBackslashLoc);
					m_InputFolder = m_CurrentFolder;
				}
			}
			for (const auto& entry : fs::directory_iterator(m_CurrentFolder)) {
				if (entry.status().type() == fs::file_type::directory && entry.path().filename() != "System Volume Information" && entry.path().filename() != "$RECYCLE.BIN") {
					if (ImGui::Button(entry.path().filename().u8string().c_str())) {
						m_CurrentFolder += "\\" + entry.path().filename().u8string();
						m_InputFolder = m_CurrentFolder;
					}
				}
			}

			for (const auto& entry : fs::directory_iterator(m_CurrentFolder)) {
				if (entry.status().type() != fs::file_type::directory) {

					bool inFilter = false;
					for (std::string& str : Filter) {
						if (entry.path().extension() == str) {
							inFilter = true;
							break;
						}
					}
					if (!inFilter)
						continue;

					bool is_selected = (m_CurrentselectedFilePath == entry.path().u8string());
					if (ImGui::Selectable(entry.path().filename().u8string().c_str(), &is_selected))
						m_CurrentselectedFilePath = entry.path().u8string();

					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
					{
						if (func != nullptr)
							func(m_CurrentselectedFilePath);
						m_WindowOpen = false;
					}
				}
			}

			ImGui::ListBoxFooter();
		}

		if (ImGui::Button("Select"))
		{
			if(func != nullptr)
				func(m_CurrentselectedFilePath);
			m_WindowOpen = false;
		}

		ImGui::End();

		//Handle OnCloseWindow if the new window state is false and the last one is true
		if (m_LastWindowState == true && m_WindowOpen == false) {
			if (OnCloseWindow)
				OnCloseWindow();
			m_LastWindowState = false;
		}
	}
}