#pragma once

#include "Background.h"
#include "ViewportWindow.h"
#include "AppStatusWindow.h"
#include "Grid.h"
#include "InputManager.h"
#include "EditorStyles.h"
#include "environment/Environment.h"
#include "environment/ParticleSystem.h"
#include "environment/Sprite.h"
#include "environment/LitSprite.h"
#include "ExportCamera.h"
#include "file/FolderBrowser.h"
#include "EditorPreferences.h"

namespace Ainan {
	bool SaveEnvironment(const Environment& env, std::string path);
	std::string CheckEnvironmentFile(const std::string& path);
	Environment* LoadEnvironment(const std::string& path);

	const float c_StartMenuBtnWidth     = 300.0f;
	const float c_StartMenuBtnHeight    = 75.0f;
	const float c_StartMenuBtnStartLeft = 60.0f;
	const float c_StartMenuBtnStartTop  = 50.0f;
	const float c_StartMenuBtnMarginY   = 10.0f;
	const float c_CreateEnvBtnWidth     = 250.0f;
	const float c_CreateEnvBtnHeight    = 75.0f;

	class Editor
	{
		enum EditorState
		{
			State_NoEnvLoaded, //this usuallly means we just started the app or quit from an env
			State_CreateEnv,   //this means we are on Create Environment menu
			State_EditorMode,  //this means we are in the normal enviornment editor
			State_PlayMode,    //this means that the environment is being simulated/run
			State_PauseMode,   //this means that there is a pause in simulation and can be resumed to where it was stopped
			State_ExportMode   //this means that the environment is being simulated/run and there is also recording/exporting
		};

		enum class Profiler
		{
			ParticleProfiler,
			PlaymodeProfiler,
			RenderingProfiler
		};

	public:
		Editor();
		~Editor();

		void Update();
		void Draw();
		void StartFrame();
		void EndFrame();

	private:
		Environment* m_Env = nullptr;
		EditorPreferences m_Preferences;
		Camera m_Camera;
		ViewportWindow m_ViewportWindow;
		AppStatusWindow m_AppStatusWindow;
		ExportCamera m_ExportCamera;
		RenderSurface m_RenderSurface;
		Gizmo m_Gizmo;
		Grid m_Grid;
		Background m_Background;

		bool m_EnvironmentControlsWindowOpen = true;
		bool m_ObjectInspectorWindowOpen = true;
		bool m_ProfilerWindowOpen = true;
		bool m_EnvironmentSettingsWindowOpen = true;
		bool m_PreferencesWindowOpen = false;
		bool m_ShowGrid = true;

		EditorState m_State = State_NoEnvLoaded;
		std::filesystem::path m_EnvironmentFolderPath;
		std::shared_ptr<Texture> m_PlayButtonTexture;
		std::shared_ptr<Texture> m_PauseButtonTexture;
		std::shared_ptr<Texture> m_StopButtonTexture;
		std::shared_ptr<Texture> m_SpriteIconTexture;
		std::shared_ptr<Texture> m_LitSpriteIconTexture;
		std::shared_ptr<Texture> m_ParticleSystemIconTexture;
		std::shared_ptr<Texture> m_SpotLightIconTexture;
		std::shared_ptr<Texture> m_RadialLightIconTexture;
		float m_TimeSincePlayModeStarted = 0.0f;
		bool m_ExportedFrame = false;
		bool m_AddObjectWindowOpen = false;
		std::string m_AddObjectWindowObjectName = "Example Name";
		EnvironmentObjectType m_AddObjectWindowObjectType = SpriteType;
		std::clock_t m_TimeStart = 0, m_TimeEnd = 0;
		std::array<float, 120> m_DeltaTimeHistory;
		Profiler m_ActiveProfiler = Profiler::RenderingProfiler;

		float m_SimulationSpeedFactor = 1.0f;
		std::string m_EnvironmentCreateFolderPath;
		std::string m_EnvironmentCreateName;
		bool m_CreateEvironmentDirectory = false;
		FolderBrowser m_FolderBrowser;
		FileBrowser m_LoadEnvironmentBrowser;
		bool m_IncludeStarterAssets = false;
		bool m_ShouldDeleteEnv = false;

		std::array<std::thread, 4> WorkerThreads;
		std::queue<EnvironmentObjectInterface*> UpdateQueue;
		std::mutex UpdateMutex;
		std::condition_variable StartUpdating;
		std::condition_variable FinishedUpdating;
		std::atomic_bool DestroyThreads = false;
		float m_DeltaTime = 0.0f;			//change in realtime
		float m_SimulationDeltaTime = 0.0f; //change in simulation time
		int32_t m_AverageFPS = 0;
		uint32_t m_GPUMemAllocated = 0;
		int32_t m_DrawCalls = 0;

	private:
		void WorkerThreadLoop();

		//methods based on editor state
		void Update_EditorMode(float deltaTime);
		void Update_PlayMode(float deltaTime);
		void Update_PauseMode(float deltaTime);
		void Update_ExportMode(float deltaTime);

		void DrawHomeWindow();
		void DrawEnvironmentCreationWindow();

		void DrawEnvironment(bool drawWorldSpaceUI);
		void DrawUI();

		//general functions for reusing code
		void OnEnvironmentLoad();
		void OnEnvironmentDestroy();
		void DisplayMainMenuBarGUI();
		void DisplayEnvironmentControlsGUI();
		void Stop();
		void Pause();
		void Resume();
		void ExportMode();
		void PlayMode();
		void DisplayObjectInspecterGUI();
		void RefreshObjectOrdering();
		void Duplicate(EnvironmentObjectInterface& obj);
		void FocusCameraOnObject(EnvironmentObjectInterface& object);
		void AddEnvironmentObject(EnvironmentObjectType type, const std::string& name);
		void RegisterEnvironmentInputKeys();
		void DisplayProfilerGUI();
		void DisplayPreferencesGUI();
		void UpdateTitle();
	};
}

