#pragma once

#include "FileManager.h"
#include "stb/stb_image_write.h"

#include "ParticleSystem.h"
#include "Camera.h"
#include "EditorStyles.h"

#include "ParticleCustomizer.h"
#include "GeneralSettingsGUI.h"
#include "FolderBrowserGUI.h"
#include "SaveItemGUI.h"

#include "ImGuiWrapper.h"

#include "graphics/RenderSurface.h"
#include "graphics/GaussianBlur.h"
#include "graphics/Texture.h"
#include "graphics/CircleOutline.h"
#include "graphics/Background.h"
#include "graphics/Grid.h"
#include "Renderer.h"

#include "InputManager.h"

#include "vendor/json/json_fwd.hpp"

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
		void Duplicate(InspectorInterface& obj);
		void FocusCameraOnObject(InspectorInterface& object);

	public:
		bool ShouldDelete = false;

	private:
		std::clock_t timeStart = 0, timeEnd = 0;
		GeneralSettingsGUI m_Settings;

		RenderSurface m_FrameBuffer;
		Camera m_Camera;
		InputManager m_InputManager;
		Grid m_Grid;
		std::vector<std::unique_ptr<InspectorInterface>> InspectorObjects;

		bool m_HideGUI = false;
		bool m_ObjectInspectorWindowOpen = true;
		bool m_EnvironmentStatusWindowOpen = true;
		bool m_EnvironmentControlsWindowOpen = true;
		EnvironmentStatus m_Status = EnvironmentStatus::None;

		std::unique_ptr<Texture> m_PlayButtonTexture;
		std::unique_ptr<Texture> m_PauseButtonTexture;
		std::unique_ptr<Texture> m_ResumeButtonTexture;
		std::unique_ptr<Texture> m_StopButtonTexture;

		bool m_MousePressedLastFrame = false;
		bool m_SaveNextFrameAsImage = false;

		Background m_Background;

		SaveItemGUI m_EnvironmentSaveBrowser;
		bool m_SaveLocationSelected = false;


		//expose private parameters for environment serilization (saving and loading environments)
		friend bool SaveEnvironment(const Environment& env, std::string path);
		friend Environment* LoadEnvironment(const std::string& path);
		friend void SettingsFromJson(Environment* env, nlohmann::json& data);
		friend void ParticleSystemFromJson(Environment* env, nlohmann::json& data, std::string id);
		friend void RadialLightFromJson(Environment* env, nlohmann::json& data, std::string id);
	};


	//defined in EnvSave.cpp
	bool SaveEnvironment(const Environment& env, std::string path);

	//defined in EnvLoad.cpp
	Environment* LoadEnvironment(const std::string& path);
	//returns error string or "" if successful
	std::string CheckEnvironmentFile(const std::string& path);
}