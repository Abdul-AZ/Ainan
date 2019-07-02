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
			glfwSetWindowSize(&Window::GetWindow(), WINDOW_SIZE_FACTOR_ON_LAUNCH, WINDOW_SIZE_FACTOR_ON_LAUNCH * 9 / 16);
			Window::CenterWindow();
		};
	}

	void StartMenu::Update(Environment*& currentEnv)
	{
		assert(!currentEnv);

		if(LoadEnvironmentPath.OnCloseWindow == nullptr)
			LoadEnvironmentPath.OnCloseWindow = []() {
			glfwSetWindowSize(&Window::GetWindow(), WINDOW_SIZE_FACTOR_ON_LAUNCH, WINDOW_SIZE_FACTOR_ON_LAUNCH * 9 / 16);
			Window::CenterWindow();
		};

		ImGuiWrapper::NewFrame();	
		
		ImGui::SetNextWindowPos({ 0,0 });
		int screenSizeX, screenSizeY;
		glfwGetWindowSize(&Window::GetWindow(), &screenSizeX, &screenSizeY);
		ImGui::SetNextWindowSize({ (float)screenSizeX , (float)screenSizeY });

		ImGui::Begin("Start Menu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

		ImGui::SetWindowSize(ImVec2(Window::WindowSize.x, Window::WindowSize.y));

		ImGui::SetCursorPosX(screenSizeX / 2 - START_MENU_BUTTON_WIDTH / 2);

		if (ImGui::Button("Create New Environment", ImVec2(START_MENU_BUTTON_WIDTH, START_MENU_BUTTON_HEIGHT)))
		{
			glfwMaximizeWindow(&Window::GetWindow());
			currentEnv = new Environment();
		}

		ImGui::SetCursorPosX(screenSizeX / 2 - START_MENU_BUTTON_WIDTH / 2);

		if (ImGui::Button("Load Environment", ImVec2(START_MENU_BUTTON_WIDTH, START_MENU_BUTTON_HEIGHT)))
		{
			glfwSetWindowSize(&Window::GetWindow(), WINDOW_SIZE_FACTOR_ON_LAUNCH * 3, WINDOW_SIZE_FACTOR_ON_LAUNCH * 3 * 9 / 16);
			Window::CenterWindow();
			LoadEnvironmentPath.OpenWindow();
		}

		ImGui::SetCursorPosX(screenSizeX / 2 - START_MENU_BUTTON_WIDTH / 2);

		if (ImGui::Button("Exit App", ImVec2(START_MENU_BUTTON_WIDTH, START_MENU_BUTTON_HEIGHT)))
		{
			glfwSetWindowShouldClose(&Window::GetWindow(), true);
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

						glfwMaximizeWindow(&Window::GetWindow());
						currentEnv = LoadEnvironment(path);
					}
				}
			});

		if(m_EnvironmentLoadError != "")
			ImGui::OpenPopup("Error Loading Env");
		ImGui::SetNextWindowSize(ImVec2(400, 200));

		if (ImGui::BeginPopupModal("Error Loading Env")) {
			ImGui::TextWrapped(m_EnvironmentLoadError.c_str());

			ImGui::SetCursorPosX(ImGui::GetWindowSize().x / 2 - 75 / 2);
			ImGui::SetCursorPosY(ImGui::GetWindowSize().y - 75);

			if (ImGui::Button("Ok", ImVec2(75,50))) {
				m_EnvironmentLoadError = "";
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
		 
		ImGuiWrapper::Render();
	}
}