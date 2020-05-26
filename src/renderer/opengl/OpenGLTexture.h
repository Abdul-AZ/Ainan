#pragma once

#include "renderer/Texture.h"
#include "renderer/Image.h"

namespace Ainan {
	namespace OpenGL {

		class OpenGLTexture : public Texture
		{
		public:
			OpenGLTexture(const glm::vec2& size, uint8_t* data = nullptr);
			virtual ~OpenGLTexture();

			// Inherited via Texture
			virtual void SetImage(const Image& image) override;
			virtual void SetImage(const glm::vec2& size, int comp = 4) override;
			virtual void Bind(const int& slot = 0) const override;
			virtual void Unbind(const int& slot = 0) const override;
			virtual void SetDefaultTextureSettings() override;
			virtual unsigned int GetRendererID() const override { return m_RendererID; }
			virtual glm::vec2 GetSize() const override { return m_Size; }
		private:
			unsigned int m_RendererID;
			glm::vec2 m_Size = { 0.0f, 0.0f };
		};
	}
}