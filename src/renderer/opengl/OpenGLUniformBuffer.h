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

			virtual std::string GetName() const override { return m_Name; };
			virtual uint32_t GetPackedSize() const override { return m_PackedSize; };
			virtual uint32_t GetAlignedSize() const override { return m_AlignedSize; };

		public:
			uint32_t m_RendererID = 0;
			uint32_t m_AlignedSize = 0;
			uint32_t m_PackedSize = 0;
			uint8_t* m_BufferMemory = nullptr;
			VertexLayout m_Layout;
			std::string m_Name = "";
		};
	}
}