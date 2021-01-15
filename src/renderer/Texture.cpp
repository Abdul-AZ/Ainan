#include "Texture.h"

#include "Renderer.h"

namespace Ainan
{
	void Texture::SetImage(std::shared_ptr<Image> image)
	{
		//Misc1 size to be interpreted as glm::vec2, Misc2 is Format and Extradata is a heap allocated array that will be freed by delete[]
		RenderCommand cmd;
		cmd.Type = RenderCommandType::UpdateTexture;
		cmd.UpdateTextureCmdDesc.Texture = &Renderer::Rdata->Textures[Identifier];
		cmd.UpdateTextureCmdDesc.Width = image->m_Width;
		cmd.UpdateTextureCmdDesc.Height = image->m_Height;
		cmd.UpdateTextureCmdDesc.Format = image->Format;
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
		cmd.UpdateTextureCmdDesc.Data = new uint8_t[image->m_Width * image->m_Height * comp];
		memcpy(cmd.UpdateTextureCmdDesc.Data, image->m_Data, sizeof(uint8_t) * image->m_Width * image->m_Height * comp);

		Renderer::PushCommand(cmd);
	}

	uint64_t Texture::GetTextureID()
	{
		auto& data = Renderer::Rdata->Textures[Identifier];
		if (data.Sampler == std::numeric_limits<uint64_t>::max())
			return Renderer::Rdata->Textures[Identifier].Identifier;
		else
			return data.View;
	}
}
