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

	class ExportCamera {
	public:
		ExportCamera();
		void DrawOutline();
		void DisplayGUI();
		void ExportFrame(Environment& env);

	public:
		bool SettingsWindowOpen = true;

		glm::vec2 m_ExportCameraPosition = { 0.0f,0.0f };
		int32_t m_WidthRatio = 16;
		int32_t m_HeightRatio = 9;

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
		std::array<glm::vec2, 4> m_OutlineVertices;
		std::shared_ptr<VertexBuffer> m_OutlineVertexBuffer = nullptr;
		std::shared_ptr<UniformBuffer> m_OutlineUniformBuffer = nullptr;
		SaveItemBrowser m_ImageLocationBrowser;
		bool m_FinalizeExportWindowOpen = false;
	};
}