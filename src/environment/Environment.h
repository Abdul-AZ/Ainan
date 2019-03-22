#pragma once

#include "file/stb_image_write.h"

#include "ParticleSystem.h"
#include "gui/ParticleCustomizer.h"
#include "gui/GeneralSettingsGUI.h"
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
	void DisplayObjectInspecterGUI();
	void DisplayEnvironmentStatusGUI();


private:
	std::clock_t timeStart, timeEnd;
	GeneralSettingsGUI settings;

	FrameBuffer m_FrameBuffer;
	std::vector<ParticleSystemObject> m_ParticleSystems;

	//TODO abstract this to a seperate class or something
	bool m_SaveNextFrameAsImage = false;
	float m_MinTimeBetweenFrameCapture = 2.0f; //2 seconds
	float m_CurrentTimeBetweenFrameCapture = m_MinTimeBetweenFrameCapture;

	//to keep track of when window gets resized
	//TODO make it into a callback
	glm::vec2 lastSize;
};
