#include <pch.h>

#include "misc/Window.h"
#include "misc/Environment.h"
#include "misc/StartMenu.h"
#include "renderer/Renderer.h"

void clearArea(const ALZ::Viewport& vp)
{
	glEnable(GL_SCISSOR_TEST);
	glScissor(vp.x, vp.y, vp.width, vp.height);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
}

int main(int argc, const char* argv[]) {

	using namespace ALZ;

	Window::Init();
	FileManager::Init(argv[0]);
	Renderer::Init();
	
	ImGuiWrapper::Init();
	SetEditorStyle(EditorStyle::Dark_Gray);

	Environment* env = nullptr;
	StartMenu startMenu;

	while (Window::ShouldClose == false)
	{
		Window::HandleWindowEvents();

		if (env) 
		{
			env->Update();
			env->HandleInput();
			env->Render();

			int redraws = ImGuiWrapper::RedrawsRequired();
			env->RenderGUI(redraws > 0);
		
			if (env->ShouldDelete) 
			{
				delete env;
				env = nullptr;
			}

			Window::Present();

			//if (redraws > 1)
			//	Window::Clear();
			//else
			//	clearArea(env->m_ViewportWindow.RenderViewport);
		}
		else 
		{
			startMenu.Update(env);
			Window::Present();
			Window::Clear();
		}

	}

	if(env)
		delete env;

	Renderer::Terminate();
	ImGuiWrapper::Terminate();
	Window::Terminate();
}