#include <pch.h>

#include "misc/Window.h"
#include "misc/StartMenu.h"
#include "renderer/Renderer.h"

#include "editor/Editor.h"

int main() 
{
	using namespace Ainan;

	Window::Init();
	Renderer::Init();
	
	ImGuiWrapper::Init();
	SetEditorStyle(EditorStyle::Dark_Gray);

	//Environment* env = nullptr;
	//StartMenu startMenu;

	Editor* editor = new Editor;

	while (Window::ShouldClose == false)
	{
		Renderer::ClearScreen();
		editor->StartFrame();
		Window::HandleWindowEvents();
		editor->Update();
		editor->Draw();
		editor->EndFrame();
		Window::Present();

		//if (env) 
		//{
		//	env->StartFrame();
		//
		//	Window::HandleWindowEvents();
		//
		//	env->Update();
		//	env->HandleInput();
		//	env->Render();
		//	env->RenderGUI();
		//
		//	if (env->ShouldDelete) 
		//	{
		//		delete env;
		//		env = nullptr;
		//	}
		//	else 
		//	{
		//		env->EndFrame();
		//		Window::Present();
		//	}
		//}
		//else
		//{
		//	Window::HandleWindowEvents();
		//
		//	startMenu.Update(env);
		//
		//	Window::Present();
		//
		//}
	}

	delete editor;

	//if(env)
	//	delete env;
	//
	Renderer::Terminate();
	ImGuiWrapper::Terminate();
	Window::Terminate();
}