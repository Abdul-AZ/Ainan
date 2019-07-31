#include <pch.h>
#include "Image.h"
#include <thread>

namespace ALZ {

	//this should not be called outside of this file
	//this runs on a seperate thread to not block the program
	//this will call delete[] on the dataCpy, so you should first copy the data to a seperate buffer with new then pass it here.
	//that is to make sure we dont have threading problems
	static void t_SaveToFile(std::string pathAndName, int width, int height, int comp, unsigned char* dataCpy, ImageFormat format)
	{
		stbi_flip_vertically_on_write(true);

		switch (format)
		{
		case ImageFormat::png:
			stbi_write_png((pathAndName + ".png").c_str(), width, height, comp, dataCpy, width * comp);
			break;

		case ImageFormat::jpeg:
			stbi_write_jpg((pathAndName + ".jpg").c_str(), width, height, comp, dataCpy, 100);
			break;

		case ImageFormat::bmp:
			stbi_write_bmp((pathAndName + ".bmp").c_str(), width, height, comp, dataCpy);
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

	Image Image::LoadFromFile(const std::string& pathAndName, int desiredComp)
	{
		Image image;

		image.m_Data = stbi_load(pathAndName.c_str(), &image.m_Width, &image.m_Height, &image.m_Comp, desiredComp);

		return image;
	}

	void Image::SaveToFile(const std::string& pathAndName, const ImageFormat & format)
	{
		unsigned char* dataCpy = new unsigned char[m_Width * m_Height * m_Comp * sizeof(unsigned char)];
		memcpy(dataCpy, m_Data, m_Width * m_Height * m_Comp * sizeof(unsigned char));
		std::thread thread(t_SaveToFile, pathAndName, m_Width, m_Height, m_Comp, dataCpy, format);
		thread.detach();
	}

	Image::Image(const Image& image)
	{
		m_Width = image.m_Width;
		m_Height = image.m_Height;
		m_Comp = image.m_Comp;
		m_Data = new unsigned char[m_Width * m_Height * m_Comp];
		memcpy(m_Data, image.m_Data, m_Width * m_Height * m_Comp * sizeof(unsigned char));
	}

	Image Image::operator=(const Image& image)
	{
		return Image(image);
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

	Image Image::FromFrameBuffer(RenderSurface & framebuffer)
	{
		Image image;
		image.m_Width  = (unsigned int)framebuffer.GetSize().x;
		image.m_Height = (unsigned int)framebuffer.GetSize().y;
		image.m_Data = new unsigned char[image.m_Width * image.m_Height * 4];
		image.m_Comp = 4;

		framebuffer.m_FrameBuffer->Bind();
		glReadPixels(0, 0, image.m_Width, image.m_Height, GL_RGBA, GL_UNSIGNED_BYTE, image.m_Data);

		return image;
	}

	Image Image::FromFrameBuffer(RenderSurface & renderSurface, const unsigned int & width, const unsigned int & height)
	{
		Image image;
		image.m_Width = width;
		image.m_Height = height;
		image.m_Data = new unsigned char[image.m_Width * image.m_Height * 4];
		image.m_Comp = 4;

		RenderSurface tempRenderSurface;
		tempRenderSurface.SetSize(glm::vec2(width, height));
		glBindFramebuffer(GL_READ_FRAMEBUFFER, renderSurface.m_FrameBuffer->GetRendererID());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tempRenderSurface.m_FrameBuffer->GetRendererID());

		glBlitFramebuffer(0, 0, (GLuint)renderSurface.GetSize().x, (GLuint)renderSurface.GetSize().y, 0, 0, (GLuint)tempRenderSurface.GetSize().x, (GLuint)tempRenderSurface.GetSize().y, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		tempRenderSurface.m_FrameBuffer->Bind();
		glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image.m_Data);

		return image;
	}

	Image Image::FromFrameBuffer(RenderSurface& renderSurface, const glm::ivec2& size)
	{
		return FromFrameBuffer(renderSurface, size.x, size.y);
	}
}