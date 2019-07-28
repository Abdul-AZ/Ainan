#include <pch.h>
#include "GaussianBlur.h"

namespace ALZ {

	ShaderProgram* BlurShader;

	void GaussianBlur::Init()
	{
		BlurShader = Renderer::CreateShaderProgram("shaders/Image.vert", "shaders/Blur.frag").release();
	}

	void GaussianBlur::Blur(RenderSurface& surface, float radius)
	{
		BlurShader->SetUniformVec2("u_Resolution", surface.GetSize());
		BlurShader->SetUniform1f("u_Radius", radius);
		BlurShader->SetUniform1i("u_BlurTarget", 0);

		//Horizontal blur
		static RenderSurface tempSurface;
		tempSurface.SetSize(surface.GetSize());
		tempSurface.m_FrameBuffer->Bind();

		//this specifies that we are doing horizontal blur
		BlurShader->SetUniformVec2("u_BlurDirection", glm::vec2(1.0f, 0.0f));

		//do the horizontal blur to the surface we revieved and put the result in tempSurface
		surface.Render(*BlurShader);

		//this specifies that we are doing vertical blur
		BlurShader->SetUniformVec2("u_BlurDirection", glm::vec2(0.0f, 1.0f));

		//clear the buffer we recieved
		surface.m_FrameBuffer->Bind();
		Renderer::ClearScreen();

		//do the vertical blur to the tempSurface and put the result in the buffer we recieved
		tempSurface.Render(*BlurShader);
	}
}