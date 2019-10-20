#include <pch.h>

#include "GaussianBlur.h"

namespace ALZ {

	void GaussianBlur(RenderSurface& surface, float radius)
	{
		Viewport lastViewport = Renderer::GetCurrentViewport();

		Viewport viewport;
		viewport.x = 0;
		viewport.y = 0;
		viewport.width  = (int)surface.GetSize().x;
		viewport.height = (int)surface.GetSize().y;

		Renderer::SetViewport(viewport);
		auto& shader = Renderer::ShaderLibrary["BlurShader"];

		shader->SetUniformVec2("u_Resolution", surface.GetSize());
		shader->SetUniform1f("u_Radius", radius);
		shader->SetUniform1i("u_BlurTarget", 0);

		//Horizontal blur
		static RenderSurface tempSurface;
		tempSurface.SetSize(surface.GetSize());
		tempSurface.SurfaceFrameBuffer->Bind();

		//this specifies that we are doing horizontal blur
		shader->SetUniformVec2("u_BlurDirection", glm::vec2(1.0f, 0.0f));

		//do the horizontal blur to the surface we revieved and put the result in tempSurface
		surface.Render(*shader);

		//this specifies that we are doing vertical blur
		shader->SetUniformVec2("u_BlurDirection", glm::vec2(0.0f, 1.0f));

		//clear the buffer we recieved
		surface.SurfaceFrameBuffer->Bind();
		Renderer::ClearScreen();

		//do the vertical blur to the tempSurface and put the result in the buffer we recieved
		tempSurface.Render(*shader);

		Renderer::SetViewport(lastViewport);
	}
}