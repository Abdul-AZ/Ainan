#include <pch.h>

#include "environment/Window.h"
#include "environment/Environment.h"

#include "file/FileManager.h"

#include "renderer/Line.h"

int main(int argc, const char* argv[]) {

	Window::Init();
	FileManager::Init(argv[0]);
	
	Environment* env = new Environment;

	while (!glfwWindowShouldClose(&Window::GetWindow()))
	{
		env->Update();
		env->HandleInput();
		env->Render();
		env->RenderGUI();

		Window::Present();
		Window::Clear();
	}
	
	delete env;
	
	Window::Terminate();
}