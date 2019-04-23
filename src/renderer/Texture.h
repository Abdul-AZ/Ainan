#pragma once

class Texture {
public:
	Texture(const std::string& pathToImage);
	~Texture();
	unsigned int GetID() { return m_TextureID; }

private:
	unsigned int m_TextureID;
};