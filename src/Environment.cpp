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

		for (int i = 0; i < InspectorObjects.size(); i++) {
			if (InspectorObjects[i]->ToBeDeleted)
				InspectorObjects.erase(InspectorObjects.begin() + i);
		}


		if (m_Status == EnvironmentStatus::PlayMode) {
			for (Inspector_obj_ptr& obj : InspectorObjects)
				obj->Update(deltaTime, m_Camera);
		}

		if (Window::WindowSizeChangedSinceLastFrame())
			m_FrameBuffer.SetSize(Window::WindowSize);

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

		for (int i = 0; i < InspectorObjects.size(); i++)
		{

			ImGui::PushID(InspectorObjects[i]->ID);

			if (ImGui::Selectable((InspectorObjects[i]->m_Name.size() > 0) ? InspectorObjects[i]->m_Name.c_str() : "No Name", &InspectorObjects[i]->Selected)) {

				//if this is selected. deselect all other particle systems
				for (auto& particle : InspectorObjects) {
					if (particle->ID != InspectorObjects[i]->ID)
						particle->Selected = false;
				}
			}

			//show menu when right clicking
			if (ImGui::BeginPopupContextItem("Object Popup"))
			{
				if (ImGui::Selectable("Edit"))
					InspectorObjects[i]->EditorOpen = !InspectorObjects[i]->EditorOpen;

				if (ImGui::Selectable("Delete")) {
					InspectorObjects[i]->ToBeDeleted = true;
				}

				if (ImGui::Selectable("Duplicate")) 
				{
					Duplicate(*InspectorObjects[i]);
					ImGui::EndPopup();
					ImGui::PopID();
					continue;
				}

				if (ImGui::Selectable("Rename"))
					InspectorObjects[i]->RenameTextOpen = !InspectorObjects[i]->RenameTextOpen;


				ImGui::EndPopup();
			}

			//display particle system buttons only if it is selected
			if (InspectorObjects[i]->Selected) {
				if (ImGui::Button("Edit"))
					InspectorObjects[i]->EditorOpen = !InspectorObjects[i]->EditorOpen;

				ImGui::SameLine();
				if (ImGui::Button("Delete")) 
					InspectorObjects[i]->ToBeDeleted = true;

				ImGui::SameLine();
				if (ImGui::Button("Rename"))
					InspectorObjects[i]->RenameTextOpen = !InspectorObjects[i]->RenameTextOpen;

				ImGui::SameLine();

				if (ImGui::Button("Find"))
					FocusCameraOnObject(*InspectorObjects[i]);
			}

			ImGui::Spacing();

			if (InspectorObjects[i]->RenameTextOpen) {
				auto flags = ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue;
				if (ImGui::InputText("Name", &InspectorObjects[i]->m_Name, flags)) {
					InspectorObjects[i]->RenameTextOpen = !InspectorObjects[i]->RenameTextOpen;
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

		ImGui::Text("Global Particle Count :");
		ImGui::SameLine();

		unsigned int activeParticleCount = 0;
		for (Inspector_obj_ptr& pso : InspectorObjects)
		{
			if (pso->Type == InspectorObjectType::ParticleSystemType) {
				ParticleSystem* ps = static_cast<ParticleSystem*>(pso.get());
				activeParticleCount += ps->ActiveParticleCount;
			}
		}
		ImGui::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, std::to_string(activeParticleCount).c_str());

		ImGui::Separator();

		for (Inspector_obj_ptr& pso : InspectorObjects)
		{
			if (pso->Type == InspectorObjectType::ParticleSystemType) {

				ParticleSystem* ps = static_cast<ParticleSystem*>(pso.get());

				ImGui::Text((pso->m_Name + ":").c_str());
				ImGui::SameLine();
				ImGui::Text("Particle Count = ");
				ImGui::SameLine();
				ImGui::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, std::to_string(ps->ActiveParticleCount).c_str());
				ImGui::Separator();
			}

			ImGui::Spacing();
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
		viewport.Size = ImVec2(Window::WindowSize.x, Window::WindowSize.y);
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

		m_InputManager.RegisterKey(GLFW_KEY_W, "Move Camera Up", [this]() { m_Camera.SetPosition(m_Camera.Position + glm::vec2(0.0f, -10.0f)); }, GLFW_REPEAT);
		m_InputManager.RegisterKey(GLFW_KEY_S, "Move Camera Down", [this]() { m_Camera.SetPosition(m_Camera.Position + glm::vec2(0.0f, 10.0f)); }, GLFW_REPEAT);
		m_InputManager.RegisterKey(GLFW_KEY_D, "Move Camera To The Right", [this]() { m_Camera.SetPosition(m_Camera.Position + glm::vec2(-10.0f, 0.0f)); }, GLFW_REPEAT);
		m_InputManager.RegisterKey(GLFW_KEY_A, "Move Camera To The Left", [this]() { m_Camera.SetPosition(m_Camera.Position + glm::vec2(10.0f, 0.0f)); }, GLFW_REPEAT);

		//delete keyboard shortcut
		m_InputManager.RegisterKey(GLFW_KEY_DELETE, "Delete Object", [this]() {

			for (int i = 0; i < InspectorObjects.size(); i++)
			{
				if (InspectorObjects[i]->Selected) {
					InspectorObjects[i]->ToBeDeleted = true;
					break;
				}
			}
		});

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

	void Environment::Duplicate(InspectorInterface& obj)
	{
		if (obj.Type == InspectorObjectType::ParticleSystemType) {
			std::unique_ptr<ParticleSystem> startingPS = std::make_unique<ParticleSystem>();
			Inspector_obj_ptr startingPSi((InspectorInterface*)(startingPS.release()));

			InspectorObjects.push_back(std::move(startingPSi));
			*InspectorObjects[InspectorObjects.size() - 1].get() = *static_cast<ParticleSystem*>(&obj);
			InspectorObjects[InspectorObjects.size() - 1]->m_Name += "-copy";
			InspectorObjects[InspectorObjects.size() - 1]->ID++;
		}
	}

	void Environment::FocusCameraOnObject(InspectorInterface& object)
	{
		InspectorObjectType type = object.Type;

		if (type == ALZ::ParticleSystemType) {
			ParticleSystem& ps = *static_cast<ParticleSystem*>(&object);

			switch (ps.Customizer.Mode)
			{
			case SpawnMode::SpawnOnPoint:

				m_Camera.SetPosition(glm::vec3(ps.Customizer.m_SpawnPosition.x * -GlobalScaleFactor, ps.Customizer.m_SpawnPosition.y * -GlobalScaleFactor, 0.0f)
					+ glm::vec3(Window::WindowSize.x / 2, Window::WindowSize.y / 2, 0.0f));
				break;

			case SpawnMode::SpawnOnCircle:
				m_Camera.SetPosition(glm::vec3(ps.Customizer.m_CircleOutline.Position.x, ps.Customizer.m_CircleOutline.Position.y, 0.0f)
								   + glm::vec3(Window::WindowSize.x / 2, Window::WindowSize.y / 2, 0.0f));
				break;

			case SpawnMode::SpawnOnLine:

				m_Camera.SetPosition(glm::vec3(ps.Customizer.m_LinePosition.x * -GlobalScaleFactor, ps.Customizer.m_LinePosition.y * -GlobalScaleFactor, 0.0f)
								   + glm::vec3(Window::WindowSize.x / 2, Window::WindowSize.y / 2, 0.0f));
				break;
			}
		}
		else if (type == ALZ::RadiaLightType) {
			RadialLight& ps = *static_cast<RadialLight*>(&object);

			m_Camera.SetPosition(glm::vec3(ps.Position.x, ps.Position.y, 0.0f)
							   + glm::vec3(Window::WindowSize.x / 2, Window::WindowSize.y / 2, 0.0f));
		}
	}
}