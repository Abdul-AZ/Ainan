#pragma once

#include <pch.h>

#include "OpenGLRendererContext.h"

namespace Ainan {
	namespace OpenGL {
		RendererType OpenGLRendererContext::GetType() const
		{
			return RendererType::OpenGL;
		}
	}
}