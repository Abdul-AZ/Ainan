#include <pch.h>
#include "StartMenu.h"
#include "ImGuiWrapper.h"

#define START_MENU_BUTTON_WIDTH 200
#define START_MENU_BUTTON_HEIGHT 75

namespace Ainan {
	bool SaveEnvironment(const EnvironmentData& env, std::string path);
	std::string CheckEnvironmentFile(const std::string& path);
	EnvironmentData* LoadEnvironment(const std::string& path);

	StartMenu::StartMenu() :
	m_LoadEnvironmentBrowser(STARTING_BROWSER_DIRECTORY, "Load Environment")
	{
		m_LoadEnvironmentBrowser.Filter.push_back(".env");
		m_LoadEnvironmentBrowser.OnCloseWindow = []() {
			Window::SetSize({ WINDOW_SIZE_FACTOR_ON_LAUNCH, WINDOW_SIZE_FACTOR_ON_LAUNCH * 9 / 16 });
			Window::CenterWindow();
		};
	}

	void StartMenu::Draw(EnvironmentData*& currentEnv)
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

	inline void StartMenu::DisplayMainGUI(EnvironmentData*& currentEnv)
	{
		if (m_LoadEnvironmentBrowser.OnCloseWindow == nullptr)
			m_LoadEnvironmentBrowser.OnCloseWindow = []() {
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
			Window::SetSize({ WINDOW_SIZE_ON_CREATE_ENVIRONMENT_X, WINDOW_SIZE_ON_CREATE_ENVIRONMENT_Y });
			Window::CenterWindow();
		}

		ImGui::SetCursorPosX((float)Window::FramebufferSize.x / 2 - (float)START_MENU_BUTTON_WIDTH / 2);

		if (ImGui::Button("Load Environment", ImVec2(START_MENU_BUTTON_WIDTH, START_MENU_BUTTON_HEIGHT)))
		{
			m_LoadEnvironmentBrowser.OpenWindow();
		}

		ImGui::SetCursorPosX((float)Window::FramebufferSize.x / 2 - (float)START_MENU_BUTTON_WIDTH / 2);

		if (ImGui::Button("Exit App", ImVec2(START_MENU_BUTTON_WIDTH, START_MENU_BUTTON_HEIGHT)))
		{
			Window::SetShouldClose();
		}

		ImGui::End();

		m_LoadEnvironmentBrowser.DisplayGUI([&currentEnv, this](const std::filesystem::path path)
			{
				//check if file is selected
				if (path.extension().u8string() == ".env") {
					//TODO
					m_EnvironmentLoadError = CheckEnvironmentFile(path.u8string());

					//if there are no errors load environment
					if (m_EnvironmentLoadError == "") {
						//remove minimizing event on file browser window close
						m_LoadEnvironmentBrowser.OnCloseWindow = nullptr;

						Window::Maximize();
						currentEnv = LoadEnvironment(path.u8string());
					}
				}
			});

		ImGuiWrapper::Render();
	}

	inline void StartMenu::DisplayCreateEnvironmentGUI(EnvironmentData*& currentEnv)
	{
		ImGuiWrapper::NewFrame();

		ImGui::SetNextWindowDockID(ImGui::DockSpaceOverViewport(), ImGuiCond_::ImGuiCond_Always);
		ImGui::Begin("Create Environment", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

		ImGui::Text("Environment Folder");
		ImGui::SameLine();
		ImGui::InputText("##Environment Folder", &m_EnvironmentCreateFolderPath);
		ImGui::SameLine();
		if (ImGui::Button("Browser"))
		{
			m_FolderBrowser.WindowOpen = true;
		}

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
			|| m_EnvironmentCreateName.find("\\") != std::string::npos
			|| m_EnvironmentCreateName.find("/") != std::string::npos
			|| m_EnvironmentCreateName.find("\'") != std::string::npos
			|| m_EnvironmentCreateName.find("\"") != std::string::npos
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
			canSaveEnvironment = true;
		}
		else
		{
			ImVec4 redColor = { 0.8f, 0.0f, 0.0f, 1.0f };

			//display why the path is not valid
			if (!fileExists)
				ImGui::TextColored(redColor, "Directoy does not exists");
			else if (!fileIsDirectory)
				ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "File is not a Directory");
			else if (!fileIsEmpty && !m_CreateEvironmentDirectory)
				ImGui::TextColored(redColor, "Directory is not empty");
			else if (unsupportedEnvironmentName)
				ImGui::TextColored(redColor, "Name is not valid");
			else if (m_CreateEvironmentDirectory && !canCreateDirectory)
				ImGui::TextColored(redColor, "Directory already exists");
			else
				assert(false); //we should never get to here

			canSaveEnvironment = false;
		}

		ImGui::Text("Include Starter Assets");
		ImGui::SameLine();
		ImGui::Checkbox("##Include Starter Assets", &m_IncludeStarterAssets);

		ImGui::SetCursorPosY(ImGui::GetWindowSize().y - (START_MENU_BUTTON_HEIGHT + 10));

		if (ImGui::Button("Cancel", ImVec2(START_MENU_BUTTON_WIDTH, START_MENU_BUTTON_HEIGHT)))
		{
			m_CurrentStatus = DisplayingMainGUI;
			Window::SetSize({ WINDOW_SIZE_ON_LAUNCH_X, WINDOW_SIZE_ON_LAUNCH_Y });
			Window::CenterWindow();
		}

		//change create button color to green to show that the path given is valid
		if(canSaveEnvironment)
			ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.8f, 0.0f, 1.0f });
		//change create button color to red to show that the path given is not valid
		else
			ImGui::PushStyleColor(ImGuiCol_Button, { 0.8f, 0.0f, 0.0f, 1.0f });

		ImGui::SetCursorPosY(ImGui::GetWindowSize().y - (START_MENU_BUTTON_HEIGHT + 10));
		ImGui::SetCursorPosX(ImGui::GetWindowSize().x - (START_MENU_BUTTON_WIDTH + 10));

		if (ImGui::Button("Create", ImVec2(START_MENU_BUTTON_WIDTH, START_MENU_BUTTON_HEIGHT)))
		{
			if (canSaveEnvironment) {
				Window::Maximize();

				//make sure the folder path has a backslash at the end
				if (m_EnvironmentCreateFolderPath[m_EnvironmentCreateFolderPath.size() - 1] != '\\')
					m_EnvironmentCreateFolderPath = m_EnvironmentCreateFolderPath + "\\";

				std::string dirPath = "";
				if (m_CreateEvironmentDirectory)
				{
					dirPath = m_EnvironmentCreateFolderPath + m_EnvironmentCreateName + "\\";
					std::filesystem::create_directory(dirPath);
				}
				else
				{
					dirPath = m_EnvironmentCreateFolderPath;
				}

				currentEnv = new EnvironmentData;
				currentEnv->FolderPath = dirPath;
				currentEnv->Name = m_EnvironmentCreateName;

				//add starting object
				//TODO
				//currentEnv->AddEnvironmentObject(ParticleSystemType, "Particle System");

				if (m_IncludeStarterAssets)
					std::filesystem::copy("res\\StarterAssets", dirPath + "\\StarterAssets");

				m_CurrentStatus = DisplayingMainGUI;
			}
		}

		ImGui::PopStyleColor();

		ImGui::End();

		m_FolderBrowser.DisplayGUI([this](const std::string& dir) {
			m_EnvironmentCreateFolderPath = dir;
			});

		ImGuiWrapper::Render();
	}
}