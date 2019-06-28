#pragma once

#include "graphics/Texture.h"
#include "Image.h"

namespace ALZ {

	class OpenGLTexture : public Texture
	{
	public:
		OpenGLTexture();
		~OpenGLTexture();

		// Inherited via Texture
		virtual void SetImage(const Image& image) override;
		virtual void Bind(const int& slot = 0) override;
		virtual void Unbind(const int& slot = 0) override;
		virtual unsigned int GetRendererID() override { return m_RendererID; }
	private:
		unsigned int m_RendererID;
	};

}