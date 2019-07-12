#include <pch.h>

#include "misc/Window.h"
#include "misc/Environment.h"
#include "misc/StartMenu.h"
#include "renderer/Renderer.h"

int main(int argc, const char* argv[]) {

	using namespace ALZ;

	Window::Init();
	FileManager::Init(argv[0]);
	Renderer::Init();
	
	ImGuiWrapper::Init();
	SetEditorStyle(EditorStyle::Dark_Gray);

	Environment* env = nullptr;
	StartMenu startMenu;

	while (!glfwWindowShouldClose(&Window::GetWindow()))
	{
		Window::HandleWindowEvents();

		if (env) {
			env->Update();
			env->HandleInput();
			env->Render();
			env->RenderGUI();
		
			if (env->ShouldDelete) {
				delete env;
				env = nullptr;
			}
		}
		else
			startMenu.Update(env);

		ImGuiWrapper::Render();

		Window::Present();
		Window::Clear();
	}
	

	if(env)
		delete env;
	ImGuiWrapper::Terminate();
	Window::Terminate();
}