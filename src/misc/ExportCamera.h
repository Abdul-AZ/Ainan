#pragma once

#include "renderer/Renderer.h"
#include "Line.h"
#include "Camera.h"
#include "file/SaveItemBrowser.h"

#include "object/ParticleSystem.h"
#include "Background.h"
#include "GaussianBlur.h"
#include "EnvironmentObjectInterface.h"

namespace Ainan {

	class ExportCamera {
	public:
		ExportCamera();
		void DrawOutline();
		void DisplayGUI();
		void Update(float deltaTime);
		void StartExporting();
		//TODO change blur radius argument to it's own struct called PosProcessingSettings or something like that
		void ExportFrame(Background& background, std::vector<pEnvironmentObject>& objects, float blurRadius);
		void BeginExportScene();

	public:
		enum ExportMode 
		{
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
		bool NeedToExport = false;

		//only used in multiple frame capture
		bool ExportedEverything = false;
		bool StartedMultiFrameExport = false;

		//only used in single frame capture
		bool AlreadyExportedFrame = false;

		int RemainingFramesToBeCaptured = 0;
	private:
		void SetSize();

	public:
		bool m_DrawExportCamera = false;
		glm::vec2 m_Edges[4];
		Line m_Outline;
		SaveItemBrowser m_ImageLocationBrowser;

		//These are only used when exporting multiple frames
		int m_CaptureFrameCount = 3;
		float m_TimeBetweenCaptures = 0.5f;
		float m_TimeSinceLastCapture = 0.0f;
	};
}