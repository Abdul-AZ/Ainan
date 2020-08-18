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
	InputManager::Init();
	
	Editor* editor = new Editor;
	
	while (Window::ShouldClose == false)
	{
		editor->StartFrame();
		
		Window::HandleWindowEvents();
		editor->Update();
		editor->Draw();
		
		editor->EndFrame();
	
		Renderer::Present();
	}
	
	delete editor;
	
	InputManager::Terminate();
	Renderer::Terminate();
	Window::Terminate();
}