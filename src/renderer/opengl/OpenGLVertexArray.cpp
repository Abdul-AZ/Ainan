#include <pch.h>
#include <glad/glad.h>


#include "OpenGLVertexArray.h"

namespace Ainan {
	namespace OpenGL {

		OpenGLVertexArray::OpenGLVertexArray()
		{
			glGenVertexArrays(1, &m_RendererID);
		}

		OpenGLVertexArray::~OpenGLVertexArray()
		{
			glDeleteVertexArrays(1, &m_RendererID);
		}

		void OpenGLVertexArray::Bind() const
		{
			glBindVertexArray(m_RendererID);
		}

		void OpenGLVertexArray::Unbind() const
		{
			glBindVertexArray(0);
		}
	}
}