#pragma once

#include "Line.h"
#include "Background.h"
#include "editor/Camera.h"
#include "file/SaveItemBrowser.h"
#include "renderer/Renderer.h"
#include "environment/ParticleSystem.h"
#include "environment/EnvironmentObjectInterface.h"
#include "renderer/RenderSurface.h"
#include "renderer/Image.h"

namespace Ainan {

	class ExportCamera {
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
		Background m_Background;

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

	private:
		bool m_DrawExportCamera = false;
		glm::vec2 m_Edges[4];
		Line m_Outline;
		SaveItemBrowser m_ImageLocationBrowser;
		bool m_FinalizeExportWindowOpen = false;
	};
}