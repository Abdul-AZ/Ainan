#pragma once

#include "renderer/UniformBuffer.h"

namespace Ainan {
	namespace OpenGL {

		class OpenGLUniformBuffer : public UniformBuffer
		{
		public:
			OpenGLUniformBuffer(uint32_t size, void* data);
			virtual ~OpenGLUniformBuffer();

			virtual void UpdateData(void* data) override;
			virtual void Bind(uint32_t index) override;

		private:
			uint32_t m_RendererID;
			uint32_t m_Size;
			std::string m_Name;
		};
	}
}