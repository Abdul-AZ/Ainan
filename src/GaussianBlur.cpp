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
	FrameBuffer tempFB;
	tempFB.Bind();
	//Horizontal blur
	frameBuffer.Render(Hblur5x5);
	
	//Vertical blur directly to screen buffer
	tempFB.Unbind();
	frameBuffer.Render(Vblur5x5);

}
