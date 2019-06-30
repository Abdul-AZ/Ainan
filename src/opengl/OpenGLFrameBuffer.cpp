#include <pch.h>
#include <glad/glad.h>

#include "OpenGLFrameBuffer.h"

namespace ALZ {

	OpenGLFrameBuffer::OpenGLFrameBuffer()
	{
		glGenFramebuffers(1, &m_RendererID);
	}

	OpenGLFrameBuffer::~OpenGLFrameBuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
	}

	void OpenGLFrameBuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
	}

	void OpenGLFrameBuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFrameBuffer::SetActiveTexture(const Texture& texture)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.GetRendererID(), 0);
	}

	void OpenGLFrameBuffer::Blit(FrameBuffer* otherBuffer, const glm::vec2& sourceSize, const glm::vec2& targetSize)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_RendererID);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, otherBuffer ? otherBuffer->GetRendererID() : 0);

		glBlitFramebuffer(0, 0, (GLint)sourceSize.x, (GLint)sourceSize.y,
						  0, 0, (GLint)targetSize.x, (GLint)targetSize.y,
			GL_COLOR_BUFFER_BIT,
			GL_LINEAR);
	}
}