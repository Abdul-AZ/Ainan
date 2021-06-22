#pragma once

#include "renderer/Renderer.h"

namespace Ainan {

	enum class SkyMode
	{
		FlatColor,
		CubemapTexture
	};
	std::string SkyModeStr(SkyMode mode);
	SkyMode SkyModeFromStr(const std::string& str);

	class Environment;

	class Skybox
	{
		public:

		void Init(SkyMode mode = SkyMode::FlatColor, glm::vec4 color = glm::vec4(0, 0, 0, 1), std::array<std::filesystem::path, 6> paths = {});
		void Draw(const Camera& camera);
		void DisplayGUI();

		~Skybox();

	private:
		void SendDataToGPU();

	public:
		bool SkyboxWindowOpen = false;
		bool Initilized = false;

	private:
		Texture m_Cubemap;
		VertexBuffer m_VertexBuffer;
		UniformBuffer m_UniformBuffer;
		SkyMode m_Mode = SkyMode::FlatColor;
		glm::vec4 m_SkyboxColor = glm::vec4(0, 0, 0, 1);
		std::array<std::filesystem::path, 6> m_TexturePaths;

		friend bool SaveEnvironment(const Environment& env, std::string path);
	};
}