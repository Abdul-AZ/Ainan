#pragma once

#include "file/FileManager.h"
#include "file/stb_image_write.h"

#include "ParticleSystem.h"
#include "Camera.h"
#include "EditorStyles.h"

#include "gui/ParticleCustomizer.h"
#include "gui/GeneralSettingsGUI.h"
#include "gui/FolderBrowserGUI.h"

#include "renderer/FrameBuffer.h"
#include "renderer/GaussianBlur.h"
#include "renderer/Texture.h"
#include "renderer/CircleOutline.h"
#include "renderer/Background.h"

#include "input/InputManager.h"

namespace ALZ {

	enum class EnvironmentStatus {
		None,
		PlayMode,
		PauseMode
	};

	using Inspector_obj_ptr = std::unique_ptr<InspectorInterface>;

	class Environment
	{
	public:
		Environment();
		~Environment();

		void Update();
		void Render();
		void RenderGUI();
		void HandleInput();

	private:
		//helper functions to spread code around
		void DisplayObjectInspecterGUI();
		void DisplayEnvironmentStatusGUI();
		void DisplayEnvironmentControlsGUI();
		void DisplayMainMenuBarGUI();

		void Play();
		void Stop();
		void Pause();
		void Resume();
		void RegisterEnvironmentInputKeys();

		void AddPS();
		void AddRadialLight();
		void FocusCameraOnObject(InspectorInterface& object);

	private:
		std::clock_t timeStart, timeEnd;
		GeneralSettingsGUI settings;

		FrameBuffer m_FrameBuffer;
		Camera m_Camera;
		InputManager m_InputManager;
		std::vector<std::unique_ptr<InspectorInterface>> InspectorObjects;

		bool m_HideGUI = false;
		bool m_ObjectInspectorWindowOpen = true;
		bool m_EnvironmentStatusWindowOpen = true;
		bool m_EnvironmentControlsWindowOpen = true;
		EnvironmentStatus m_Status = EnvironmentStatus::None;

		Texture m_PlayButtonTexture;
		Texture m_PauseButtonTexture;
		Texture m_ResumeButtonTexture;
		Texture m_StopButtonTexture;

		bool m_MousePressedLastFrame = false;

		bool m_SaveNextFrameAsImage = false;

		Background m_Background;
	};
}