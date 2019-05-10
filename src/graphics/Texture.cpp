#include <pch.h>

#include "Texture.h"
#include "stb/stb_image.h"

namespace ALZ {

	void Texture::Init(const std::string& pathToImage, const int& bytesPerPixel)
	{
		if (TextureActive)
			Delete();

		int width, height, bytes_per_pixel;
		unsigned char* data = stbi_load(pathToImage.c_str(), &width, &height, &bytes_per_pixel, bytesPerPixel);

		glGenTextures(1, (GLuint*)&TextureID);
		TextureActive = true;
		glBindTexture(GL_TEXTURE_2D, (GLuint)TextureID);

		if (bytesPerPixel == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		else if (bytesPerPixel == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);
	}

	void Texture::Delete()
	{
		glDeleteTextures(1, (GLuint*)&TextureID);
		TextureActive = false;
	}

	Texture::~Texture()
	{
		if (TextureActive)
			Delete();
	}

	void Texture::Bind(const int& slot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, (GLuint)TextureID);
	}

	void Texture::Unbind(const int& slot)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}