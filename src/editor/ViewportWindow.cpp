#include "ViewportWindow.h"
#include "renderer/Renderer.h"

#include <glad/glad.h>

namespace Ainan {

	void ViewportWindow::DisplayGUI(FrameBufferNew fb)
	{
		ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Viewport", nullptr);

		WindowPosition = { ImGui::GetWindowPos().x, ImGui::GetWindowPos().y };
		WindowSize = { ImGui::GetWindowSize().x, ImGui::GetWindowSize().y };
		WindowContentRegionSize = { ImGui::GetContentRegionMax().x, ImGui::GetContentRegionMax().y };

		ImGui::DockBuilderGetNode(ImGui::GetWindowDockID())->LocalFlags &= ~ImGuiDockNodeFlags_NoTabBar;

		RenderViewport.Width = ImGui::GetWindowContentRegionMax().x;
		RenderViewport.Height = ImGui::GetWindowContentRegionMax().y - ImGui::GetFrameHeight();

		RenderViewport.X = 0;
		RenderViewport.Y = 0;

		ImVec2 uv0;
		ImVec2 uv1;

		auto data = Renderer::Rdata;

		//flip horizontally if we are in OpenGL
		if (data->CurrentActiveAPI->GetContext()->GetType() == RendererType::OpenGL)
		{
			uv0 = ImVec2(0, 1);
			uv1 = ImVec2(1, 0);
		}
		else
		{
			uv0 = ImVec2(0, 0);
			uv1 = ImVec2(1, 1);
		}

		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(), ImVec2(Window::Size.x, Window::Size.y), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)));
		ImGui::Image(fb.GetTextureID(), ImVec2(RenderViewport.Width, RenderViewport.Height), uv0, uv1);

		IsHovered = ImGui::IsWindowHovered();
		IsFocused = ImGui::IsWindowFocused();
		ImGui::End();
		ImGui::PopStyleVar();
	}
}