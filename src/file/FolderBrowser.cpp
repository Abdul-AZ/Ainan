#include "FolderBrowser.h"

#include "renderer/Renderer.h"

namespace Ainan {

	namespace fs = std::filesystem;

	FolderBrowser::FolderBrowser(const std::string & startingFolder, const std::string& windowName) :
		SelectedFolder(startingFolder),
		m_WindowName(windowName),
		m_InputFolder(startingFolder)
	{}

	FolderBrowser::FolderBrowser() : 
		SelectedFolder(STARTING_BROWSER_DIRECTORY)
	{
		m_WindowName = "Folder Browser";
		m_InputFolder = STARTING_BROWSER_DIRECTORY;
	}

	void FolderBrowser::DisplayGUI(std::function<void(const fs::path&)> onSelect)
	{
		if (!WindowOpen)
			return;

		ImGui::PushID(this);

		ImGui::SetNextWindowSizeConstraints(ImVec2(BROWSER_WINDOW_WIDTH, BROWSER_MIN_WINDOW_HEIGHT), ImVec2(1000, 1000), BrowserWindowSizeCallback);
		
		ImGui::Begin(m_WindowName.c_str(), &WindowOpen, ImGuiWindowFlags_NoDocking);

		ImGui::Text("Current Directory :");
		auto flags = ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue;

		if (ImGui::InputText("##empty", &m_InputFolder, flags)) 
		{
			if (fs::exists(m_InputFolder))
				SelectedFolder = m_InputFolder;
		}

		ImGui::PushItemWidth(-1);
		if (ImGui::ListBoxHeader("##empty", ImVec2(-1, ImGui::GetWindowSize().y - 100))) 
		{
			//check if we can go back
			if (SelectedFolder.parent_path() != SelectedFolder) 
			{
				//back button
				if (ImGui::Button("..")) {
					SelectedFolder = SelectedFolder.parent_path();
					m_InputFolder = SelectedFolder.u8string();
				}
			}

			for (const auto& entry : fs::directory_iterator(SelectedFolder)) 
			{
				if (AssetManager::EvaluateDirectory(entry.path()))
				{
					bool is_selected = (SelectedFolder == entry.path());
					if (ImGui::Selectable(entry.path().filename().u8string().c_str(), &is_selected))
						SelectedFolder = entry.path();
				}
			}

			ImGui::ListBoxFooter();
		}

		if (ImGui::Button("Select"))
		{
			if (onSelect != nullptr)
				onSelect(SelectedFolder);
			WindowOpen = false;
		}

		Renderer::RegisterWindowThatCanCoverViewport();
		ImGui::End();

		ImGui::PopID();
	}
}
