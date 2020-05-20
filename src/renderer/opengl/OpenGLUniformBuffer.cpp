#include <pch.h>
#include <glad/glad.h>

#include "OpenGLUniformBuffer.h"

namespace Ainan {
	namespace OpenGL {

		OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, void* data) :
			m_Size(size)
		{
			glGenBuffers(1, &m_RendererID);
			glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
			glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		OpenGLUniformBuffer::~OpenGLUniformBuffer()
		{
			glDeleteBuffers(1, &m_RendererID);
		}

		void OpenGLUniformBuffer::UpdateData(void* data)
		{
			glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, m_Size, data);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		void OpenGLUniformBuffer::Bind(uint32_t index)
		{
			glBindBufferRange(GL_UNIFORM_BUFFER, index, m_RendererID, 0, m_Size);
		}
	}
}