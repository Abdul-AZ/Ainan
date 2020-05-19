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

	{
		glm::vec2 vertices[2] = { {-1.0f,-1.0f}, {1.0f,1.0f} };
		auto exampleShader = Renderer::CreateShaderProgram("shaders/Line", "shaders/FlatColor");
		auto exampleVertexBuffer = Renderer::CreateVertexBuffer(vertices, sizeof(vertices), false);
		VertexLayout layout(1);
		layout[0] = { "aPos", ShaderVariableType::Vec2 };
		exampleVertexBuffer->SetLayout(layout, exampleShader);

		while (Window::ShouldClose == false)
		{
			Renderer::ClearScreen();
			Window::HandleWindowEvents();

			ImGuiWrapper::NewFrame();
			ImGui::ShowDemoWindow();
			ImGuiWrapper::Render();

			Renderer::Present();
		}
	}

	ImGuiWrapper::Terminate();
	Renderer::Terminate();
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
	
	ImGuiWrapper::Terminate();
	Renderer::Terminate();
	Window::Terminate();
#endif
}