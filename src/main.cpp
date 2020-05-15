#include <pch.h>

#include "editor/Window.h"
#include "renderer/Renderer.h"

#include "editor/Editor.h"

#include "renderer/d3d11/D3D11RendererAPI.h"

#define USE_D3D11 0

int main() 
{
	using namespace Ainan;

#if USE_D3D11
	Window::Init(RendererType::D3D11);
	Renderer::Init(RendererType::D3D11);
	ImGuiWrapper::Init();

	while (Window::ShouldClose == false)
	{
		Renderer::ClearScreen();
		Window::HandleWindowEvents();

		ImGuiWrapper::NewFrame();
		ImGui::ShowDemoWindow();
		ImGuiWrapper::Render();

		Renderer::Present();
	}

	ImGuiWrapper::Terminate();
	Window::Terminate();

#else

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
#endif
}