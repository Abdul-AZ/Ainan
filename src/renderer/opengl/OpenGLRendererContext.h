#pragma once

#include "renderer/RendererContext.h"

namespace Ainan {
	namespace OpenGL {
		class OpenGLRendererContext : public RendererContext
		{
			virtual RendererType GetType() const override;
		};
	}
}