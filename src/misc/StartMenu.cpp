#include <pch.h>
#include "StartMenu.h"
#include "ImGuiWrapper.h"

#define START_MENU_BUTTON_WIDTH 200
#define START_MENU_BUTTON_HEIGHT 75

namespace ALZ {
	
	StartMenu::StartMenu() :
	LoadEnvironmentPath(FileManager::ApplicationFolder, "Load Environment")
	{
		LoadEnvironmentPath.Filter.push_back(".env");
		LoadEnvironmentPath.OnCloseWindow = []() {
			Window::SetSize({ WINDOW_SIZE_FACTOR_ON_LAUNCH, WINDOW_SIZE_FACTOR_ON_LAUNCH * 9 / 16 });
			Window::CenterWindow();
		};
	}

	void StartMenu::Update(Environment*& currentEnv)
	{
		assert(!currentEnv);

		switch (m_CurrentStatus)
		{
		case DisplayingMainGUI:
			DisplayMainGUI(currentEnv);
			break;

		case DisplayingCreateEnvironmentGUI:
			DisplayCreateEnvironmentGUI(currentEnv);
			break;

		default:
			assert(false);
		}
	}

	inline void StartMenu::DisplayMainGUI(Environment*& currentEnv)
	{
		if (LoadEnvironmentPath.OnCloseWindow == nullptr)
			LoadEnvironmentPath.OnCloseWindow = []() {
			Window::SetSize({ WINDOW_SIZE_FACTOR_ON_LAUNCH, WINDOW_SIZE_FACTOR_ON_LAUNCH * 9 / 16 });
			Window::CenterWindow();
		};

		ImGuiWrapper::NewFrame();

		ImGui::SetNextWindowPos({ 0,0 });
		ImGui::SetNextWindowSize({ (float)Window::FramebufferSize.x , Window::FramebufferSize.y });

		ImGui::SetNextWindowDockID(ImGui::DockSpaceOverViewport(), ImGuiCond_::ImGuiCond_Always);
		ImGui::Begin("Start Menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

		ImGui::SetWindowSize(ImVec2(Window::Size.x, Window::FramebufferSize.y));

		ImGui::SetCursorPosX((float)Window::FramebufferSize.x / 2 - (float)START_MENU_BUTTON_WIDTH / 2);

		if (ImGui::Button("Create New Environment", ImVec2(START_MENU_BUTTON_WIDTH, START_MENU_BUTTON_HEIGHT)))
		{
			m_CurrentStatus = DisplayingCreateEnvironmentGUI;
			//Window::Maximize();
			//currentEnv = new Environment();
		}

		ImGui::SetCursorPosX((float)Window::FramebufferSize.x / 2 - (float)START_MENU_BUTTON_WIDTH / 2);

		if (ImGui::Button("Load Environment", ImVec2(START_MENU_BUTTON_WIDTH, START_MENU_BUTTON_HEIGHT)))
		{
			LoadEnvironmentPath.OpenWindow();
		}

		ImGui::SetCursorPosX((float)Window::FramebufferSize.x / 2 - (float)START_MENU_BUTTON_WIDTH / 2);

		if (ImGui::Button("Exit App", ImVec2(START_MENU_BUTTON_WIDTH, START_MENU_BUTTON_HEIGHT)))
		{
			Window::SetShouldClose();
		}

		ImGui::End();

		LoadEnvironmentPath.DisplayGUI([&currentEnv, this](const std::string& path)
			{
				//check if file is selected
				if (path.find(".env") != std::string::npos) {
					m_EnvironmentLoadError = CheckEnvironmentFile(path);

					//if there are no errors load environment
					if (m_EnvironmentLoadError == "") {
						//remove minimizing event on file browser window close
						LoadEnvironmentPath.OnCloseWindow = nullptr;

						Window::Maximize();
						currentEnv = LoadEnvironment(path);
					}
				}
			});

		if (m_EnvironmentLoadError != "")
			ImGui::OpenPopup("Error Loading Env");
		ImGui::SetNextWindowSize(ImVec2(400, 200));

		if (ImGui::BeginPopupModal("Error Loading Env")) {
			ImGui::TextWrapped(m_EnvironmentLoadError.c_str());

			ImGui::SetCursorPosX(ImGui::GetWindowSize().x / 2 - 75 / 2);
			ImGui::SetCursorPosY(ImGui::GetWindowSize().y - 75);

			if (ImGui::Button("Ok", ImVec2(75, 50))) {
				m_EnvironmentLoadError = "";
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGuiWrapper::Render();
	}

	inline void StartMenu::DisplayCreateEnvironmentGUI(Environment*& currentEnv)
	{
		ImGuiWrapper::NewFrame();

		ImGui::SetNextWindowDockID(ImGui::DockSpaceOverViewport(), ImGuiCond_::ImGuiCond_Always);
		ImGui::Begin("Create Environment", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

		ImGui::Text("Environment Folder");
		ImGui::SameLine();
		ImGui::InputText("##Environment Folder", &m_EnvironmentCreateFolderPath);

		bool fileExists = false, fileIsDirectory = false, fileIsEmpty = false;
		ImGui::Text("Create Directory For Environment: ");
		ImGui::SameLine();
		ImGui::Checkbox("##Create Directory For Environment: ", &m_CreateEvironmentDirectory);
		bool canCreateDirectory = !std::filesystem::exists(m_EnvironmentCreateFolderPath + m_EnvironmentCreateName + "\\");

		fileExists = std::filesystem::exists(m_EnvironmentCreateFolderPath);

		if (fileExists) 
		{
			fileIsDirectory = std::filesystem::is_directory(m_EnvironmentCreateFolderPath);
			if(fileIsDirectory)
				fileIsEmpty = std::filesystem::is_empty(m_EnvironmentCreateFolderPath);
		}

		bool unsupportedEnvironmentName = 
			   m_EnvironmentCreateName.find(" ") != std::string::npos
			|| m_EnvironmentCreateName.find(".") != std::string::npos
			|| m_EnvironmentCreateName == "";

		ImGui::Text("Environment Name");
		ImGui::SameLine();
		ImGui::InputTextWithHint("##Environment Name", "MyEnvironment", &m_EnvironmentCreateName);

		bool canSaveEnvironment = false;

		if (   fileExists     //the directory we create our environment in or the directory that we create the folder that holds our environment exists
			&& fileIsDirectory //make sure it is a directory
			&& (fileIsEmpty || m_CreateEvironmentDirectory)  //make sure the directory is not empty if aren't making a seperate directory for the environment
			&& (!m_CreateEvironmentDirectory || (m_CreateEvironmentDirectory && canCreateDirectory)) //if we are making a directory make sure there is no one like it
			&& !unsupportedEnvironmentName ) //environment name is not valid
		{
			//change create button color to green to show that the path given is valid
			ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 1.0f, 0.0f, 1.0f });
			canSaveEnvironment = true;
		}
		else
		{
			//change create button color to red to show that the path given is not valid
			ImGui::PushStyleColor(ImGuiCol_Button, { 1.0f, 0.0f, 0.0f, 1.0f });

			ImVec4 redColor = { 1.0f, 0.0f, 0.0f, 1.0f };

			//display why the path is not valid
			if (!fileExists)
				ImGui::TextColored(redColor, "Directoy does not exists");
			else if (!fileIsDirectory)
				ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "File is not a Directory");
			else if (!fileIsEmpty && !m_CreateEvironmentDirectory)
				ImGui::TextColored(redColor, "Directory is not empty");
			else if (m_CreateEvironmentDirectory && !canCreateDirectory)
				ImGui::TextColored(redColor, "Directory already exists");
			else if (unsupportedEnvironmentName)
				ImGui::TextColored(redColor, "Name is not valid");
			else
				assert(false); //we should never get to here

			canSaveEnvironment = false;
		}

		if (canSaveEnvironment) {
			if (ImGui::Button("Create"))
			{
				Window::Maximize();

				std::string dirPath = m_EnvironmentCreateFolderPath + m_EnvironmentCreateName + "\\";

				if (m_CreateEvironmentDirectory) {
					std::filesystem::create_directory(dirPath);
				}

				currentEnv = new Environment(dirPath, m_EnvironmentCreateName);
				m_CurrentStatus = DisplayingMainGUI;
			}
		}

		ImGui::PopStyleColor();

		if (ImGui::Button("Cancel"))
			m_CurrentStatus = DisplayingMainGUI;

		ImGui::End();
		ImGuiWrapper::Render();
	}
}