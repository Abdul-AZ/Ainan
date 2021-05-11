#pragma once

#include "environment/UUID.h"
#include "ViewportWindow.h"
#include "AppStatusWindow.h"
#include "Grid.h"
#include "InputManager.h"
#include "EditorStyles.h"
#include "environment/Environment.h"
#include "environment/ParticleSystem.h"
#include "environment/Sprite.h"
#include "environment/LitSprite.h"
#include "environment/Model.h"
#include "environment/RadialLight.h"
#include "environment/SpotLight.h"
#include "environment/CameraObject.h"
#include "Exporter.h"
#include "file/FolderBrowser.h"
#include "EditorPreferences.h"
#include "ImGuizmo.h"

namespace Ainan {
	bool SaveEnvironment(const Environment& env, std::string path);
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
			State_PauseMode    //this means that there is a pause in simulation and can be resumed to where it was stopped
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
		bool NeedToPresent();

	private:
		int32_t m_RedrawUI = 1;

		Environment* m_Env = nullptr;
		EditorPreferences m_Preferences;
		EditorCamera m_Camera;
		bool SnappingEnabled = false;
		float Snap = 1.0f;
		ViewportWindow m_ViewportWindow;
		AppStatusWindow m_AppStatusWindow;
		Exporter m_Exporter;
		RenderSurface m_RenderSurface;
		Grid m_Grid;

		bool m_PropertiesWindowOpen = true;
		bool m_EnvironmentControlsWindowOpen = true;
		bool m_ObjectInspectorWindowOpen = true;
		bool m_ProfilerWindowOpen = true;
		bool m_EnvironmentSettingsWindowOpen = true;
		bool m_PreferencesWindowOpen = false;
		bool m_ShowGrid = true;
		bool m_ShowObjectIcons = true;

		EditorState m_State = State_NoEnvLoaded;
		std::filesystem::path m_EnvironmentFolderPath;
		Texture m_PlayButtonTexture;
		Texture m_PauseButtonTexture;
		Texture m_StopButtonTexture;
		Texture m_SpriteIconTexture;
		Texture m_LitSpriteIconTexture;
		Texture m_MeshIconTexture;
		Texture m_ParticleSystemIconTexture;
		Texture m_SpotLightIconTexture;
		Texture m_RadialLightIconTexture;
		Texture m_CameraIconTexture;
		float m_TimeSincePlayModeStarted = 0.0f;
		bool m_AddObjectWindowOpen = false;
		std::string m_AddObjectWindowObjectName = "Example Name";
		EnvironmentObjectType m_AddObjectWindowObjectType = SpriteType;
		std::clock_t m_TimeStart = 0, m_TimeEnd = 0;
		std::array<float, 120> m_DeltaTimeHistory;
		Profiler m_ActiveProfiler = Profiler::RenderingProfiler;
		ImGuizmo::OPERATION m_GizmoOperation = ImGuizmo::OPERATION::TRANSLATE;

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
		float m_SimulationDeltaTime = 0.0f; //change in simulation time
		int32_t m_AverageFPS = 0;
		uint32_t m_GPUMemAllocated = 0;
		int32_t m_DrawCalls = 0;
		uint32_t FrameCounter = 0; //advances by 1 on every update iteration. when it reaches c_ApplicationFramerate, it goes back to 0.
		std::mt19937 m_RandomNumberGenerator;
		
		Texture Skybox;
		VertexBuffer SkyboxVertexBuffer;
		UniformBuffer SkyboxUniformBuffer;

	private:
		void WorkerThreadLoop();

		//methods based on editor state
		void Update_EditorMode(float deltaTime);
		void Update_PlayMode(float deltaTime);
		void Update_PauseMode(float deltaTime);

		void DrawHomeWindow();
		void DrawEnvironmentCreationWindow();

		void DrawEnvironment(bool drawWorldSpaceUI);
		void DrawUI();

		//general functions for reusing code
		void OnEnvironmentLoad();
		void OnEnvironmentDestroy();
		void DisplayMainMenuBarGUI();
		void DisplayEnvironmentControlsGUI();
		void DisplayPropertiesGUI();
		void Stop();
		void Pause();
		void Resume();
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

		friend class Exporter;
	};
}

