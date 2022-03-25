#pragma once

#include "Image.h"

namespace Ainan {

	class Framebuffer
	{
	public:
		//This is used by the renderer to interact with the abstracted uniform buffer
		uint32_t Identifier = 0;

		void Resize(const glm::vec2& newSize);

		void* GetTextureID();
		glm::vec2 GetSize() const;

		//will read the entire framebuffer if no arguments are specified
		//returns an image that should be freed with delete
		Image* ReadPixels(glm::vec2 bottomLeftPixel = { 0,0 }, glm::vec2 topRightPixel = { 0,0 });

		void Bind() const;
	};

	struct FramebufferDataView
	{
		uint64_t Identifier = 0;
		uint64_t TextureIdentifier = 0;
		uint64_t SamplerIdentifier = 0;
		uint64_t ResourceIdentifier = 0;
		glm::vec2 Size;
		bool Deleted = false;
	};
}
