#pragma once

#include "file/FileManager.h"
#include "file/stb_image_write.h"

#include "ParticleSystem.h"
#include "Camera.h"

#include "gui/ParticleCustomizer.h"
#include "gui/GeneralSettingsGUI.h"
#include "gui/FolderBrowserGUI.h"

#include "renderer/FrameBuffer.h"
#include "renderer/GaussianBlur.h"
#include "renderer/Texture.h"

namespace ALZ {

	enum class EnvironmentStatus {
		None,
		PlayMode,
		PauseMode
	};

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

	private:
		std::clock_t timeStart, timeEnd;
		GeneralSettingsGUI settings;

		FrameBuffer m_FrameBuffer;
		Camera m_Camera;
		std::vector<ParticleSystem> m_ParticleSystems;

		bool m_ObjectInspectorWindowOpen = true;
		bool m_GeneralSettingsWindowOpen = true;
		bool m_EnvironmentStatusWindowOpen = true;
		bool m_EnvironmentControlsWindowOpen = true;
		EnvironmentStatus m_Status = EnvironmentStatus::None;

		Texture m_PlayButtonTexture;
		Texture m_PauseButtonTexture;
		Texture m_ResumeButtonTexture;
		Texture m_StopButtonTexture;

		bool m_MousePressedLastFrame = false;

		//TODO abstract this to a seperate class or something
		bool m_SaveNextFrameAsImage = false;
		float m_MinTimeBetweenFrameCapture = 2.0f; //2 seconds
		float m_CurrentTimeBetweenFrameCapture = m_MinTimeBetweenFrameCapture;
	};
}