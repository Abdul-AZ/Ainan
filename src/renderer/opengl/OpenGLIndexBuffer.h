#pragma once

#include "renderer/IndexBuffer.h"

namespace Ainan {
	namespace OpenGL {

		class OpenGLIndexBuffer : public IndexBuffer
		{
		public:
			OpenGLIndexBuffer(uint32_t* data, uint32_t count);
			~OpenGLIndexBuffer();

			// Inherited via IndexBuffer
			virtual uint32_t GetCount() const override { return m_Count; };
			virtual uint32_t GetUsedMemory() const override { return Memory; };
			virtual void Bind() const override;
			virtual void Unbind() const override;

		private:
			uint32_t m_RendererID;
			uint32_t m_Count;
			uint32_t Memory;
		};
	}
}