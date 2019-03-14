#pragma once

#include <iostream>
#include <chrono>
#include <ctime>
#include <random>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "ParticleSystem.h"
#include "ParticleCustomizer.h"
#include "GeneralSettingsGUI.h"

struct ParticleSystemObject 
{
	ParticleSystemObject():
		m_EditorOpen(true) 
	{
		static int nameIndextemp = 0;
		m_Name = "Particle System (" + std::to_string(nameIndextemp) + ")";
		nameIndextemp++;
	}

	ParticleSystem m_PS;
	ParticleCustomizer m_PC;
	bool m_EditorOpen;
	std::string m_Name;

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


private:
	std::clock_t timeStart, timeEnd;
	GeneralSettingsGUI settings;

	std::vector<ParticleSystemObject> m_ParticleSystems;

};
