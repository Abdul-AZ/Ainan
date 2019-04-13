#include <pch.h>
#include "Environment.h"

namespace ALZ {

	Environment::Environment() :
		m_PlayButtonTexture("res/PlayButton.png"),
		m_PauseButtonTexture("res/PauseButton.png"),
		m_ResumeButtonTexture("res/ResumeButton.png"),
		m_StopButtonTexture("res/StopButton.png")
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

		ParticleSystem startingPS;

		m_ParticleSystems.push_back(startingPS);

		GaussianBlur::Init();
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
		m_CurrentTimeBetweenFrameCapture -= deltaTime;

		Window::Update();
		m_Camera.Update(deltaTime);

		if (m_Status == EnvironmentStatus::PlayMode) {
			for (ParticleSystem& obj : m_ParticleSystems)
				obj.Update(deltaTime, m_Camera);
		}

		if (Window::WindowSizeChangedSinceLastFrame())
			m_FrameBuffer.SetSize(Window::GetSize());
	}

	void Environment::Render()
	{
		if (m_Status == EnvironmentStatus::None)
			return;

		//stuff to only render in play mode
		m_FrameBuffer.Bind();
		glClear(GL_COLOR_BUFFER_BIT);

		for (ParticleSystem& obj : m_ParticleSystems)
			obj.Draw();

		m_FrameBuffer.Unbind();

		if (settings.GetBlurEnabled())
			GaussianBlur::Blur(m_FrameBuffer, settings.GetBlurScale(), settings.GetBlurStrength(), settings.GetBlurGaussianSigma());

		m_FrameBuffer.Bind();

		if (m_SaveNextFrameAsImage)
		{
			Image image = Image::FromFrameBuffer(m_FrameBuffer, settings.GetImageResolution().x, settings.GetImageResolution().y);
			image.SaveToFile(settings.GetImageSaveLocation() + '/' + settings.GetImageName(), settings.GetImageFormat());
			m_SaveNextFrameAsImage = false;
		}
		m_FrameBuffer.RenderToScreen();
	}

	void Environment::RenderGUI()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		DisplayMainMenuBarGUI();
		DisplayEnvironmentControlsGUI();

		if (m_ObjectInspectorWindowOpen)
			DisplayObjectInspecterGUI();

		if (m_GeneralSettingsWindowOpen)
			settings.DisplayGUI(m_GeneralSettingsWindowOpen);

		if (m_EnvironmentStatusWindowOpen)
			DisplayEnvironmentStatusGUI();

		for (ParticleSystem& obj : m_ParticleSystems)
			obj.DisplayGUI(m_Camera);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void Environment::HandleInput()
	{
		//Playmode specific input :
		if (m_Status == EnvironmentStatus::PlayMode) {
			if (glfwGetMouseButton(&Window::GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !ImGui::GetIO().WantCaptureMouse)
			{
				for (ParticleSystem& obj : m_ParticleSystems) {
					if (obj.m_Customizer.m_Mode == SpawnMode::SpawnOnMousePosition) {
						if (!m_MousePressedLastFrame)
							obj.m_TimeTillNextParticleSpawn = 0.0f;
						obj.m_ShouldSpawnParticles = true;
					}
				}
				m_MousePressedLastFrame = true;
			}
			else {

				for (ParticleSystem& obj : m_ParticleSystems) {
					if (obj.m_Customizer.m_Mode == SpawnMode::SpawnOnMousePosition) {
						obj.m_ShouldSpawnParticles = false;
					}
				}
				m_MousePressedLastFrame = false;
			}
		}
		//TEMPORARY camera input
		{
			float speed = 10.0f;
			if (glfwGetKey(&Window::GetWindow(), GLFW_KEY_W) == GLFW_PRESS) {
				m_Camera.SetPosition(m_Camera.Position + glm::vec3(0, speed, 0));
			}
			if (glfwGetKey(&Window::GetWindow(), GLFW_KEY_S) == GLFW_PRESS) {
				m_Camera.SetPosition(m_Camera.Position + glm::vec3(0, -speed, 0));
			}
			if (glfwGetKey(&Window::GetWindow(), GLFW_KEY_D) == GLFW_PRESS) {
				m_Camera.SetPosition(m_Camera.Position + glm::vec3(-speed, 0, 0));
			}
			if (glfwGetKey(&Window::GetWindow(), GLFW_KEY_A) == GLFW_PRESS) {
				m_Camera.SetPosition(m_Camera.Position + glm::vec3(speed, 0, 0));
			}
		}

		if (glfwGetKey(&Window::GetWindow(), GLFW_KEY_SPACE) == GLFW_PRESS)
			for (ParticleSystem& obj : m_ParticleSystems)
				obj.ClearParticles();

		if (glfwGetKey(&Window::GetWindow(), GLFW_KEY_F11) == GLFW_PRESS && m_CurrentTimeBetweenFrameCapture < 0.0f) {
			m_SaveNextFrameAsImage = true;
			m_CurrentTimeBetweenFrameCapture = m_MinTimeBetweenFrameCapture;
		}

		if (glfwGetKey(&Window::GetWindow(), GLFW_KEY_F5) == GLFW_PRESS) {
			if (m_Status == EnvironmentStatus::None)
				Play();
			if (m_Status == EnvironmentStatus::PauseMode)
				Resume();
		}
	}

	void Environment::DisplayObjectInspecterGUI()
	{
		auto flags = ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysUseWindowPadding;
		ImGui::Begin("Object Inspector", &m_ObjectInspectorWindowOpen, flags);

		ImGui::PushItemWidth(ImGui::GetWindowWidth());
		ImGui::ListBoxHeader("##Inspector", -1, 30);

		//show menu when right clicking
		if (ImGui::BeginPopupContextItem("Inspector Popup"))
		{
			if (ImGui::Selectable("Add Particle System")) {
				ParticleSystem pso;
				m_ParticleSystems.push_back(pso);
			}
			ImGui::EndPopup();
		}

		for (int i = 0; i < m_ParticleSystems.size(); i++)
		{
			auto& particleSystem = m_ParticleSystems[i];

			ImGui::PushID(particleSystem.m_ID);


			if (ImGui::Selectable((particleSystem.m_Name.size() > 0) ? particleSystem.m_Name.c_str() : "No Name", &particleSystem.m_Selected)) {
				//if this is selected. deselect all other particle systems
				for (auto& particle : m_ParticleSystems) {
					if (particle.m_ID != particleSystem.m_ID)
						particle.m_Selected = false;
				}
			}

			//show menu when right clicking
			if (ImGui::BeginPopupContextItem("Object Popup"))
			{
				if (ImGui::Selectable("Edit"))
					particleSystem.m_EditorOpen = !particleSystem.m_EditorOpen;

				if (ImGui::Selectable("Delete"))
					m_ParticleSystems.erase(m_ParticleSystems.begin() + i);

				if (ImGui::Selectable("Rename"))
					particleSystem.m_RenameTextOpen = !particleSystem.m_RenameTextOpen;

				ImGui::EndPopup();
			}

			//display particle system buttons only if it is selected
			if (particleSystem.m_Selected) {
				if (ImGui::Button("Edit"))
					particleSystem.m_EditorOpen = !particleSystem.m_EditorOpen;

				ImGui::SameLine();
				if (ImGui::Button("Delete"))
					m_ParticleSystems.erase(m_ParticleSystems.begin() + i);

				ImGui::SameLine();
				if (ImGui::Button("Rename"))
					particleSystem.m_RenameTextOpen = !particleSystem.m_RenameTextOpen;
			}

			ImGui::Spacing();

			if (particleSystem.m_RenameTextOpen) {
				auto flags = ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue;
				if (ImGui::InputText("Name", &particleSystem.m_Name, flags)) {
					particleSystem.m_RenameTextOpen = !particleSystem.m_RenameTextOpen;
				}
			}

			ImGui::PopID();
		}



		ImGui::ListBoxFooter();

		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 30.0f);

		if (ImGui::Button("Add Particle System"))
		{
			ParticleSystem pso;
			m_ParticleSystems.push_back(pso);
		}

		ImGui::End();
	}

	void Environment::DisplayEnvironmentStatusGUI()
	{
		ImGui::Begin("Environment Status", &m_EnvironmentStatusWindowOpen);

		ImGui::Text("Particle Count :");
		ImGui::SameLine();

		unsigned int activeParticleCount = 0;
		for (ParticleSystem& pso : m_ParticleSystems)
			activeParticleCount += pso.m_ActiveParticleCount;

		ImGui::TextColored({ 0.0f,1.0f,0.0f,1.0f }, std::to_string(activeParticleCount).c_str());

		if (ImGui::TreeNode("Detailed Particle Distribution")) {

			for (auto& pso : m_ParticleSystems)
			{
				if (ImGui::TreeNode(pso.m_Name.c_str())) {

					ImGui::Text("Particle Count :");

					ImGui::SameLine();
					ImGui::TextColored({ 0.0f,1.0f,0.0f,1.0f }, std::to_string(pso.m_ActiveParticleCount).c_str());
					ImGui::TreePop();
				}

				ImGui::Spacing();
			}

			ImGui::TreePop();
		}


		ImGui::End();
	}

	void Environment::DisplayEnvironmentControlsGUI()
	{
		//return if window is not open
		if (!m_EnvironmentControlsWindowOpen)
			return;


		ImGui::Begin("Controls", &m_EnvironmentControlsWindowOpen);

		int width = ImGui::GetWindowSize().x;
		ImGui::SetCursorPosX(width / 2 - 20);

		if (m_Status == EnvironmentStatus::PlayMode || m_Status == EnvironmentStatus::PauseMode) {
			if (ImGui::ImageButton((ImTextureID)m_StopButtonTexture.GetID(), ImVec2(30, 20), ImVec2(0, 0), ImVec2(1, 1), 1)) {
				Stop();
			}
		}
		else {
			if (ImGui::ImageButton((ImTextureID)m_PlayButtonTexture.GetID(), ImVec2(30, 20), ImVec2(0, 0), ImVec2(1, 1), 1)) {
				Play();
			}
		}

		ImGui::SameLine();
		if (m_Status == EnvironmentStatus::PlayMode) {
			if (ImGui::ImageButton((ImTextureID)m_PauseButtonTexture.GetID(), ImVec2(30, 20), ImVec2(0, 0), ImVec2(1, 1), 1))
				Pause();
		}
		else if (m_Status == EnvironmentStatus::PauseMode) {
			if (ImGui::ImageButton((ImTextureID)m_ResumeButtonTexture.GetID(), ImVec2(30, 20), ImVec2(0, 0), ImVec2(1, 1), 1))
				Resume();
		}

		ImGui::End();
	}

	void Environment::DisplayMainMenuBarGUI()
	{
		int MenuBarHeight = 0;
		if (ImGui::BeginMainMenuBar()) {

			if (ImGui::BeginMenu("Edit")) {

				if (ImGui::MenuItem("Clear Particle Systems")) {
					m_ParticleSystems.clear();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Window")) {

				if (ImGui::MenuItem("Environment Controls"))
					m_EnvironmentControlsWindowOpen = !m_EnvironmentControlsWindowOpen;

				if (ImGui::MenuItem("Object Inspector"))
					m_ObjectInspectorWindowOpen = !m_ObjectInspectorWindowOpen;

				if (ImGui::MenuItem("General Settings"))
					m_GeneralSettingsWindowOpen = !m_GeneralSettingsWindowOpen;

				if (ImGui::MenuItem("Environment Status"))
					m_EnvironmentStatusWindowOpen = !m_EnvironmentStatusWindowOpen;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Editor Style")) {

				if (ImGui::MenuItem("Dark"))
					ImGui::StyleColorsDark();

				if (ImGui::MenuItem("Light"))
					ImGui::StyleColorsLight();

				if (ImGui::MenuItem("Classic"))
					ImGui::StyleColorsClassic();

				ImGui::EndMenu();
			}

			MenuBarHeight = ImGui::GetWindowSize().y;

			ImGui::EndMainMenuBar();
		}

		//TODO change this to a seperate function
		ImGuiViewport viewport;
		viewport.Size = ImVec2(Window::GetSize().x, Window::GetSize().y);
		viewport.Pos = ImVec2(0, MenuBarHeight);
		ImGui::DockSpaceOverViewport(&viewport, ImGuiDockNodeFlags_PassthruDockspace, 0);
	}

	void Environment::Play()
	{
		assert(m_Status == EnvironmentStatus::None);
		m_Status = EnvironmentStatus::PlayMode;
	}

	void Environment::Stop()
	{
		assert(m_Status == EnvironmentStatus::PlayMode || m_Status == EnvironmentStatus::PauseMode);
		m_Status = EnvironmentStatus::None;

		for (ParticleSystem& obj : m_ParticleSystems) {
			obj.ClearParticles();
		}
	}

	void Environment::Pause()
	{
		assert(m_Status == EnvironmentStatus::PlayMode);
		m_Status = EnvironmentStatus::PauseMode;
	}

	void Environment::Resume()
	{
		assert(m_Status == EnvironmentStatus::PauseMode);
		m_Status = EnvironmentStatus::PlayMode;
	}
}