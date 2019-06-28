#include <pch.h>

#include "OpenGLTexture.h"

namespace ALZ {

	OpenGLTexture::OpenGLTexture()
	{
		glGenTextures(1, &m_RendererID);
	}

	OpenGLTexture::~OpenGLTexture()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture::SetImage(const Image& image)
	{
		Bind();
		
		if (image.m_Comp == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.m_Width, image.m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, image.m_Data);
		else if (image.m_Comp == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.m_Width, image.m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.m_Data);
		else
			assert(false);

		glGenerateMipmap(GL_TEXTURE_2D);
	}

	void OpenGLTexture::Bind(const int& slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
	}

	void OpenGLTexture::Unbind(const int& slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}