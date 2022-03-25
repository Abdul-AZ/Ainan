#include "FileBrowser.h"

#include "renderer/Renderer.h"

#define STARTING_BROWSER_DIRECTORY "/"
namespace Ainan {

	namespace fs = std::filesystem;

	FileBrowser::FileBrowser(const std::string& startingFolder, const std::string& windowName) :
		m_CurrentFolderPath(startingFolder),
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

	void FileBrowser::DisplayGUI(const std::function<void(const std::filesystem::path)>& func)
	{
		if (!m_WindowOpen)
			return;

		ImGui::SetNextWindowSizeConstraints(ImVec2(BROWSER_WINDOW_WIDTH, BROWSER_MIN_WINDOW_HEIGHT), ImVec2(1000, 1000), BrowserWindowSizeCallback);

		ImGui::Begin(m_WindowName.c_str(), &m_WindowOpen);
		

		ImGui::Text("Current Directory :");
		ImGui::SameLine();
		if (ImGui::InputText("##empty", &m_InputFolder, ImGuiInputTextFlags_EnterReturnsTrue)) 
			if (fs::exists(m_InputFolder))
				m_CurrentFolderPath = m_InputFolder;

		ImGui::Text("Current Chosen File :");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), m_CurrentselectedFilePath.u8string().c_str());
		ImGui::PushItemWidth(-1);
		if (ImGui::ListBoxHeader("##empty", ImVec2(-1, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing())))
		{
			//check if we can go back
			if (m_CurrentFolderPath.parent_path() != m_CurrentFolderPath) {
				//back button
				if (ImGui::Button("..")) {
					m_CurrentFolderPath = m_CurrentFolderPath.parent_path();
					m_InputFolder = m_CurrentFolderPath.u8string();
				}
			}

			for (const auto& entry : fs::directory_iterator(m_CurrentFolderPath, fs::directory_options::skip_permission_denied))
			{
				if (AssetManager::EvaluateDirectory(entry.path())) 
				{
					//make a button for every directory
					if (ImGui::Button(entry.path().filename().u8string().c_str())) 
					{
						//if button is pressed, enter that directory:
						
						//if we are in root (eg "C:\", "D:\") we can just append the relative path of the folder
						if (m_CurrentFolderPath.parent_path() == m_CurrentFolderPath)
							m_CurrentFolderPath += entry.path().relative_path();
						//if we are not in root we have to add a backslash before the new folder's name
						else {
							m_CurrentFolderPath += "\\";
							m_CurrentFolderPath += entry.path().filename();
						}

						//update the input folder text
						m_InputFolder = m_CurrentFolderPath.u8string();
					}
				}
			}

			for (const auto& entry : fs::directory_iterator(m_CurrentFolderPath)) {
				if (entry.status().type() != fs::file_type::directory) {

					//remove unwanted files if they are filtered 
					bool inFilter = false;
					for (std::string& str : Filter) 
					{
						if (entry.path().extension() == str) {
							inFilter = true;
							break;
						}
					}
					if (!inFilter)
						continue;

					bool is_selected = (m_CurrentselectedFilePath == entry.path());
					if (ImGui::Selectable(entry.path().filename().u8string().c_str(), &is_selected))
						m_CurrentselectedFilePath = entry.path();

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

		Renderer::RegisterWindowThatCanCoverViewport();
		ImGui::End();

		//Handle OnCloseWindow if the new window state is false and the last one is true
		if (m_LastWindowState == true && m_WindowOpen == false) {
			if (OnCloseWindow)
				OnCloseWindow();
			m_LastWindowState = false;
		}
	}
}
