#pragma once

#include "graphics/IndexBuffer.h"

namespace ALZ {

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(unsigned int* data, const int& count);
		~OpenGLIndexBuffer();

		// Inherited via IndexBuffer
		virtual unsigned int GetRendererID() override { return m_RendererID; };
		virtual int GetCount() const override { return m_Count; };
		virtual void Bind() const override;
		virtual void Unbind() const override;

	private:
		unsigned int m_RendererID;
		unsigned int m_Count;
	};
}