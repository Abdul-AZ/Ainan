#include "SaveItemBrowser.h"

#include "renderer/Renderer.h"

namespace Ainan {

	namespace fs = std::filesystem;

	SaveItemBrowser::SaveItemBrowser():
		m_CurrentFolderPath(STARTING_BROWSER_DIRECTORY),
		m_WindowName("File Browser"),
		m_InputFolder(STARTING_BROWSER_DIRECTORY)
	{}

	SaveItemBrowser::SaveItemBrowser(const std::string& startingFolder, const std::string& windowName) :
		m_CurrentFolderPath(startingFolder),
		m_WindowName(windowName),
		m_InputFolder(startingFolder)
	{}

	void SaveItemBrowser::OpenWindow()
	{
		m_WindowOpen = true;
		m_LastWindowState = true;
	}

	void SaveItemBrowser::CloseWindow()
	{
		m_WindowOpen = false;
	}

	void SaveItemBrowser::DisplayGUI(const std::function<void(const std::string&)>& func)
	{
		if (!m_WindowOpen)
			return;

		ImGui::SetNextWindowSizeConstraints(ImVec2(BROWSER_WINDOW_WIDTH, BROWSER_MIN_WINDOW_HEIGHT), ImVec2(1000,1000), BrowserWindowSizeCallback);

		ImGui::Begin(m_WindowName.c_str(), &m_WindowOpen, ImGuiWindowFlags_NoDocking);

		ImGui::Text("Current Directory :");
		ImGui::SameLine();
		if (ImGui::InputText("##empty", &m_InputFolder, ImGuiInputTextFlags_EnterReturnsTrue))
			if (fs::exists(m_InputFolder))
				m_CurrentFolderPath = m_InputFolder;

		ImGui::Text("Selected Path: ");
		ImGui::SameLine();
		ImGui::TextColored({ 0.0f,0.8f,0.0f,1.0f }, GetSelectedSavePath().c_str());

		if (ImGui::ListBoxHeader("##empty", ImVec2(-1, ImGui::GetWindowSize().y - 125)))
		{
			//check if we can go back
			if (m_CurrentFolderPath.parent_path() != m_CurrentFolderPath) {
				//back button
				if (ImGui::Button("..")) {
					m_CurrentFolderPath = m_CurrentFolderPath.parent_path();
					m_InputFolder = m_CurrentFolderPath.u8string();
				}
			}
			for (const auto& entry : fs::directory_iterator(m_CurrentFolderPath)) 
			{
				if (entry.status().type() == fs::file_type::directory) 
				{
					//make a button for every directory
					if (ImGui::Button(entry.path().filename().u8string().c_str())) {

						//if button is pressed, enter that directory:

						//if we are in root (eg "C:\", "D:\") we can just append the relative path of the folder
						if (m_CurrentFolderPath.parent_path() == m_CurrentFolderPath)
							m_CurrentFolderPath += entry.path().relative_path();
						//if we are not in root we have to add a backslash before the new folder's name
						else 
						{
							m_CurrentFolderPath += "\\";
							m_CurrentFolderPath += entry.path().filename();
						}

						//update the input folder text
						m_InputFolder = m_CurrentFolderPath.u8string();
					}
				}
			}
			ImGui::ListBoxFooter();
		}


		ImGui::PushItemWidth(BROWSER_WINDOW_WIDTH - 70);

		if (ImGui::InputText("##FileName", &m_FileName)) {
			if (m_FileName.size() > 0)
				m_FileNameChosen = true;
			else
				m_FileNameChosen = false;
		}

		ImGui::SameLine();

		if (ImGui::Button("Save"))
		{
			if (func != nullptr && m_FileNameChosen) 
			{
				func(GetSelectedSavePath());
				m_WindowOpen = false;
			}
		}

		if (m_FileName.find(".") != std::string::npos)
		{
			m_FileNameChosen = false;
			ImGui::TextColored(ImVec4(0.8f, 0.0f, 0.0f, 1.0f), "Do not include file format!");
		}

		Renderer::RegisterWindowThatCanCoverViewport();
		ImGui::End();

		//Handle OnCloseWindow if the new window state is false and the last one is true
		if (m_LastWindowState == true && m_WindowOpen == false) 
		{
			if (OnCloseWindow)
				OnCloseWindow();
			m_LastWindowState = false;
		}
	}

	std::string SaveItemBrowser::GetSelectedSavePath()
	{
		if (m_CurrentFolderPath.parent_path() == m_CurrentFolderPath)
			return std::filesystem::path(m_CurrentFolderPath.u8string()  + m_FileName + FileExtension).u8string();

		return std::filesystem::path(m_CurrentFolderPath.u8string() + "\\" + m_FileName + FileExtension).u8string();
	}
}
