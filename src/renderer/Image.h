#pragma once

#include "stb/stb_image_write.h"
#include "stb/stb_image.h"

namespace Ainan {

	enum class ImageFormat 
	{
		png,
		jpeg,
		bmp
	};

	enum class TextureFormat
	{
		RGBA,
		RGB,
		RG,
		R,
		Unspecified
	};

	constexpr uint32_t GetBytesPerPixel(TextureFormat format)
	{
		switch (format)
		{
		case Ainan::TextureFormat::RGBA:
			return 4;
			break;

		case Ainan::TextureFormat::RGB:
			return 3;
			break;

		case Ainan::TextureFormat::RG:
			return 2;
			break;

		case Ainan::TextureFormat::R:
			return 1;
			break;

		case Ainan::TextureFormat::Unspecified:
		default:
			assert(false);
			return 0;
			break;
		}
	}


	class RenderSurface;

	class Image
	{
	public:
		Image() {};
		~Image();

		static Image LoadFromFile(const std::string& pathAndName, TextureFormat desiredFormat = TextureFormat::Unspecified);
		void SaveToFile(const std::string& pathAndName, const ImageFormat& format);

		Image(const Image& image);
		Image operator=(const Image& image);

		void FlipHorizontally();

		static std::string GetFormatString(const ImageFormat& format);

		//this is because modern OpenGL does not render GrayscaleImages well(as it treats it as RGB but G = 0 and B = 0)
		//that is why we change it to RGB so that grayscale colors are rendered correctly and dont look red
		static void GrayScaleToRGB(Image& image);
		static void GrayScaleToRGBA(Image& image);

	public:
		//pointer to the image pixel array
		unsigned char* m_Data = nullptr;

		TextureFormat Format;

		int m_Width = 0;
		int m_Height = 0;
	};
}