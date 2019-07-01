#pragma once

namespace ALZ {

	class Image;


	class Texture {
	public:
		virtual ~Texture() {};
		virtual void SetImage(const Image& image) = 0;
		//creates an image with the specified size that has no data(pixels)
		virtual void SetImage(const glm::vec2& size, int comp = 4) = 0;

		//TODO improve this with flags and stuff
		virtual void SetDefaultTextureSettings() = 0;

		virtual void Bind(const int& slot = 0) const = 0;
		virtual void Unbind(const int& slot = 0) const = 0;
		virtual unsigned int GetRendererID() const = 0;
	};
}