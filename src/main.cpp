
#include "Window.h"
#include "Environment.h"

#include "stb_image.h"
#include "stb_image_write.h"

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