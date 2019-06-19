#include <pch.h>

#include "Window.h"
#include "Environment.h"
#include "StartMenu.h"

int main(int argc, const char* argv[]) {

	using namespace ALZ;

	Window::Init();
	FileManager::Init(argv[0]);
	
	ImGuiWrapper::Init();
	SetEditorStyle(EditorStyle::Dark_Gray);

	Environment* env = nullptr;
	StartMenu startMenu;

	while (!glfwWindowShouldClose(&Window::GetWindow()))
	{
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

		Window::Present();
		Window::Clear();
	}
	

	if(env)
		delete env;
	ImGuiWrapper::Terminate();
	Window::Terminate();
}