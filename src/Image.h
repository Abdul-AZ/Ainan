#pragma once

#include "graphics/RenderSurface.h"
#include "stb/stb_image_write.h"
#include "stb/stb_image.h"

namespace ALZ {

	enum class ImageFormat {
		png,
		jpeg,
		bmp
	};

	class RenderSurface;

	class Image
	{
	public:
		Image() {};
		~Image();

		static Image LoadFromFile(const std::string& pathAndName, int desiredComp = 4);
		void SaveToFile(const std::string& pathAndName, const ImageFormat& format);

		Image(const Image& image);
		Image operator=(const Image& image);

		static std::string GetFormatString(const ImageFormat& format);
		static Image FromFrameBuffer(RenderSurface& framebuffer);
		static Image FromFrameBuffer(RenderSurface& framebuffer, const unsigned int& width, const unsigned int& height);

	public:
		//pointer to the image pixel array
		unsigned char* m_Data = nullptr;

		//3 means RGB and 4 means RGBA
		int m_Comp = 0;

		int m_Width = 0;
		int m_Height = 0;
	};
}