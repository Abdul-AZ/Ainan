#pragma once

#include "editor/Camera.h"
#include "file/SaveItemBrowser.h"
#include "renderer/Renderer.h"
#include "environment/ParticleSystem.h"
#include "environment/LitSprite.h"
#include "environment/RadialLight.h"
#include "environment/SpotLight.h"
#include "environment/Environment.h"
#include "environment/EnvironmentObjectInterface.h"
#include "renderer/RenderSurface.h"
#include "renderer/Image.h"

namespace Ainan {

	const glm::vec4 c_OutlineColor = { 0.8f, 0.0, 0.0f, 0.8f };

	class ExportCamera 
	{
		enum ExportMode
		{
			Picture,
			Video
		};
	public:
		ExportCamera();
		void DrawOutline();
		void DisplayGUI();
		void ExportFrame(Environment& env);

	public:
		bool SettingsWindowOpen = true;

		glm::vec2 m_ExportCameraPosition = { 0.0f,0.0f };
		float m_AspectRatio = 16.0f / 9.0f;

		Camera RealCamera;
		RenderSurface m_RenderSurface;

		std::string   ImageSavePath;
		ImageFormat   SaveImageFormat = ImageFormat::png;
		Image* m_ExportTargetImage = nullptr;
		std::shared_ptr<Texture> m_ExportTargetTexture = nullptr;

		//this means after x seconds we will capture the frame using this export camera
		//timing is handled in the environment class not here
		float ImageCaptureTime = 5.0f;

		int RemainingFramesToBeCaptured = 0;
	private:
		void SetSize();
		void ExportVideoLoop(Environment& env);
		void RenderNextFrame(Environment& env);

	private:
		bool m_DrawExportCamera = false;
		std::array<glm::vec2, 4> m_OutlineVertices;
		std::shared_ptr<VertexBuffer> m_OutlineVertexBuffer = nullptr;
		std::shared_ptr<UniformBuffer> m_OutlineUniformBuffer = nullptr;
		SaveItemBrowser m_ImageLocationBrowser;
		bool m_FinalizeExportWindowOpen = false;
		ExportMode m_Mode = ExportMode::Picture;

		std::string GetModeString(ExportMode mode)
		{
			switch (mode)
			{
			case Picture:
				return "Picture";

			case Video:
				return "Video";

			default:
				return "";
			}
		}
	};
}