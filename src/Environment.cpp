#include "Environment.h"

Environment::Environment()
{
	//setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(&Window::GetWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 400");
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking

	timeStart = 0;
	timeEnd = 0;

	ParticleSystemObject startingPS;

	m_ParticleSystems.push_back(startingPS);
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

	auto flags = ImGuiDockNodeFlags_PassthruDockspace;
	ImGui::DockSpaceOverViewport(0, flags, 0);

	settings.DisplayGUI();

	DisplayObjectInspecterGUI();

	for (ParticleSystemObject& obj : m_ParticleSystems)
		obj.DisplayGUI();


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

void Environment::DisplayObjectInspecterGUI()
{
	ImGuiWindowFlags flags = ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar;
	ImGui::Begin("Particle Systems", nullptr, flags);

	if(ImGui::BeginMenuBar()) {

		if(ImGui::BeginMenu("Edit")) {

			if (ImGui::MenuItem("Clear All")) {
				m_ParticleSystems.clear();
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	for (int i = 0; i < m_ParticleSystems.size(); i++)
	{
		ImGui::PushID(i);
		ImGui::Text(m_ParticleSystems[i].m_Name.c_str());
		if (ImGui::Button("Edit")) {
			m_ParticleSystems[i].m_EditorOpen = !m_ParticleSystems[i].m_EditorOpen;
		}

		if (ImGui::Button("Delete")) {
			m_ParticleSystems.erase(m_ParticleSystems.begin() + i);
		}

		ImGui::Spacing();

		ImGui::PopID();
	}

	if (ImGui::Button("Add Particle System"))
	{
		ParticleSystemObject pso;
		m_ParticleSystems.push_back(pso);
	}

	ImGui::End();
}

void ParticleSystemObject::DisplayGUI()
{
	if(m_EditorOpen)
		m_PC.DisplayGUI(m_Name);
}
