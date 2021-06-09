#include "Texture.h"

#include "Renderer.h"

namespace Ainan
{
	void Texture::UpdateData(std::shared_ptr<Image> image)
	{
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

	void Texture::UpdateData(std::array<Image, 6> images)
	{
		assert(Renderer::Rdata->Textures[Identifier].Type == TextureType::Cubemap);

		RenderCommand cmd;
		cmd.Type = RenderCommandType::UpdateTexture;
		cmd.UpdateTextureCmdDesc.Texture = &Renderer::Rdata->Textures[Identifier];
		cmd.UpdateTextureCmdDesc.Width = images[0].m_Width;
		cmd.UpdateTextureCmdDesc.Height = images[0].m_Height;
		cmd.UpdateTextureCmdDesc.Format = images[0].Format;
		int32_t bpp = GetBytesPerPixel(images[0].Format);
		cmd.UpdateTextureCmdDesc.Data = new uint8_t[images[0].m_Width * images[0].m_Height * bpp * images.size()];
		for (size_t i = 0; i < images.size(); i++)
			memcpy((uint8_t*)cmd.UpdateTextureCmdDesc.Data + i * images[0].m_Width * images[0].m_Height * bpp, images[i].m_Data,
				sizeof(uint8_t) * images[0].m_Width * images[0].m_Height * bpp);

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
