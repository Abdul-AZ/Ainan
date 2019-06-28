#pragma once

namespace ALZ {

	class Image;

	class Texture {
	public:
		virtual void SetImage(const Image& image) = 0;

		virtual void Bind(const int& slot = 0) = 0;
		virtual void Unbind(const int& slot = 0) = 0;
		virtual unsigned int GetRendererID() = 0;
	};
}