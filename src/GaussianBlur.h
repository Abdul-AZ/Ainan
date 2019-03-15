#pragma once

#include "FrameBuffer.h"

class GaussianBlur {
public:
	static void Init();
	static void BlurAndRenderToScreen(FrameBuffer& frameBuffer);

private:
	static ShaderProgram Hblur5x5;
	static ShaderProgram Vblur5x5;
};