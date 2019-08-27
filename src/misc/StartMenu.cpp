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
			glfwSetWindowSize(Window::Ptr, WINDOW_SIZE_FACTOR_ON_LAUNCH, WINDOW_SIZE_FACTOR_ON_LAUNCH * 9 / 16);
			Window::CenterWindow();
		};
	}

	void StartMenu::Update(Environment*& currentEnv)
	{
		assert(!currentEnv);

		if(LoadEnvironmentPath.OnCloseWindow == nullptr)
			LoadEnvironmentPath.OnCloseWindow = []() {
			glfwSetWindowSize(Window::Ptr, WINDOW_SIZE_FACTOR_ON_LAUNCH, WINDOW_SIZE_FACTOR_ON_LAUNCH * 9 / 16);
			Window::CenterWindow();
		};

		ImGuiWrapper::NewFrame();	
		
		ImGui::SetNextWindowPos({ 0,0 });
		int screenSizeX, screenSizeY;
		glfwGetWindowSize(Window::Ptr, &screenSizeX, &screenSizeY);
		ImGui::SetNextWindowSize({ (float)screenSizeX , (float)screenSizeY });

		ImGui::SetNextWindowDockID(ImGui::DockSpaceOverViewport(), ImGuiCond_::ImGuiCond_Always);
		ImGui::Begin("Start Menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
		
		ImGui::SetWindowSize(ImVec2(Window::FramebufferSize.x, Window::FramebufferSize.y));

		ImGui::SetCursorPosX((float)screenSizeX / 2 - (float)START_MENU_BUTTON_WIDTH / 2);

		if (ImGui::Button("Create New Environment", ImVec2(START_MENU_BUTTON_WIDTH, START_MENU_BUTTON_HEIGHT)))
		{
			glfwMaximizeWindow(Window::Ptr);
			currentEnv = new Environment();
		}

		ImGui::SetCursorPosX((float)screenSizeX / 2 - (float)START_MENU_BUTTON_WIDTH / 2);

		if (ImGui::Button("Load Environment", ImVec2(START_MENU_BUTTON_WIDTH, START_MENU_BUTTON_HEIGHT)))
		{
			LoadEnvironmentPath.OpenWindow();
		}

		ImGui::SetCursorPosX((float)screenSizeX / 2 - (float)START_MENU_BUTTON_WIDTH / 2);

		if (ImGui::Button("Exit App", ImVec2(START_MENU_BUTTON_WIDTH, START_MENU_BUTTON_HEIGHT)))
		{
			glfwSetWindowShouldClose(Window::Ptr, true);
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

						glfwMaximizeWindow(Window::Ptr);
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