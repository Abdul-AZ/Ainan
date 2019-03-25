#pragma once

#include "file/FileManager.h"
#include "file/stb_image_write.h"

#include "ParticleSystem.h"

#include "gui/ParticleCustomizer.h"
#include "gui/GeneralSettingsGUI.h"
#include "gui/FolderBrowserGUI.h"

#include "renderer/FrameBuffer.h"
#include "renderer/GaussianBlur.h"

struct ParticleSystemObject 
{
	ParticleSystemObject();

	ParticleSystem m_PS;
	ParticleCustomizer m_PC;
	std::string m_Name;
	bool m_EditorOpen;
	bool m_RenameTextOpen;
	int m_ID;

	void DisplayGUI();
};

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
	std::vector<ParticleSystemObject> m_ParticleSystems;

	bool m_ObjectInspectorWindowOpen = true;
	bool m_GeneralSettingsWindowOpen = true;
	bool m_EnvironmentStatusWindowOpen = true;
	bool m_EnvironmentControlsWindowOpen = true;
	EnvironmentStatus m_Status = EnvironmentStatus::None;

	//TODO abstract this to a seperate class or something
	bool m_SaveNextFrameAsImage = false;
	float m_MinTimeBetweenFrameCapture = 2.0f; //2 seconds
	float m_CurrentTimeBetweenFrameCapture = m_MinTimeBetweenFrameCapture;

	//to keep track of when window gets resized
	//TODO make it into a callback
	glm::vec2 lastSize;
};
