#include <pch.h>

#include "environment/Window.h"
#include "environment/Environment.h"

#include "noise/PerlinNoise2D.h"

int main(int argc, const char* argv[]) {

	using namespace ALZ;

	Window::Init();
	FileManager::Init(argv[0]);
	
	Environment* env = new Environment;

	float loop = 0.0f;

	PerlinNoise2D noise;
	noise.Init();

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