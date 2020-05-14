#include <pch.h>

#include "editor/Window.h"
#include "renderer/Renderer.h"

#include "editor/Editor.h"

#include "renderer/d3d11/D3D11RendererAPI.h"

int main() 
{
	using namespace Ainan;

	Window::Init();

	//D3D11::D3D11RendererAPI api;
	//
	//api.ClearScreen();
	//api.Present();
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
		
		Renderer::Present();
	}

	delete editor;

	Renderer::Terminate();
	ImGuiWrapper::Terminate();
	Window::Terminate();
}