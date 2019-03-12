#pragma once

#include <iostream>
#include <chrono>
#include <ctime>
#include <random>

#define IMGUI_IMPL_OPENGL_LOADER_GLAD

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "ParticleSystem.h"
#include "ParticleCustomizer.h"
#include "GeneralSettingsGUI.h"

struct ParticleSystemObject 
{
	ParticleSystem m_PS;
	ParticleCustomizer m_PC;
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
	std::clock_t timeStart, timeEnd;
	GeneralSettingsGUI settings;

	std::vector<ParticleSystemObject> m_ParticleSystems;

};
