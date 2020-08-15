#pragma once

#include "Image.h"

namespace Ainan {

	class Texture;

	class FrameBuffer
	{
	public:
		virtual ~FrameBuffer() {};
		//we take a pointer instead of a refrence because sometimes we need to the render to the default buffer (by passing nullptr)
		//this makes us avoid having to create a buffer for bliting the default buffer
		//basically this function copies the buffer to another buffer
		virtual void Blit(FrameBuffer* otherBuffer, const glm::vec2& sourceSize, const glm::vec2& targetSize) = 0;
		virtual void Resize(const glm::vec2& newSize) = 0;
		virtual void ResizeUnsafe(const glm::vec2& newSize) = 0;

		virtual void* GetTextureID() = 0;
		virtual glm::vec2 GetSize() const = 0;

		//will read the entire framebuffer if no arguments are specified
		virtual Image ReadPixels(glm::vec2 bottomLeftPixel = { 0,0 }, glm::vec2 topRightPixel = { 0,0 }) = 0;

		virtual void Bind() const = 0;
		virtual void BindUnsafe() const = 0;
	};
}
