#pragma once

#include "renderer/Texture.h"
#include "misc/Image.h"

namespace Ainan {
	namespace OpenGL {

		class OpenGLTexture : public Texture
		{
		public:
			OpenGLTexture();
			virtual ~OpenGLTexture();

			// Inherited via Texture
			virtual void SetImage(const Image& image) override;
			virtual void SetImage(const glm::vec2& size, int comp = 4) override;
			virtual void Bind(const int& slot = 0) const override;
			virtual void Unbind(const int& slot = 0) const override;
			virtual unsigned int GetRendererID() const override { return m_RendererID; }
		private:
			unsigned int m_RendererID;
			glm::vec2 m_Size = { 0.0f, 0.0f };

			// Inherited via Texture
			virtual void SetDefaultTextureSettings() override;

			// Inherited via Texture
			virtual glm::vec2 GetSize() const override { return m_Size; }
		};
	}
}