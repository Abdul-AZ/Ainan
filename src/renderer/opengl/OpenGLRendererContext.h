#pragma once

#include "renderer/RendererContext.h"

namespace Ainan {
	namespace OpenGL {
		class OpenGLRendererContext : public RendererContext
		{
			virtual RendererType GetType() const override;
			virtual std::string GetVersionString() override;
			virtual std::string GetPhysicalDeviceName() override;

			std::string OpenGLVersion;
			std::string PhysicalDeviceName;

			friend class OpenGLRendererAPI;
		};
	}
}