#include <iostream>
#include <chrono>
#include <ctime>
#include <random>

#include <glad/glad.h>
#include "Window.h"
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include "ParticleSystem.h"
#include "ParticleCustomizer.h"
#include "GeneralSettingsGUI.h"

int main() {
	Window::Init();

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glfwSwapInterval(1);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	ParticleSystem Psystem;

	std::clock_t timeStart = 0;
	std::clock_t timeEnd = 0;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(&Window::GetWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 130");
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	GeneralSettingsGUI settings;
	ParticleCustomizer customizer;

	while (!glfwWindowShouldClose(&Window::GetWindow()))
	{
		timeEnd = clock();
		float deltaTime = (timeEnd - timeStart) / 1000.0f;
		timeStart = timeEnd;

		//Update
		Window::Update();
		Psystem.Update(deltaTime);
		
		if (glfwGetMouseButton(&Window::GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !io.WantCaptureMouse) 
		{
			Psystem.SpawnParticle(customizer.GetParticle());
		}

		if (glfwGetKey(&Window::GetWindow(), GLFW_KEY_SPACE) == GLFW_PRESS)
			Psystem.ClearParticles();

		//Render
		glClear(GL_COLOR_BUFFER_BIT);
		Psystem.Draw();

		//Render ImGui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		customizer.DisplayImGuiCustomizer();
		settings.DisplayGUI();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		Window::Present();
	}
	

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(&Window::GetWindow());
	glfwTerminate();
}