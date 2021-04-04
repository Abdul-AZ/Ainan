#include "editor/Window.h"
#include "editor/Editor.h"
#include "editor/EditorPreferences.h"
#include "renderer/Renderer.h"

int main() 
{
	using namespace Ainan;

#ifndef NDEBUG
	InitAinanLogger();;
#endif // !NDEBUG

	auto api = EditorPreferences::LoadFromDefaultPath().RenderingBackend;

	Window::Init(api);
	Renderer::Init(api);
	
	Editor* editor = new Editor;
	InputManager::Init();

	while (Window::ShouldClose == false)
	{
		Window::HandleWindowEvents();
		editor->Update();
		InputManager::HandleInput();
		editor->Draw();

		if (editor->NeedToPresent())
			Renderer::Present();
		else
			Renderer::SleepExtraFrametime();
	}
	
	InputManager::Terminate();
	delete editor;
	Renderer::Terminate();
	Window::Terminate();
}