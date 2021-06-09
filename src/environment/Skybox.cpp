#include "Skybox.h"

#include "file/AssetManager.h"

namespace Ainan {
	
	std::string SkyModeStr(SkyMode mode)
	{
		switch (mode)
		{
		case SkyMode::FlatColor:
			return "Flat Color";
		case SkyMode::CubemapTexture:
			return "Cubemap Texture";
		}

		AINAN_LOG_FATAL("Invalid Skymode Set");
		return nullptr;
	}

	SkyMode SkyModeFromStr(const std::string& str)
	{
		if (str == "Flat Color")
			return SkyMode::FlatColor;
		else if (str == "Cubemap Texture")
			return SkyMode::CubemapTexture;
		
		AINAN_LOG_FATAL("Invalid Skymode Set");
		return SkyMode::FlatColor;
	}

	Skybox::Skybox()
	{
		float skyboxVertices[] = 
		{
			// positions          
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f
		};

		std::array<Image, 6> faces =
		{
			faces[0] = Image::FromColor(m_SkyboxColor, TextureFormat::RGBA, glm::vec2(1028, 1028)),
			faces[1] = Image::FromColor(m_SkyboxColor, TextureFormat::RGBA, glm::vec2(1028, 1028)),
			faces[2] = Image::FromColor(m_SkyboxColor, TextureFormat::RGBA, glm::vec2(1028, 1028)),
			faces[3] = Image::FromColor(m_SkyboxColor, TextureFormat::RGBA, glm::vec2(1028, 1028)),
			faces[5] = Image::FromColor(m_SkyboxColor, TextureFormat::RGBA, glm::vec2(1028, 1028)),
			faces[4] = Image::FromColor(m_SkyboxColor, TextureFormat::RGBA, glm::vec2(1028, 1028)),
		};

		m_Cubemap = Renderer::CreateCubemapTexture(faces);
		VertexLayout layout =
		{
			VertexLayoutElement("POSITION", 0, ShaderVariableType::Vec3)
		};
		m_VertexBuffer = Renderer::CreateVertexBuffer(skyboxVertices, sizeof(skyboxVertices),
			layout, Renderer::ShaderLibrary()["SkyboxShader"]);

		layout =
		{
			VertexLayoutElement("POSITION", 0, ShaderVariableType::Mat4)
		};
		m_UniformBuffer = Renderer::CreateUniformBuffer("SkyboxTransform", 1, layout);
	}

	void Skybox::Draw(const Camera& camera)
	{
		//remove translation and keep rotation
		glm::mat4 u_ViewProjection = camera.GetProjectionMatrix() * glm::mat4(glm::mat3(camera.GetViewMatrix()));
		m_UniformBuffer.UpdateData(&u_ViewProjection, sizeof(u_ViewProjection));

		Renderer::ShaderLibrary()["SkyboxShader"].BindTexture(m_Cubemap, 0, RenderingStage::FragmentShader);
		Renderer::ShaderLibrary()["SkyboxShader"].BindUniformBuffer(m_UniformBuffer, 1, RenderingStage::VertexShader);
		Renderer::Draw(m_VertexBuffer, Renderer::ShaderLibrary()["SkyboxShader"], Primitive::Triangles, 36);
	}

