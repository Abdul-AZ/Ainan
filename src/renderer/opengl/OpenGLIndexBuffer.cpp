#include <glad/glad.h>

#include "OpenGLIndexBuffer.h"

namespace Ainan {
	namespace OpenGL {

		OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* data, uint32_t count) :
			m_Count(count),
			Memory(count * sizeof(uint32_t))
		{
			glGenBuffers(1, &m_RendererID);
			Bind();
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, Memory, data, GL_DYNAMIC_DRAW);
		}

		OpenGLIndexBuffer::~OpenGLIndexBuffer()
		{
			glDeleteBuffers(1, &m_RendererID);
		}

		void OpenGLIndexBuffer::Bind() const
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		}

		void OpenGLIndexBuffer::Unbind() const
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
	}
}