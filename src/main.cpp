#include <pch.h>

#include "editor/Window.h"
#include "renderer/Renderer.h"

#include "editor/Editor.h"

#include "renderer/d3d11/D3D11RendererAPI.h"

#define USE_D3D11 1

int main() 
{
	using namespace Ainan;

#if USE_D3D11
	Window::Init(RendererType::D3D11);
	Renderer::Init(RendererType::D3D11);
	ImGuiWrapper::Init();

	{
		glm::vec2 vertices[] = 
		{   {-1.0f,-1.0f}, {0.0f,0.0f},
			{-1.0f,1.0f}, {0.0f, 1.0f},
			{1.0f, -1.0f}, {1.0f, 0.0f},
			{1.0f,1.0f}, {1.0f,1.0f}
		};
		uint32_t indecies[] = { 0, 1, 2, 3, 2, 1 };
		auto exampleShader = Renderer::CreateShaderProgram("shaders/Line", "shaders/FlatColor");
		VertexLayout layout(2);
		layout[0] = { "aPos", ShaderVariableType::Vec2 };
		layout[1] = { "aTexCoords", ShaderVariableType::Vec2 };
		auto exampleVertexBuffer = Renderer::CreateVertexBuffer(vertices, sizeof(vertices), layout, exampleShader, false);
		auto exampleIndexBuffer = Renderer::CreateIndexBuffer(indecies, 6);
		glm::mat4 viewProjection(1.0f);
		auto exampleUniformBuffer = Renderer::CreateUniformBuffer("FrameData", 0, { {"u_ViewProjection", ShaderVariableType::Mat4} }, &viewProjection);

		exampleShader->BindUniformBuffer(exampleUniformBuffer, 0, RenderingStage::VertexShader);

		viewProjection = glm::scale(viewProjection, glm::vec3(0.5f, 0.5f, 0.5f));
		exampleUniformBuffer->UpdateData(&viewProjection);
		
		auto exampleTexture = Renderer::CreateTexture(Image::LoadFromFile("res/Circle.png", 4));
		exampleTexture->Bind(0);

		while (Window::ShouldClose == false)
		{
			Renderer::ClearScreen();
			Window::HandleWindowEvents();

			//ImGuiWrapper::NewFrame();
			//ImGui::ShowDemoWindow();
			//ImGuiWrapper::Render();

			Renderer::Draw(*exampleVertexBuffer, *exampleShader, Primitive::Triangles, *exampleIndexBuffer);

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