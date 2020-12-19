#include "Texture.h"

#include "Renderer.h"

namespace Ainan
{
	void TextureNew::SetImage(std::shared_ptr<Image> image)
	{
		//Misc1 size to be interpreted as glm::vec2, Misc2 is Format and Extradata is a heap allocated array that will be freed by delete[]
		RenderCommand cmd;
		cmd.Type = RenderCommandType::UpdateTexture;
		cmd.NewTex = &Renderer::Rdata->Textures[Identifier];
		glm::vec2 size(image->m_Width, image->m_Height);
		memcpy(&cmd.Misc1, &size, sizeof(glm::vec2));
		cmd.Misc2 = (uint64_t)image->Format;
		int32_t comp = 0;
		switch (image->Format)
		{
		case TextureFormat::RGBA:
			comp = 4;
			break;

		case TextureFormat::RGB:
			comp = 3;
			break;

		case TextureFormat::RG:
			comp = 2;
			break;

		case TextureFormat::R:
			comp = 1;
			break;
		default:
			break;
		}
		cmd.ExtraData = new uint8_t[size.x * size.y * comp];
		memcpy(cmd.ExtraData, image->m_Data, sizeof(uint8_t) * size.x * size.y * comp);

		Renderer::PushCommand(cmd);
	}

	uint64_t TextureNew::GetTextureID()
	{
		return Renderer::Rdata->Textures[Identifier].Identifier;
	}
}
