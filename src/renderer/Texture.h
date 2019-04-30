#pragma once

namespace ALZ {

	class Texture {
	public:
		void Init(const std::string& pathToImage, const int& bytesPerPixel);
		void Delete();
		~Texture();

		void Bind(const int& slot = 0);
		void Unbind(const int& slot = 0);

	public:
		unsigned int TextureID = 0;
		bool TextureActive = false;
	};
}