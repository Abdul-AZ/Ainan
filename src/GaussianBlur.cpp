#include "GaussianBlur.h"


ShaderProgram GaussianBlur::Hblur5x5;
ShaderProgram GaussianBlur::Vblur5x5;

void GaussianBlur::Init()
{
	Hblur5x5.Init("shaders/Image.vert", "shaders/GaussianBlur5x5H.frag");
	Vblur5x5.Init("shaders/Image.vert", "shaders/GaussianBlur5x5V.frag");
}

void GaussianBlur::BlurAndRenderToScreen(FrameBuffer & frameBuffer)
{
	//TODO make this changable in GUI
	float scale = 1.5;

	glm::vec2 normal = { Window::GetSize().x , Window::GetSize().y };
	glm::vec2 downsampled = { Window::GetSize().x / scale , Window::GetSize().y / scale };

	glViewport(0, 0, downsampled.x, downsampled.y);
	FrameBuffer tempFB;
	tempFB.SetSize(downsampled);
	tempFB.Bind();
	//Horizontal blur
	frameBuffer.Render(Hblur5x5);
	
	//Vertical blur directly to screen buffer
	FrameBuffer tempFB2;
	tempFB2.SetSize(downsampled);
	tempFB2.Bind();
	frameBuffer.Render(Vblur5x5);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, tempFB2.m_RendererID); // READ:  Supersampled
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);               // WRITE: Default

	// Downsample the supersampled FBO using LINEAR interpolation
	glBlitFramebuffer(0, 0, downsampled.x, downsampled.y,
		0, 0, normal.x, normal.y,
		GL_COLOR_BUFFER_BIT,
		GL_LINEAR);

	glViewport(0, 0, normal.x, normal.y);
}
