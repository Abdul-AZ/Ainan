#include <pch.h>

#include "misc/Window.h"
#include "renderer/Renderer.h"

#include "editor/Editor.h"

int main() 
{
	using namespace Ainan;

	Window::Init();
	Renderer::Init();
	
	ImGuiWrapper::Init();
	SetEditorStyle(EditorStyle::Dark_Gray);

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
	}

	delete editor;

	Renderer::Terminate();
	ImGuiWrapper::Terminate();
	Window::Terminate();
}