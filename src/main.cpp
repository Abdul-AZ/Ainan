#include <pch.h>

#include "editor/Window.h"
#include "renderer/Renderer.h"

#include "editor/Editor.h"

#include "renderer/d3d11/D3D11RendererAPI.h"

int main() 
{
	using namespace Ainan;

	//Window::Init(RendererType::D3D11);
	//Renderer::Init(RendererType::D3D11);
	//
	//while (Window::ShouldClose == false)
	//{
	//	Window::HandleWindowEvents();
	//	Renderer::ClearScreen();
	//	Renderer::Present();
	//}
	//Window::Terminate();

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
	
	Renderer::Terminate();
	ImGuiWrapper::Terminate();
	Window::Terminate();
}