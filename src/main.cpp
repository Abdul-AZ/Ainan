#include <pch.h>

#include "editor/Window.h"
#include "editor/Editor.h"
#include "renderer/Renderer.h"

int main() 
{
	using namespace Ainan;

	auto api = RendererType::OpenGL;
	
	Window::Init(api);
	Renderer::Init(api);
	
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

		Renderer::Present();
	}
	
	delete editor;
	
	Renderer::Terminate();
	Window::Terminate();
}