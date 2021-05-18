#include "Image.h"
#include <thread>

namespace Ainan {

	//this should not be called outside of this file
	//this runs on a seperate thread to not block the program
	//this will call delete[] on the dataCpy, so you should first copy the data to a seperate buffer with new then pass it here.
	//that is to make sure we dont have threading problems
	static void t_SaveToFile(std::string path, int width, int height, int comp, unsigned char* dataCpy, ImageFormat format)
	{
		stbi_flip_vertically_on_write(true);

		switch (format)
		{
		case ImageFormat::png:
			stbi_write_png(path.c_str(), width, height, comp, dataCpy, width * comp);
			break;

		case ImageFormat::jpeg:
			stbi_write_jpg(path.c_str(), width, height, comp, dataCpy, 100);
			break;

		case ImageFormat::bmp:
			stbi_write_bmp(path.c_str(), width, height, comp, dataCpy);
			break;

		default:
			break;
		}

		delete[] dataCpy;
	}

	Image::~Image()
	{
		if (m_Data)
			delete[] m_Data;
	}

	Image Image::LoadFromFile(const std::string& pathAndName, TextureFormat desiredFormat, bool flip)
	{
		Image image;

		int comp = 0;

		stbi_set_flip_vertically_on_load(flip);

		if(desiredFormat == TextureFormat::RGBA)
			image.m_Data = stbi_load(pathAndName.c_str(), &image.m_Width, &image.m_Height, &comp, 4);
		else if(desiredFormat == TextureFormat::RGB)
			image.m_Data = stbi_load(pathAndName.c_str(), &image.m_Width, &image.m_Height, &comp, 3);
		else
			image.m_Data = stbi_load(pathAndName.c_str(), &image.m_Width, &image.m_Height, &comp, 0);

		if (image.m_Data == nullptr)
			AINAN_LOG_FATAL("Could not load image");

		if (desiredFormat != TextureFormat::Unspecified)
			image.Format = desiredFormat;
		else 
		{
			if (comp == 4)
				image.Format = TextureFormat::RGBA;
			else if (comp == 3)
				image.Format = TextureFormat::RGB;
			else if (comp == 2)
				image.Format = TextureFormat::RG;
			else if (comp == 1)
				image.Format = TextureFormat::R;
			else
				assert(false);
		}

		return image;
	}

	void Image::SaveToFile(const std::string& path, const ImageFormat& format)
	{
		uint32_t comp = GetBytesPerPixel(Format);

		unsigned char* dataCpy = new unsigned char[m_Width * m_Height * comp * sizeof(unsigned char)];
		memcpy(dataCpy, m_Data, m_Width * m_Height * comp * sizeof(unsigned char));
		std::thread thread(t_SaveToFile, path, m_Width, m_Height, comp, dataCpy, format);
		thread.detach();
	}

	Image::Image(const Image& image)
	{
		m_Width = image.m_Width;
		m_Height = image.m_Height;
		Format = image.Format;
		m_Data = new unsigned char[m_Width * m_Height * GetBytesPerPixel(Format)];
		memcpy(m_Data, image.m_Data, m_Width * m_Height * GetBytesPerPixel(Format) * sizeof(unsigned char));
	}

	Image Image::operator=(const Image& image)
	{
		return Image(image);
	}

	void Image::FlipHorizontally()
	{
		uint32_t comp = GetBytesPerPixel(Format);

		uint32_t byteCount = m_Width * m_Height * comp;
		uint32_t rowSize = m_Width * comp;
		unsigned char* buffer = new unsigned char[byteCount];
		memcpy(buffer, m_Data, byteCount);

		for (size_t y = 0; y < m_Height; y++)
		{
			memcpy(&m_Data[y * rowSize], &buffer[rowSize * ( m_Height - y - 1)], rowSize);
		}

		delete[] buffer;
	}

	std::string Image::GetFormatString(const ImageFormat & format)
	{
		switch (format)
		{
		case ImageFormat::png:
			return "png";

		case ImageFormat::bmp:
			return "bmp";

		case ImageFormat::jpeg:
			return "jpeg";
		default:
			return "";
		}
	}

	void Image::GrayScaleToRGB(Image& image)
	{
		assert(image.Format == TextureFormat::R);

		size_t pixelCount = image.m_Width * image.m_Height;

		uint8_t* buffer = (uint8_t* )malloc(sizeof(uint8_t) * pixelCount * 3);

		for (size_t i = 0; i < pixelCount; i++)
		{
			buffer[i * 3] = image.m_Data[i];
			buffer[i * 3 + 1] = image.m_Data[i];
			buffer[i * 3 + 2] = image.m_Data[i];
		}

		image.Format = TextureFormat::RGB;
		free(image.m_Data);
		image.m_Data = buffer;
	}

	void Image::GrayScaleToRGBA(Image& image)
	{
		assert(image.Format == TextureFormat::R);

		size_t pixelCount = image.m_Width * image.m_Height;

		uint8_t* buffer = (uint8_t*)malloc(sizeof(uint8_t) * pixelCount * 4);

		for (size_t i = 0; i < pixelCount; i++)
		{
			buffer[i * 4] = image.m_Data[i];
			buffer[i * 4 + 1] = image.m_Data[i];
			buffer[i * 4 + 2] = image.m_Data[i];
			buffer[i * 4 + 3] = 255;
		}

		image.Format = TextureFormat::RGBA;
		free(image.m_Data);
		image.m_Data = buffer;
	}
}