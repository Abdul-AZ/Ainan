#include <pch.h>
#include "Texture.h"

#include "file/stb_image.h"

Texture::Texture(const std::string & pathToImage)
{
	int width, height, bytes_per_pixel;
	unsigned char* data = stbi_load(pathToImage.c_str() , &width, &height, &bytes_per_pixel, 3);
	//TODO change this to work with other than RGB images
	assert(bytes_per_pixel == 3);
	glGenTextures(1, &m_TextureID);
	glBindTexture(GL_TEXTURE_2D, m_TextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_TextureID);
}
