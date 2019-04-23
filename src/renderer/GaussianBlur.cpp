#include <pch.h>
#include "GaussianBlur.h"

namespace ALZ {

	ShaderProgram GaussianBlur::Hblur5x5;
	ShaderProgram GaussianBlur::Vblur5x5;

	void GaussianBlur::Init()
	{
		Hblur5x5.Init("shaders/Image.vert", "shaders/GaussianBlur5x5H.frag");
		Vblur5x5.Init("shaders/Image.vert", "shaders/GaussianBlur5x5V.frag");
	}

	void GaussianBlur::Blur(FrameBuffer & frameBuffer, const float& scale, const float& blurScale, const float& sigma)
	{
		glm::vec2 normal = { Window::GetSize().x , Window::GetSize().y };
		glm::vec2 downsampled = { Window::GetSize().x / scale , Window::GetSize().y / scale };
		glViewport(0, 0, downsampled.x, downsampled.y);

		float BlurPixelValues[3];
		for (int i = 0; i < 3; i++)
		{
			BlurPixelValues[i] = GaussianDistribution(i, sigma) * blurScale;
		}
		Hblur5x5.setUniform1fs("BlurStrength", BlurPixelValues, 3);
		Vblur5x5.setUniform1fs("BlurStrength", BlurPixelValues, 3);


		//Horizontal blur
		FrameBuffer tempFB;
		tempFB.SetSize(downsampled);
		tempFB.Bind();
		frameBuffer.Render(Hblur5x5);

		//Vertical blur directly to screen buffer
		FrameBuffer tempFB2;
		tempFB2.SetSize(downsampled);
		tempFB2.Bind();
		frameBuffer.Render(Vblur5x5);

		frameBuffer.Bind();
		glClear(GL_COLOR_BUFFER_BIT);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, tempFB2.RendererID); // READ:  Supersampled
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer.RendererID);					  // WRITE: Default

		// Downsample the supersampled FBO using LINEAR interpolation
		glBlitFramebuffer(0, 0, downsampled.x, downsampled.y,
			0, 0, normal.x, normal.y,
			GL_COLOR_BUFFER_BIT,
			GL_LINEAR);

		glViewport(0, 0, normal.x, normal.y);
	}

	float GaussianBlur::GaussianDistribution(float x, float sigma)
	{
		return (1 / (sigma * sqrt(2 * 3.14159265359)) * std::exp(-(x * x) / (2 * sigma * sigma)));
	}
}