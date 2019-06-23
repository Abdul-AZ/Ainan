#include <pch.h>
#include "FolderBrowserGUI.h"

namespace ALZ {

	namespace fs = std::filesystem;

	FolderBrowser::FolderBrowser(const std::string & startingFolder, const std::string& windowName) :
		m_CurrentFolder(startingFolder),
		m_WindowName(windowName),
		m_InputFolder(startingFolder)
	{}

	FolderBrowser::FolderBrowser()
	{
		m_CurrentFolder = FileManager::ApplicationFolder;
		m_WindowName = "Folder Browser";
		m_InputFolder = m_CurrentFolder;
	}

	void FolderBrowser::DisplayGUI()
	{
		if (!WindowOpen)
			return;

		ImGui::SetNextWindowSizeConstraints(ImVec2(BROWSER_WINDOW_WIDTH, BROWSER_MIN_WINDOW_HEIGHT), ImVec2(1000, 1000), BrowserWindowSizeCallback);
		
		ImGui::Begin(m_WindowName.c_str(), &WindowOpen, ImGuiWindowFlags_NoDocking);

		ImGui::Text("Current Directory :");
		auto flags = ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue;
		if (ImGui::InputText("##empty", &m_InputFolder, flags)) {
			if (fs::exists(m_InputFolder))
				m_CurrentFolder = m_InputFolder;
		}

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

			for (const auto & entry : fs::directory_iterator(m_CurrentFolder)) {
				if (entry.status().type() == fs::file_type::directory) {
					if (ImGui::Button(entry.path().filename().u8string().c_str())) {
						m_CurrentFolder += "\\" + entry.path().filename().u8string();
						m_InputFolder = m_CurrentFolder;
					}
				}
			}
			ImGui::ListBoxFooter();
		}
		ImGui::End();
	}
}