#pragma once

#include "editor/EditorCamera.h"
#include "file/SaveItemBrowser.h"
#include "renderer/Renderer.h"
#include "environment/ParticleSystem.h"
#include "environment/LitSprite.h"
#include "environment/RadialLight.h"
#include "environment/SpotLight.h"
#include "environment/CameraObject.h"
#include "environment/Environment.h"
#include "environment/EnvironmentObjectInterface.h"
#include "renderer/RenderSurface.h"
#include "renderer/Image.h"

namespace Ainan {

	class Exporter 
	{
		enum ExportMode
		{
			Picture,
			Video
		};
	public:
		Exporter();
		~Exporter();
		void DisplayGUI(Environment& env);
		void OpenExporterWindow();

		void ExportIfScheduled(Editor& editor);
		void ExportImage(Editor& editor);
		void ExportVideo(Editor& editor);

	public:
		bool m_ExporterWindowOpen = false;
		bool CurrentlyExporting = false;

		Image* m_ExportTargetImage = nullptr;
		Texture m_ExportTargetTexture;

		Camera m_Camera;
		UUID ExportCameraID;
		RenderSurface m_RenderSurface;

		struct ExportVideoSettings
		{
			SaveItemBrowser ExportTargetLocation;
			std::filesystem::path ExportTargetPath;
			int32_t Framerate = 60;
			int32_t LengthMinutes = 0;
			int32_t LengthSeconds = 5;
		} VideoSettings;

		struct ExportPictureSettings
		{
			SaveItemBrowser ExportTargetLocation;
			std::filesystem::path ExportTargetPath;
			ImageFormat Format = ImageFormat::png;
		} PictureSettings;

		//this means after x seconds we will capture the frame using this exporter
		float ExportStartTime = 5.0f;
	private:
		void DrawEnvToExportSurface(Environment& env);
		void GetImageFromExportSurfaceToRAM();
		void DisplayVideoExportSettingsControls();
		void DisplayFinalizePictureExportSettingsWindow();
		void DisplayProgressBarWindow(int32_t operationNum, int32_t operationCount, float fraction);

	private:
		bool m_ExporterScheduled = false;
		bool m_FinalizePictureExportWindowOpen = false;

		bool m_DrawExportCamera = false;
		std::array<glm::vec2, 4> m_OutlineVertices;
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