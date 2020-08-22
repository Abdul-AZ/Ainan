#pragma once

#include "renderer/VertexBuffer.h"
#include "renderer/ShaderProgram.h"
#include "renderer/Renderer.h"

namespace Ainan
{
	namespace OpenGL {
		class OpenGLVertexBuffer : public VertexBuffer
		{
		public:
			//size is in bytes
			OpenGLVertexBuffer(void* data, uint32_t size, const VertexLayout& layout, bool dynamic);
			~OpenGLVertexBuffer();

			virtual void UpdateData(int32_t offset, int32_t size, void* data) override;
			virtual void UpdateDataUnsafe(int32_t offset, int32_t size, void* data) override;
			virtual uint32_t GetUsedMemory() const override { return Memory; };
			virtual void Bind() const override;
			virtual void Unbind() const override;

		private:
			uint32_t m_RendererID;
			uint32_t m_VertexArray;
			uint32_t Memory;
		};
	}
}