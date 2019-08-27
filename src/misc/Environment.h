#pragma once

#include "file/FileManager.h"
#include "stb/stb_image_write.h"

#include "object/ParticleSystem.h"
#include "Camera.h"
#include "EditorStyles.h"
#include "ExportCamera.h"

#include "ParticleCustomizer.h"
#include "GeneralSettingsGUI.h"
#include "AppStatusWindow.h"
#include "file/FolderBrowser.h"
#include "file/SaveItemBrowser.h"

#include "ImGuiWrapper.h"

#include "misc/RenderSurface.h"
#include "misc/GaussianBlur.h"
#include "misc/CircleOutline.h"
#include "misc/Background.h"
#include "misc/Grid.h"
#include "misc/Gizmo.h"
#include "renderer/Renderer.h"
#include "ViewportWindow.h"

#include "InputManager.h"

#include "vendor/json/json_fwd.hpp"

namespace ALZ {

	class Environment
	{
		enum EnvironmentStatus {
			Status_EditorMode,
			Status_PlayMode,
			Status_PauseMode,
			Status_ExportMode
		};

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

		void PlayMode();
		void ExportMode();
		void Stop();
		void Pause();
		void Resume();
		void RegisterEnvironmentInputKeys();

		void AddInspectorObject(EnvironmentObjectType type);
		void Duplicate(EnvironmentObjectInterface& obj);
		void FocusCameraOnObject(EnvironmentObjectInterface& object);

	public:
		bool ShouldDelete = false;

	private:
		std::clock_t timeStart = 0, timeEnd = 0;
		GeneralSettingsGUI m_Settings;

		RenderSurface m_RenderSurface;
		Camera m_Camera;
		ExportCamera m_ExportCamera;
		InputManager m_InputManager;
		Grid m_Grid;
		Gizmo m_Gizmo;
		std::vector<std::unique_ptr<EnvironmentObjectInterface>> InspectorObjects;

		bool m_HideGUI = false;
		bool m_ObjectInspectorWindowOpen = true;
		bool m_EnvironmentStatusWindowOpen = true;
		bool m_EnvironmentControlsWindowOpen = true;
		EnvironmentStatus m_Status = Status_EditorMode;

		std::unique_ptr<Texture> m_PlayButtonTexture;
		std::unique_ptr<Texture> m_PauseButtonTexture;
		std::unique_ptr<Texture> m_ResumeButtonTexture;
		std::unique_ptr<Texture> m_StopButtonTexture;

		bool m_MousePressedLastFrame = false;

		float m_TimeSincePlayModeStarted = 0.0f;

		Background m_Background;
		ViewportWindow m_ViewportWindow;

		AppStatusWindow m_AppStatusWindow;

		SaveItemBrowser m_EnvironmentSaveBrowser;
		bool m_SaveLocationSelected = false;

		//expose private parameters for environment serilization (saving and loading environments)
		friend bool SaveEnvironment(const Environment& env, std::string path);
		friend Environment* LoadEnvironment(const std::string& path);
		friend void SettingsFromJson(Environment* env, nlohmann::json& data);
		friend void ParticleSystemFromJson(Environment* env, nlohmann::json& data, std::string id);
		friend void RadialLightFromJson(Environment* env, nlohmann::json& data, std::string id);
		friend void SpotLightFromJson(Environment* env, nlohmann::json& data, std::string id);
	};


	//defined in EnvSave.cpp
	bool SaveEnvironment(const Environment& env, std::string path);

	//defined in EnvLoad.cpp
	Environment* LoadEnvironment(const std::string& path);
	//returns error string or "" if successful
	std::string CheckEnvironmentFile(const std::string& path);
}