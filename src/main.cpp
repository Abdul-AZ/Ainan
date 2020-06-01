#include <pch.h>

#include "editor/Window.h"
#include "renderer/Renderer.h"

#include "editor/Editor.h"

#include "renderer/d3d11/D3D11RendererAPI.h"

int main() 
{
	using namespace Ainan;

	auto api = RendererType::OpenGL;
	
	Window::Init(api);
	Renderer::Init(api);
	
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

		Renderer::Present();
	}
	
	delete editor;
	
	ImGuiWrapper::Terminate();
	Renderer::Terminate();
	Window::Terminate();
}