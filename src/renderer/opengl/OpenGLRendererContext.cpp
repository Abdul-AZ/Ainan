#include "OpenGLRendererContext.h"

namespace Ainan {
	namespace OpenGL {
		RendererType OpenGLRendererContext::GetType() const
		{
			return RendererType::OpenGL;
		}

		std::string OpenGLRendererContext::GetVersionString()
		{
			return OpenGLVersion;
		}

		std::string OpenGLRendererContext::GetPhysicalDeviceName()
		{
			return PhysicalDeviceName;
		}
	}
}