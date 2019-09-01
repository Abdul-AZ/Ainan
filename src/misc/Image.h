#pragma once

#include "misc/RenderSurface.h"
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

		static Image LoadFromFile(const std::string& pathAndName, int desiredComp = 0);
		void SaveToFile(const std::string& pathAndName, const ImageFormat& format);

		Image(const Image& image);
		Image operator=(const Image& image);

		static std::string GetFormatString(const ImageFormat& format);
		static Image FromFrameBuffer(RenderSurface& framebuffer);
		static Image FromFrameBuffer(RenderSurface& framebuffer, const unsigned int& width, const unsigned int& height);
		//for convenience
		static Image FromFrameBuffer(RenderSurface& framebuffer, const glm::ivec2& size);

		//this is because modern OpenGL does not render GrayscaleImages well(as it treats it as RGB but G = 0 and B = 0)
		//that is why we change it to RGB so that grayscale colors are rendered correctly and dont look red
		static void GrayScaleToRGB(Image& image);

	public:
		//pointer to the image pixel array
		unsigned char* m_Data = nullptr;

		//3 means RGB and 4 means RGBA
		int m_Comp = 0;

		int m_Width = 0;
		int m_Height = 0;
	};
}