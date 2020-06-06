#pragma once

#include "renderer/IndexBuffer.h"

namespace Ainan {
	namespace OpenGL {

		class OpenGLIndexBuffer : public IndexBuffer
		{
		public:
			OpenGLIndexBuffer(unsigned int* data, const int& count);
			~OpenGLIndexBuffer();

			// Inherited via IndexBuffer
			virtual uint32_t GetCount() const override { return m_Count; };
			virtual void Bind() const override;
			virtual void Unbind() const override;

		private:
			unsigned int m_RendererID;
			unsigned int m_Count;
		};
	}
}