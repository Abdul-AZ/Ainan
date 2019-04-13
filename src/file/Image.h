#include "renderer/FrameBuffer.h"
#include "stb_image_write.h"

namespace ALZ {

	enum class ImageFormat {
		png,
		jpeg,
		bmp
	};

	class Image
	{
	public:
		Image() {};
		~Image();

		void SaveToFile(const std::string& pathAndName, const ImageFormat& format);

		Image(const Image& image);
		Image operator=(const Image& image);

		static std::string GetFormatString(const ImageFormat& format);
		static Image FromFrameBuffer(FrameBuffer& framebuffer);
		static Image FromFrameBuffer(FrameBuffer& framebuffer, const unsigned int& width, const unsigned int& height);

	public:
		//pointer to the image pixel array
		unsigned char* m_Data = nullptr;

		//3 means RGB and 4 means RGBA
		unsigned int m_Comp = 0;

		unsigned int m_Width = 0;
		unsigned int m_Height = 0;
	};
}