	void Skybox::DisplayGUI()
	{
		if (!SkyboxWindowOpen)
			return;

		ImGui::Begin("Sky", &SkyboxWindowOpen);

		IMGUI_DROPDOWN_START("Sky Mode: ", SkyModeStr(m_Mode).c_str());

		IMGUI_DROPDOWN_SELECTABLE(m_Mode, SkyMode::FlatColor, SkyModeStr(SkyMode::FlatColor).c_str());
		IMGUI_DROPDOWN_SELECTABLE(m_Mode, SkyMode::CubemapTexture, SkyModeStr(SkyMode::CubemapTexture).c_str());

		IMGUI_DROPDOWN_END();

		if (m_Mode == SkyMode::FlatColor)
		{
			if (ImGui::ColorEdit4("SkyboxColor", &m_SkyboxColor.r))
			{
				std::array<Image, 6> faces =
				{
					Image::FromColor(m_SkyboxColor, TextureFormat::RGBA, glm::vec2(1028, 1028)),
					Image::FromColor(m_SkyboxColor, TextureFormat::RGBA, glm::vec2(1028, 1028)),
					Image::FromColor(m_SkyboxColor, TextureFormat::RGBA, glm::vec2(1028, 1028)),
					Image::FromColor(m_SkyboxColor, TextureFormat::RGBA, glm::vec2(1028, 1028)),
					Image::FromColor(m_SkyboxColor, TextureFormat::RGBA, glm::vec2(1028, 1028)),
					Image::FromColor(m_SkyboxColor, TextureFormat::RGBA, glm::vec2(1028, 1028))
				};
				m_Cubemap.UpdateData(faces);
			}
		}
		else if (m_Mode == SkyMode::CubemapTexture)
		{
			auto displayDropDown = [&](std::filesystem::path& path, const std::string& label)
			{
				if (ImGui::BeginCombo(label.c_str(), path == "" ? "None" : path.lexically_relative(AssetManager::s_EnvironmentDirectory).u8string().c_str()))
				{
					if (path != "")
					{
						bool selected = false;

						if (ImGui::Selectable("None", &selected))
						{
							path = "";
						}
					}

					for (auto& imagePath : AssetManager::Images)
					{
						bool selected = path == imagePath;
						std::string textureFileName = std::filesystem::path(imagePath).filename().u8string();

						if (ImGui::Selectable(textureFileName.c_str(), &selected))
						{
							path = imagePath;
							
							//TODO remove this hacky way of calculating the biggest dimensions and load the images only once
							int32_t biggestWidth = 0;
							int32_t biggestHeight = 0;
							{
								Image faces[6] =
								{
										m_TexturePaths[0] == "" ?
										Image::FromColor(glm::vec4(0, 0, 0, 0), TextureFormat::RGBA, glm::vec2(1, 1)) :
										Image::LoadFromFile(m_TexturePaths[0].u8string(), TextureFormat::RGBA),

										m_TexturePaths[1] == "" ?
										Image::FromColor(glm::vec4(0, 0, 0, 0), TextureFormat::RGBA, glm::vec2(1, 1)) :
										Image::LoadFromFile(m_TexturePaths[1].u8string(), TextureFormat::RGBA),

										m_TexturePaths[2] == "" ?
										Image::FromColor(glm::vec4(0, 0, 0, 0), TextureFormat::RGBA, glm::vec2(1, 1)) :
										Image::LoadFromFile(m_TexturePaths[2].u8string(), TextureFormat::RGBA),

										m_TexturePaths[3] == "" ?
										Image::FromColor(glm::vec4(0, 0, 0, 0), TextureFormat::RGBA, glm::vec2(1, 1)) :
										Image::LoadFromFile(m_TexturePaths[3].u8string(), TextureFormat::RGBA),

										m_TexturePaths[4] == "" ?
										Image::FromColor(glm::vec4(0, 0, 0, 0), TextureFormat::RGBA, glm::vec2(1, 1)) :
										Image::LoadFromFile(m_TexturePaths[4].u8string(), TextureFormat::RGBA),

										m_TexturePaths[5] == "" ?
										Image::FromColor(glm::vec4(0, 0, 0, 0), TextureFormat::RGBA, glm::vec2(1, 1)) :
										Image::LoadFromFile(m_TexturePaths[5].u8string(), TextureFormat::RGBA)
								};

								for (size_t i = 0; i < 6; i++)
								{
									if (faces[i].m_Width > biggestWidth)
										biggestWidth = faces[i].m_Width;
									if (faces[i].m_Height > biggestHeight)
										biggestHeight = faces[i].m_Height;
								}
							}

							std::array<Image, 6> faces
							{
							m_TexturePaths[0] == "" ?
									Image::FromColor(glm::vec4(0, 0, 0, 0), TextureFormat::RGBA, glm::vec2(biggestWidth, biggestHeight)) :
									Image::LoadFromFile(m_TexturePaths[0].u8string(), TextureFormat::RGBA),

								m_TexturePaths[1] == "" ?
								Image::FromColor(glm::vec4(0, 0, 0, 0), TextureFormat::RGBA, glm::vec2(biggestWidth, biggestHeight)) :
								Image::LoadFromFile(m_TexturePaths[1].u8string(), TextureFormat::RGBA),

								m_TexturePaths[2] == "" ?
								Image::FromColor(glm::vec4(0, 0, 0, 0), TextureFormat::RGBA, glm::vec2(biggestWidth, biggestHeight)) :
								Image::LoadFromFile(m_TexturePaths[2].u8string(), TextureFormat::RGBA),

								m_TexturePaths[3] == "" ?
								Image::FromColor(glm::vec4(0, 0, 0, 0), TextureFormat::RGBA, glm::vec2(biggestWidth, biggestHeight)) :
								Image::LoadFromFile(m_TexturePaths[3].u8string(), TextureFormat::RGBA),

								m_TexturePaths[4] == "" ?
								Image::FromColor(glm::vec4(0, 0, 0, 0), TextureFormat::RGBA, glm::vec2(biggestWidth, biggestHeight)) :
								Image::LoadFromFile(m_TexturePaths[4].u8string(), TextureFormat::RGBA),

								m_TexturePaths[5] == "" ?
								Image::FromColor(glm::vec4(0, 0, 0, 0), TextureFormat::RGBA, glm::vec2(biggestWidth, biggestHeight)) :
								Image::LoadFromFile(m_TexturePaths[5].u8string(), TextureFormat::RGBA)
							};

							m_Cubemap.UpdateData(faces);
						}
					}

					ImGui::EndCombo();
				}
			};
			displayDropDown(m_TexturePaths[0], "Right");
			displayDropDown(m_TexturePaths[1], "Left");
			displayDropDown(m_TexturePaths[2], "Botttom");
			displayDropDown(m_TexturePaths[3], "Top");
			displayDropDown(m_TexturePaths[4], "Front");
			displayDropDown(m_TexturePaths[5], "Back");

		}

		ImGui::End();
	}

	Skybox::~Skybox()
	{
		Renderer::DestroyTexture(m_Cubemap);
		Renderer::DestroyVertexBuffer(m_VertexBuffer);
		Renderer::DestroyUniformBuffer(m_UniformBuffer);
	}
}