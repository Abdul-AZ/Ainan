#pragma once

namespace ALZ {

	class Texture;

	class FrameBuffer
	{
	public:
		//we take a pointer instead of a refrence because sometimes we need to the render to the default buffer (by passing nullptr)
		//this makes us avoid having to create a buffer for bliting the default buffer
		//basically this function copies the buffer to another buffer
		virtual void Blit(FrameBuffer* otherBuffer, const glm::vec2& sourceSize, const glm::vec2& targetSize) = 0;

		virtual void SetActiveTexture(const Texture& texture) = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual unsigned int GetRendererID() const = 0;
	};
}
