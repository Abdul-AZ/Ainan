#include "Environment.h"

Environment::Environment()
{
	//setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(&Window::GetWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 130");
	ImGuiIO& io = ImGui::GetIO(); (void)io;


	timeStart = 0;
	timeEnd = 0;

	m_ParticleSystems.push_back({ ParticleSystem(), ParticleCustomizer() });
}

Environment::~Environment()
{
	m_ParticleSystems.clear();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Environment::Update()
{
	timeEnd = clock();
	float deltaTime = (timeEnd - timeStart) / 1000.0f;
	timeStart = timeEnd;

	//Update
	Window::Update();

	for(ParticleSystemObject& obj : m_ParticleSystems)
		obj.m_PS.Update(deltaTime);
}

void Environment::Render()
{
	for (ParticleSystemObject& obj : m_ParticleSystems)
		obj.m_PS.Draw();
}

void Environment::RenderGUI()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	settings.DisplayGUI();

	for (ParticleSystemObject& obj : m_ParticleSystems)
		obj.m_PC.DisplayGUI();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Environment::HandleInput()
{
	if (glfwGetMouseButton(&Window::GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !ImGui::GetIO().WantCaptureMouse)
	{
		for (ParticleSystemObject& obj : m_ParticleSystems)
			obj.m_PS.SpawnParticle(obj.m_PC.GetParticle());
	}

	if (glfwGetKey(&Window::GetWindow(), GLFW_KEY_SPACE) == GLFW_PRESS)
		for (ParticleSystemObject& obj : m_ParticleSystems)
			obj.m_PS.ClearParticles();
}
