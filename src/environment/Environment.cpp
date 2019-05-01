#include <pch.h>
#include "Environment.h"

namespace ALZ {

	Environment::Environment()
	{
		m_PlayButtonTexture.Init("res/PlayButton.png", 3);
		m_PauseButtonTexture.Init("res/PauseButton.png", 3);
		m_ResumeButtonTexture.Init("res/ResumeButton.png", 3);
		m_StopButtonTexture.Init("res/StopButton.png", 3);


		//setup ImGui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		SetEditorStyle(EditorStyle::Dark_Gray);

		ImGui_ImplGlfw_InitForOpenGL(&Window::GetWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 400");
		ImGuiIO& io = ImGui::GetIO(); (void)io;

		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking

		timeStart = 0;
		timeEnd = 0;

		AddPS();

		GaussianBlur::Init();
		RegisterEnvironmentInputKeys();
	}

	Environment::~Environment()
	{
		InspectorObjects.clear();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void Environment::Update()
	{
		timeEnd = clock();
		float deltaTime = (timeEnd - timeStart) / 1000.0f;
		timeStart = timeEnd;

		Window::Update();
		m_Camera.Update(deltaTime);

		if (m_Status == EnvironmentStatus::PlayMode) {
			for (Inspector_obj_ptr& obj : InspectorObjects)
				obj->Update(deltaTime, m_Camera);
		}

		if (Window::WindowSizeChangedSinceLastFrame())
			m_FrameBuffer.SetSize(Window::GetSize());

		m_Background.BaseColor = settings.BackgroundColor;
		m_Background.BaseLight = settings.BaseBackgroundLight;
	}

	void Environment::Render()
	{
		m_FrameBuffer.Bind();
		glClear(GL_COLOR_BUFFER_BIT);

		for (Inspector_obj_ptr& obj : InspectorObjects)
		{
			if (obj->Type == InspectorObjectType::RadiaLightType) {
				RadialLight* light = static_cast<RadialLight*>(obj.get());
				m_Background.SubmitLight(*light);
			}
		}

		m_Background.Render(m_Camera);

		if (m_Status == EnvironmentStatus::None) {
			m_FrameBuffer.RenderToScreen();
			m_FrameBuffer.Unbind();
			return;
		}

		//stuff to only render in play mode
		for (Inspector_obj_ptr& obj : InspectorObjects)
			obj->Render(m_Camera);

		m_FrameBuffer.Unbind();

		if (settings.BlurEnabled)
			GaussianBlur::Blur(m_FrameBuffer, settings.BlurScale, settings.BlurStrength, settings.BlurGaussianSigma);

		m_FrameBuffer.Bind();

		if (m_SaveNextFrameAsImage)
		{
			Image image = Image::FromFrameBuffer(m_FrameBuffer, settings.ImageResolution.x, settings.ImageResolution.y);
			image.SaveToFile(settings.GetImageSaveLocation() + '/' + settings.ImageFileName, settings.ImageFormat);
			m_SaveNextFrameAsImage = false;
		}
		m_FrameBuffer.RenderToScreen();
	}

	void Environment::RenderGUI()
	{
		if (m_HideGUI)
			return;


		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		DisplayMainMenuBarGUI();
		DisplayEnvironmentControlsGUI();
		DisplayObjectInspecterGUI();
		settings.DisplayGUI();
		DisplayEnvironmentStatusGUI();

		for (Inspector_obj_ptr& obj : InspectorObjects)
			obj->DisplayGUI(m_Camera);

		m_InputManager.DisplayGUI();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void Environment::HandleInput()
	{
		m_InputManager.HandleInput();	
	}

	void Environment::DisplayObjectInspecterGUI()
	{
		if (!m_ObjectInspectorWindowOpen)
			return;

		auto flags = ImGuiWindowFlags_::ImGuiWindowFlags_AlwaysUseWindowPadding;
		ImGui::Begin("Object Inspector", &m_ObjectInspectorWindowOpen, flags);

		ImGui::PushItemWidth(ImGui::GetWindowWidth());
		ImGui::ListBoxHeader("##Inspector", -1, 30);

		//show menu when right clicking
		if (ImGui::BeginPopupContextItem("Inspector Popup"))
		{
			if (ImGui::Selectable("Add Particle System")) {
				AddPS();
			}

			ImGui::EndPopup();
		}

		for (int i = 0; i < InspectorObjects.size(); i++)
		{
			Inspector_obj_ptr& particleSystem = InspectorObjects[i];

			ImGui::PushID(particleSystem->m_ID);


			if (ImGui::Selectable((particleSystem->m_Name.size() > 0) ? particleSystem->m_Name.c_str() : "No Name", &particleSystem->m_Selected)) {
				//if this is selected. deselect all other particle systems
				for (auto& particle : InspectorObjects) {
					if (particleSystem->m_ID != particleSystem->m_ID)
						particleSystem->m_Selected = false;
				}
			}

			//show menu when right clicking
			if (ImGui::BeginPopupContextItem("Object Popup"))
			{
				if (ImGui::Selectable("Edit"))
					particleSystem->m_EditorOpen = !particleSystem->m_EditorOpen;

				if (ImGui::Selectable("Delete")) {
					delete InspectorObjects[i].get();
					InspectorObjects[i].release();
					InspectorObjects.erase(InspectorObjects.begin() + i);
					ImGui::PopID();
					continue;
				}

				if (ImGui::Selectable("Rename"))
					particleSystem->m_RenameTextOpen = !particleSystem->m_RenameTextOpen;

				ImGui::EndPopup();
			}

			//display particle system buttons only if it is selected
			if (particleSystem->m_Selected) {
				if (ImGui::Button("Edit"))
					particleSystem->m_EditorOpen = !particleSystem->m_EditorOpen;

				ImGui::SameLine();
				if (ImGui::Button("Delete")) {
					delete InspectorObjects[i].get();
					InspectorObjects[i].release();
					InspectorObjects.erase(InspectorObjects.begin() + i);
					ImGui::PopID();
					continue;
				}

				ImGui::SameLine();
				if (ImGui::Button("Rename"))
					particleSystem->m_RenameTextOpen = !particleSystem->m_RenameTextOpen;
			}

			ImGui::Spacing();

			if (particleSystem->m_RenameTextOpen) {
				auto flags = ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue;
				if (ImGui::InputText("Name", &particleSystem->m_Name, flags)) {
					particleSystem->m_RenameTextOpen = !particleSystem->m_RenameTextOpen;
				}
			}

			ImGui::PopID();
		}

		ImGui::ListBoxFooter();

		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 60.0f);

		if (ImGui::Button("Add Particle System"))
		{
			AddPS();
		}


		if (ImGui::Button("Add Radial Light")) {
			AddRadialLight();
		}

		ImGui::End();
	}

	void Environment::DisplayEnvironmentStatusGUI()
	{
		if (!m_EnvironmentStatusWindowOpen)
			return;

		ImGui::Begin("Environment Status", &m_EnvironmentStatusWindowOpen);

		ImGui::Text("Particle Count :");
		ImGui::SameLine();

		unsigned int activeParticleCount = 0;
		for (Inspector_obj_ptr& pso : InspectorObjects)
		{
			if (pso->Type == InspectorObjectType::ParticleSystemType) {
				ParticleSystem* ps = static_cast<ParticleSystem*>(pso.get());
				activeParticleCount += ps->m_ActiveParticleCount;
			}

		}

		ImGui::TextColored({ 0.0f,1.0f,0.0f,1.0f }, std::to_string(activeParticleCount).c_str());

		if (ImGui::TreeNode("Detailed Particle Distribution")) {

			for (Inspector_obj_ptr& pso : InspectorObjects)
			{
				if (pso->Type == InspectorObjectType::ParticleSystemType) {

					ParticleSystem* ps = static_cast<ParticleSystem*>(pso.get());

					if (ImGui::TreeNode(pso->m_Name.c_str())) {

						ImGui::Text("Particle Count :");

						ImGui::SameLine();
						ImGui::TextColored({ 0.0f,1.0f,0.0f,1.0f }, std::to_string(ps->m_ActiveParticleCount).c_str());
						ImGui::TreePop();
					}
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
			if (ImGui::ImageButton((ImTextureID)m_StopButtonTexture.TextureID, ImVec2(30, 20), ImVec2(0, 0), ImVec2(1, 1), 1)) {
				Stop();
			}
		}
		else {
			if (ImGui::ImageButton((ImTextureID)m_PlayButtonTexture.TextureID, ImVec2(30, 20), ImVec2(0, 0), ImVec2(1, 1), 1)) {
				Play();
			}
		}

		ImGui::SameLine();
		if (m_Status == EnvironmentStatus::PlayMode) {
			if (ImGui::ImageButton((ImTextureID)m_PauseButtonTexture.TextureID, ImVec2(30, 20), ImVec2(0, 0), ImVec2(1, 1), 1))
				Pause();
		}
		else if (m_Status == EnvironmentStatus::PauseMode) {
			if (ImGui::ImageButton((ImTextureID)m_ResumeButtonTexture.TextureID, ImVec2(30, 20), ImVec2(0, 0), ImVec2(1, 1), 1))
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
					InspectorObjects.clear();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Window")) {

				if (ImGui::MenuItem("Environment Controls"))
					m_EnvironmentControlsWindowOpen = !m_EnvironmentControlsWindowOpen;

				if (ImGui::MenuItem("Object Inspector"))
					m_ObjectInspectorWindowOpen = !m_ObjectInspectorWindowOpen;

				if (ImGui::MenuItem("General Settings"))
					settings.GeneralSettingsWindowOpen = !settings.GeneralSettingsWindowOpen;

				if (ImGui::MenuItem("Environment Status"))
					m_EnvironmentStatusWindowOpen = !m_EnvironmentStatusWindowOpen;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Editor Style")) {

				if (ImGui::MenuItem("Dark(Transparent)"))
					SetEditorStyle(EditorStyle::DarkTransparent);

				if (ImGui::MenuItem("Dark/Gray(default)"))
					SetEditorStyle(EditorStyle::Dark_Gray);

				if (ImGui::MenuItem("Dark"))
					SetEditorStyle(EditorStyle::Dark);

				if (ImGui::MenuItem("Light"))
					SetEditorStyle(EditorStyle::Light);

				if (ImGui::MenuItem("Classic"))
					SetEditorStyle(EditorStyle::Classic);

				ImGui::EndMenu();
			}


			if (ImGui::BeginMenu("Help")) {

				if (ImGui::MenuItem("Controls"))
					m_InputManager.ControlsWindowOpen = !m_InputManager.ControlsWindowOpen;
					
				ImGui::EndMenu();
			}

			MenuBarHeight = ImGui::GetWindowSize().y;

			ImGui::EndMainMenuBar();
		}

		//TODO change this to a seperate function
		ImGuiViewport viewport;
		viewport.Size = ImVec2(Window::GetSize().x, Window::GetSize().y);
		viewport.Pos = ImVec2(0, MenuBarHeight);
		ImGui::DockSpaceOverViewport(&viewport, ImGuiDockNodeFlags_PassthruCentralNode, 0);
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

		for (Inspector_obj_ptr& obj : InspectorObjects) {
			if (obj->Type == InspectorObjectType::ParticleSystemType) {
				ParticleSystem* ps = static_cast<ParticleSystem*>(obj.get());
				ps->ClearParticles();
			}
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

	void Environment::RegisterEnvironmentInputKeys()
	{
		m_InputManager.RegisterKey(GLFW_KEY_F5, "Play/Resume", [this]() {
			if (m_Status == EnvironmentStatus::None)
				Play();
			if (m_Status == EnvironmentStatus::PauseMode)
				Resume();
		});

		m_InputManager.RegisterKey(GLFW_KEY_F1, "Hide Menus", [this]() { m_HideGUI = !m_HideGUI; });

		m_InputManager.RegisterKey(GLFW_KEY_F11, "Capture Screenshot", [this]() 
		{
			m_SaveNextFrameAsImage = true;
		});

		m_InputManager.RegisterKey(GLFW_KEY_SPACE, "Clear All Particles", [this]()
		{
			for (Inspector_obj_ptr& obj : InspectorObjects) {
				if (obj->Type == InspectorObjectType::ParticleSystemType) {
					ParticleSystem* ps = static_cast<ParticleSystem*>(obj.get());
					ps->ClearParticles();
				}
			}
		});

		m_InputManager.RegisterKey(GLFW_KEY_W, "Move Camera Up", [this]() { m_Camera.SetPosition(m_Camera.Position + glm::vec3(0, -10.0f, 0)); }           , GLFW_REPEAT);
		m_InputManager.RegisterKey(GLFW_KEY_S, "Move Camera Down", [this]() { m_Camera.SetPosition(m_Camera.Position + glm::vec3(0, 10.0f, 0)); }        , GLFW_REPEAT);
		m_InputManager.RegisterKey(GLFW_KEY_D, "Move Camera To The Right", [this]() { m_Camera.SetPosition(m_Camera.Position + glm::vec3(-10.0f, 0, 0)); }, GLFW_REPEAT);
		m_InputManager.RegisterKey(GLFW_KEY_A, "Move Camera To The Left", [this]() { m_Camera.SetPosition(m_Camera.Position + glm::vec3(10.0f, 0, 0)); }  , GLFW_REPEAT);

		m_InputManager.RegisterMouseKey(GLFW_MOUSE_BUTTON_LEFT, "Spawn Particles If Spawn Particles On Mouse Mode Is Selected", [this]() {
			if (m_Status == EnvironmentStatus::PlayMode) 
			{
				if (!ImGui::GetIO().WantCaptureMouse) 
				{
					for (Inspector_obj_ptr& obj : InspectorObjects) {

						if (obj->Type == InspectorObjectType::ParticleSystemType) {

							ParticleSystem* ps = static_cast<ParticleSystem*>(obj.get());
							if (ps->m_Customizer.Mode == SpawnMode::SpawnOnMousePosition) {
								if (!m_MousePressedLastFrame)
									ps->m_TimeTillNextParticleSpawn = 0.0f;
								ps->m_ShouldSpawnParticles = true;
							}

						}

					}
					m_MousePressedLastFrame = true;
				}
				else 
				{
					for (Inspector_obj_ptr& obj : InspectorObjects) {
						if (obj->Type == InspectorObjectType::ParticleSystemType) {

							ParticleSystem* ps = static_cast<ParticleSystem*>(obj.get());

							if (ps->m_Customizer.Mode == SpawnMode::SpawnOnMousePosition) {
								ps->m_ShouldSpawnParticles = false;
							}
						}
					}
					m_MousePressedLastFrame = false;
				}
			}
		});

		//stop spawning particles when mouse key is released
		//NOTE: no description means it won't be displayed in the controls window
		m_InputManager.RegisterMouseKey(GLFW_MOUSE_BUTTON_LEFT, "", [this]() {
			if (m_Status == EnvironmentStatus::PlayMode)
			{
				if (!ImGui::GetIO().WantCaptureMouse)
				{
					for (Inspector_obj_ptr& obj : InspectorObjects) {

						if (obj->Type == InspectorObjectType::ParticleSystemType) {

							ParticleSystem* ps = static_cast<ParticleSystem*>(obj.get());

							if (ps->m_Customizer.Mode == SpawnMode::SpawnOnMousePosition) {
								ps->m_ShouldSpawnParticles = false;
							}
						}
					}
					m_MousePressedLastFrame = false;
				}
			}
		}, GLFW_RELEASE);
	}

	void Environment::AddPS()
	{
		std::unique_ptr<ParticleSystem> startingPS = std::make_unique<ParticleSystem>();
		Inspector_obj_ptr startingPSi((InspectorInterface*)(startingPS.release()));

		InspectorObjects.push_back(std::move(startingPSi));
	}

	void Environment::AddRadialLight()
	{
		std::unique_ptr<RadialLight> startingPS = std::make_unique<RadialLight>();
		Inspector_obj_ptr startingPSi((InspectorInterface*)(startingPS.release()));

		InspectorObjects.push_back(std::move(startingPSi));
	}
}