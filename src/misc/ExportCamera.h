#pragma once

#include "renderer/Renderer.h"
#include "Line.h"
#include "Camera.h"
#include "file/SaveItemBrowser.h"

#include "object/ParticleSystem.h"
#include "Background.h"
#include "EnvironmentObjectInterface.h"

namespace Ainan {

	class ExportCamera {
	public:
		ExportCamera();
		void DrawOutline();
		void DisplayGUI();
		//TODO change blur radius argument to it's own struct called PosProcessingSettings or something like that
		void ExportFrame(Background& background, std::vector<pEnvironmentObject>& objects, float blurRadius);

	public:

		bool SettingsWindowOpen = true;

		glm::vec2 m_ExportCameraPosition = { 0.0f,0.0f };
		glm::vec2 m_ExportCameraSize = { 1.0f, 1.0f * 9.0f / 16.0f };

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

	private:
		bool m_DrawExportCamera = false;
		glm::vec2 m_Edges[4];
		Line m_Outline;
		SaveItemBrowser m_ImageLocationBrowser;
		bool m_FinalizeExportWindowOpen = false;
	};
}