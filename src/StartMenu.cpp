#include <pch.h>
#include "StartMenu.h"
#include "ImGuiWrapper.h"

#define START_MENU_BUTTON_WIDTH 200
#define START_MENU_BUTTON_HEIGHT 75

namespace ALZ {

	void StartMenu::Update(Environment*& currentEnv)
	{
		assert(!currentEnv);

		Window::Update();

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

		if (ImGui::Button("Exit App", ImVec2(START_MENU_BUTTON_WIDTH, START_MENU_BUTTON_HEIGHT)))
		{
			glfwSetWindowShouldClose(&Window::GetWindow(), true);
		}

		ImGui::End();

		ImGuiWrapper::Render();
	}
}