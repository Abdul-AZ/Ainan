#include <pch.h>

#include "environment/Window.h"
#include "environment/Environment.h"

int main() {
	Window::Init();
	
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