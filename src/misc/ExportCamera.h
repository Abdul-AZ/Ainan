#pragma once

#include "renderer/Renderer.h"
#include "Line.h"
#include "Camera.h"
#include "file/SaveItemBrowser.h"

#include "object/ParticleSystem.h"
#include "Background.h"
#include "GaussianBlur.h"

namespace ALZ {

	class ExportCamera {
	public:
		ExportCamera();
		void DrawOutline();
		void DisplayGUI();
		//TODO change blur radius argument to it's own struct called PosProcessingSettings or something like that
		void ExportFrame(Background& background, std::vector<Inspector_obj_ptr>& objects, float blurRadius);

	public:
		enum ExportMode {
			SingleFrame,
			MultipleFramesAsSeperateImages
		};

		bool SettingsWindowOpen = true;

		glm::vec2 m_ExportCameraPosition = { 0.0f,0.0f };
		glm::vec2 m_ExportCameraSize = { 1.0f, 1.0f * 9.0f / 16.0f };

		Camera RealCamera;
		RenderSurface m_RenderSurface;

		std::string   ImageSavePath;
		ImageFormat   SaveImageFormat = ImageFormat::png;

		ExportMode m_ExportMode = SingleFrame;

		//this means after x seconds we will capture the frame using this export camera
		//timing is handled in the environment class not here
		bool EnableCaptureImageAfterXSeconds = false;
		float ImageCaptureTime = 5.0f;

	private:
		void SetSize();

	private:
		bool m_DrawExportCamera = false;
		glm::vec2 m_Edges[4];
		Line m_OutlineLines[4];
		SaveItemBrowser m_ImageLocationBrowser;

		//These are only used when exporting multiple frames
		int m_CaptureFrameCount = 3;
		float m_TimeBetweenCaptures = 0.5f;
		int m_CaptureIndex = 0;
	};
}