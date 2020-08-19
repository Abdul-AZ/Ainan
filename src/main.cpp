#include <pch.h>

#include "editor/Window.h"
#include "editor/Editor.h"
#include "editor/EditorPreferences.h"
#include "renderer/Renderer.h"

int main() 
{
	using namespace Ainan;

	auto api = EditorPreferences::LoadFromDefaultPath().RenderingBackend;

	Window::Init(api);
	Renderer::Init(api);
	
	Editor* editor = new Editor;
	InputManager::Init();

	while (Window::ShouldClose == false)
	{
		editor->StartFrame();
		
		Window::HandleWindowEvents();
		editor->Update();
		InputManager::HandleInput();
		editor->Draw();
		
		editor->EndFrame();
	
		Renderer::Present();
	}
	
	InputManager::Terminate();
	delete editor;
	Renderer::Terminate();
	Window::Terminate();
}