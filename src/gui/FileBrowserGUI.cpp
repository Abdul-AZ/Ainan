#include <pch.h>
#include "FileBrowserGUI.h"

namespace ALZ {

	namespace fs = std::filesystem;

	FileBrowser::FileBrowser(const std::string& startingFolder, const std::string& windowName) :
		m_CurrentFolder(startingFolder),
		m_WindowName(windowName),
		m_InputFolder(startingFolder)
	{}

	void FileBrowser::OpenWindow()
	{
		m_WindowOpen = true;
	}

	void FileBrowser::CloseWindow()
	{
		m_WindowOpen = false;
	}

	void FileBrowser::DisplayGUI(const std::function<void(const std::string&)>& func)
	{
		if (!m_WindowOpen)
			return;

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
		ImGui::ListBoxHeader("##empty", 0, std::distance(fs::directory_iterator(m_CurrentFolder), fs::directory_iterator{}) + 5);

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

		for (const auto & entry : fs::directory_iterator(m_CurrentFolder)) {
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
			}
		}
		ImGui::ListBoxFooter();

		if (ImGui::Button("Select"))
		{
			if(func != nullptr)
				func(m_CurrentselectedFilePath);
			m_WindowOpen = false;
		}

		ImGui::End();
	}
}