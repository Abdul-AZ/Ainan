#pragma once

#include <iostream>
#include <chrono>
#include <ctime>
#include <random>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "ParticleSystem.h"
#include "ParticleCustomizer.h"
#include "GeneralSettingsGUI.h"

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

	std::vector<ParticleSystemObject> m_ParticleSystems;

};
