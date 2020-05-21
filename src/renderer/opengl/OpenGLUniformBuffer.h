#pragma once

#include "renderer/UniformBuffer.h"
#include "renderer/VertexBuffer.h"//for VertexLayout

namespace Ainan {
	namespace OpenGL {

		class OpenGLUniformBuffer : public UniformBuffer
		{
		public:
			OpenGLUniformBuffer(const std::string& name, const VertexLayout& layout, void* data);
			virtual ~OpenGLUniformBuffer();

			virtual void UpdateData(void* data) override;
			virtual void Bind(uint32_t index) override;

		private:
			uint32_t m_RendererID = 0;
			uint32_t m_Size = 0;
			uint8_t* m_BufferMemory = nullptr;
			VertexLayout m_Layout;
			std::string m_Name = "";
		};
	}
}