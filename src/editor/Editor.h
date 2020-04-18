#pragma once

#include "environment/Environment.h"
#include "misc/Background.h"
#include "misc/ViewportWindow.h"
#include "misc/AppStatusWindow.h"
#include "file/FolderBrowser.h"
#include "misc/Grid.h"
#include "misc/InputManager.h"
#include "misc/EditorStyles.h"
#include "object/ParticleSystem.h"
#include "object/Sprite.h"
#include "misc/ExportCamera.h"

#define WINDOW_SIZE_FACTOR_ON_LAUNCH 500
#define START_MENU_BUTTON_WIDTH 200
#define START_MENU_BUTTON_HEIGHT 75

namespace Ainan {
	bool SaveEnvironment(const Environment& env, std::string path);
	std::string CheckEnvironmentFile(const std::string& path);
	Environment* LoadEnvironment(const std::string& path);

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
		bool m_ShowGrid = true;

		EditorState m_State = State_NoEnvLoaded;
		std::filesystem::path m_EnvironmentFolderPath;
		std::shared_ptr<Texture> m_PlayButtonTexture;
		std::shared_ptr<Texture> m_PauseButtonTexture;
		std::shared_ptr<Texture> m_StopButtonTexture;
		float m_TimeSincePlayModeStarted = 0.0f;
		bool m_ExportedFrame = false;
		bool m_AddObjectWindowOpen = false;
		std::string m_AddObjectWindowObjectName = "Example Name";
		EnvironmentObjectType m_AddObjectWindowObjectType = SpriteType;
		std::clock_t m_TimeStart = 0, m_TimeEnd = 0;
		float m_DeltaTime = 0.0f;
		std::array<float, 120> m_DeltaTimeHistory;
		Profiler m_ActiveProfiler = Profiler::RenderingProfiler;

		std::string m_EnvironmentCreateFolderPath;
		std::string m_EnvironmentCreateName;
		bool m_CreateEvironmentDirectory = false;
		FolderBrowser m_FolderBrowser;
		FileBrowser m_LoadEnvironmentBrowser;
		bool m_IncludeStarterAssets = false;
		bool m_ShouldDeleteEnv = false;

	private:
		//methods based on editor state
		void Update_EditorMode();
		void Update_PlayMode();
		void Update_PauseMode();
		void Update_ExportMode();

		void Draw_NoEnvLoaded();
		void Draw_CreateEnv();
		void Draw_EditorMode();
		void Draw_PlayMode();
		void Draw_PauseMode();
		void Draw_ExportMode();

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
		void UpdateTitle();
	};
}

