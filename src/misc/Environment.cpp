#include <pch.h>
#include "Environment.h"

namespace ALZ {

	Environment::Environment() : 
		m_EnvironmentSaveBrowser(FileManager::ApplicationFolder, "Save Environment")
	{
		m_PlayButtonTexture = Renderer::CreateTexture();
		m_PauseButtonTexture = Renderer::CreateTexture();
		m_ResumeButtonTexture = Renderer::CreateTexture();
		m_StopButtonTexture = Renderer::CreateTexture();

		m_PlayButtonTexture->SetImage(Image::LoadFromFile("res/PlayButton.png", 3));
		m_PauseButtonTexture->SetImage(Image::LoadFromFile("res/PauseButton.png", 3));
		m_ResumeButtonTexture->SetImage(Image::LoadFromFile("res/ResumeButton.png", 3));
		m_StopButtonTexture->SetImage(Image::LoadFromFile("res/StopButton.png", 3));

		AddPS();

		GaussianBlur::Init();
		RegisterEnvironmentInputKeys();

	}

	Environment::~Environment()
	{
		InspectorObjects.clear();
		glfwRestoreWindow(&Window::GetWindow());
		Window::SetWindowLaunchSize();
		Window::CenterWindow();
	}

	void Environment::Update()
	{
		timeEnd = clock();
		float deltaTime = (timeEnd - timeStart) / 1000.0f;
		timeStart = timeEnd;

		m_Camera.Update(deltaTime);

		for (int i = 0; i < InspectorObjects.size(); i++) {
			if (InspectorObjects[i]->ToBeDeleted)
				InspectorObjects.erase(InspectorObjects.begin() + i);
		}

		for (Inspector_obj_ptr& obj : InspectorObjects) {
			if(m_Status == EnvironmentStatus::PlayMode || m_Status == EnvironmentStatus::ExportMode)
				obj->Update(deltaTime);
		}

		if (Window::WindowSizeChangedSinceLastFrame())
			m_RenderSurface.SetSize(Window::FramebufferSize);

		if (m_Status == EnvironmentStatus::PlayMode || m_Status == EnvironmentStatus::ExportMode)
			m_TimeSincePlayModeStarted += deltaTime;

		//this code will be changed and refactored, that is why it is strcutured badly
		if (m_Status == EnvironmentStatus::ExportMode) {
			if (m_TimeSincePlayModeStarted > m_ExportCamera.ImageCaptureTime)
			{
				CaptureFrameAndExport();
				m_ExportedEverything = true;
			}

			if (m_ExportedEverything) {
				Stop();
				m_ExportedEverything = false;
				m_HideGUI = false;
			}
		}
	}

	void Environment::Render()
	{
		Renderer::BeginScene(m_Camera);
		m_RenderSurface.m_FrameBuffer->Bind();
		Renderer::ClearScreen();

		for (Inspector_obj_ptr& obj : InspectorObjects)
		{
			if (obj->Type == RadiaLightType) {
				RadialLight* light = static_cast<RadialLight*>(obj.get());
				m_Background.SubmitLight(*light);
			}
			else if (obj->Type == SpotLightType) {
				SpotLight* light = static_cast<SpotLight*>(obj.get());
				m_Background.SubmitLight(*light);
			}
		}

		m_Background.Draw();

		if(m_Settings.ShowGrid && m_Status != EnvironmentStatus::PlayMode)
			m_Grid.Draw();

		if(m_Status == EnvironmentStatus::None)
			for (Inspector_obj_ptr& obj : InspectorObjects)
				if(obj->Selected)
					m_Gizmo.Draw(obj->GetPositionRef(), m_InputManager.GetMousePositionNDC());

		//Render world space gui here because we need camera information for that
		for (Inspector_obj_ptr& obj : InspectorObjects)
		{
			if(obj->Selected)
				if (obj->Type == InspectorObjectType::ParticleSystemType)
				{
					ParticleSystem* ps = (ParticleSystem*)obj.get();
					if (ps->Customizer.Mode == SpawnMode::SpawnOnLine)
						ps->Customizer.m_Line.Draw();
					else if (ps->Customizer.Mode == SpawnMode::SpawnOnCircle || ps->Customizer.Mode == SpawnMode::SpawnInsideCircle && ps->Selected)
						ps->Customizer.m_CircleOutline.Draw();
				}
		}

		if (m_Status == EnvironmentStatus::None) {
			m_ExportCamera.DrawOutline();
			m_RenderSurface.RenderToScreen();
			m_RenderSurface.m_FrameBuffer->Unbind();
			return;
		}

		//stuff to only render in play mode and export mode
		for (Inspector_obj_ptr& obj : InspectorObjects)
			obj->Draw();


		if (m_Settings.BlurEnabled)
			GaussianBlur::Blur(m_RenderSurface, m_Settings.BlurRadius);

		//draw this after post processing because we do not want the line blured
		m_ExportCamera.DrawOutline();

		m_RenderSurface.RenderToScreen();

		Renderer::EndScene();

		if (m_SaveNextFrameAsImage)
			CaptureFrameAndExport();

		m_RenderSurface.m_FrameBuffer->Unbind();
	}

	void Environment::RenderGUI()
	{
		if (m_HideGUI)
			return;

		ImGuiWrapper::NewFrame();

		DisplayMainMenuBarGUI();

		float menuBarHeight = ImGui::GetFrameHeightWithSpacing();
		ImGuiViewport viewport;
		viewport.Size = ImVec2(Window::FramebufferSize.x, Window::FramebufferSize.y -menuBarHeight);
		viewport.Pos = ImVec2(Window::Position.x, Window::Position.y + menuBarHeight);
		ImGui::DockSpaceOverViewport(&viewport, ImGuiDockNodeFlags_PassthruCentralNode, 0);

		DisplayEnvironmentControlsGUI();
		DisplayObjectInspecterGUI();
		m_Settings.DisplayGUI();
		m_ExportCamera.DisplayGUI();
		DisplayEnvironmentStatusGUI();
		m_Background.DisplayGUI();

		for (Inspector_obj_ptr& obj : InspectorObjects)
			obj->DisplayGUI();

		m_InputManager.DisplayGUI();

		m_EnvironmentSaveBrowser.DisplayGUI([this](const std::string& path) {
			SaveEnvironment(*this, path + ".env");
			m_SaveLocationSelected = true;
			});

		ImGuiWrapper::Render();
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

		ImGui::PushItemWidth(ImGui::GetWindowWidth() - 30);
		if (ImGui::ListBoxHeader("##Inspector", -1, 30)) {

			for (int i = 0; i < InspectorObjects.size(); i++)
			{

				ImGui::PushID(InspectorObjects[i].get());

				if (ImGui::Selectable((InspectorObjects[i]->m_Name.size() > 0) ? InspectorObjects[i]->m_Name.c_str() : "No Name", &InspectorObjects[i]->Selected)) {

					//if this is selected. deselect all other particle systems
					for (auto& particle : InspectorObjects) {
						if (particle.get() != InspectorObjects[i].get())
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
		}

		ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 90.0f);

		if (ImGui::Button("Add Particle System"))
		{
			AddPS();
		}
		if (ImGui::Button("Add Radial Light")) {
			AddRadialLight();
		}
		if (ImGui::Button("Add Spot Light")) {
			AddSpotLight();
		}

		ImGui::End();
	}

	void Environment::DisplayEnvironmentStatusGUI()
	{
		if (!m_EnvironmentStatusWindowOpen)
			return;

		ImGui::Begin("Particle Status", &m_EnvironmentStatusWindowOpen);

		ImGui::Text("Global Particle Count :");
		ImGui::SameLine();

		unsigned int activeParticleCount = 0;
		for (Inspector_obj_ptr& object : InspectorObjects)
		{
			//if object is a particle system
			if (object->Type == InspectorObjectType::ParticleSystemType) {
				//cast it to a particle system pointer
				ParticleSystem* ps = static_cast<ParticleSystem*>(object.get());

				//increment active particles by how many particles are active in this particle system
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

		auto id = ImGui::GetWindowDockID();

		ImGui::End();

		ImGui::SetNextWindowDockID(id, ImGuiCond_Always);

		ImGui::Begin("PlayMode Mode Status", nullptr, ImGuiWindowFlags_NoSavedSettings);

		//this is to control how many decimal points we want to display
		std::stringstream stream;
		//we want 3 decimal places
		stream << std::setprecision(3) << m_TimeSincePlayModeStarted;
		ImGui::Text("Time Since PlayMode Mode Started :" );
		ImGui::SameLine();
		ImGui::TextColored({ 0.0f,0.8f,0.0f,1.0f }, stream.str().c_str());

		ImGui::End();
	}

	void Environment::DisplayEnvironmentControlsGUI()
	{
		//return if window is not open
		if (!m_EnvironmentControlsWindowOpen)
			return;

		ImGui::Begin("Controls", &m_EnvironmentControlsWindowOpen);
		if (m_Status == EnvironmentStatus::ExportMode)
		{
			ImGui::End();
			return;
		}

		int width = (int)ImGui::GetWindowSize().x;
		ImGui::SetCursorPosX((float)width / 2 - 20);

		if (m_Status == EnvironmentStatus::PlayMode || m_Status == EnvironmentStatus::PauseMode) {
			if (ImGui::ImageButton((void*)(uintptr_t)m_StopButtonTexture->GetRendererID(), ImVec2(30, 20), ImVec2(0, 0), ImVec2(1, 1), 1)) {
				Stop();
			}
		}

		else {
			if (ImGui::ImageButton((void*)(uintptr_t)m_PlayButtonTexture->GetRendererID(), ImVec2(30, 20), ImVec2(0, 0), ImVec2(1, 1), 1)) {
				PlayMode();
			}
			if (ImGui::Button("Export")) {
				m_HideGUI = true;
				ExportMode();
			}
		}

		ImGui::SameLine();
		if (m_Status == EnvironmentStatus::PlayMode) {
			if (ImGui::ImageButton((void*)(uintptr_t)m_PauseButtonTexture->GetRendererID(), ImVec2(30, 20), ImVec2(0, 0), ImVec2(1, 1), 1))
				Pause();
		}
		else if (m_Status == EnvironmentStatus::PauseMode) {
			if (ImGui::ImageButton((void*)(uintptr_t)m_ResumeButtonTexture->GetRendererID(), ImVec2(30, 20), ImVec2(0, 0), ImVec2(1, 1), 1))
				Resume();
		}

		ImGui::End();
	}

	void Environment::DisplayMainMenuBarGUI()
	{
		if (ImGui::BeginMainMenuBar()) {

			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Save"))
				{
					if (m_SaveLocationSelected)
						SaveEnvironment(*this, m_EnvironmentSaveBrowser.GetSelectedSavePath() + ".env");
					else
						m_EnvironmentSaveBrowser.OpenWindow();
				}

				if (ImGui::MenuItem("Save As"))
				{
					m_EnvironmentSaveBrowser.OpenWindow();
				}

				if (ImGui::MenuItem("Close Environment")) 
				{
					ShouldDelete = true;
				}

				if (ImGui::MenuItem("Exit"))
				{
					exit(0);
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit")) {

				if (ImGui::MenuItem("Clear Particle Systems")) {
					InspectorObjects.clear();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Window")) {

				ImGui::MenuItem("Environment Controls", nullptr, &m_EnvironmentControlsWindowOpen);
				ImGui::MenuItem("Object Inspector", nullptr, &m_ObjectInspectorWindowOpen);
				ImGui::MenuItem("General Settings",nullptr, &m_Settings.GeneralSettingsWindowOpen);
				ImGui::MenuItem("Environment Status", nullptr, &m_EnvironmentStatusWindowOpen);
				ImGui::MenuItem("Background Settings", nullptr, &m_Background.SettingsWindowOpen);
				ImGui::MenuItem("ExportMode Settings", nullptr, &m_ExportCamera.SettingsWindowOpen);

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

			ImGui::EndMainMenuBar();
		}
	}

	void Environment::PlayMode()
	{
		assert(m_Status == EnvironmentStatus::None);
		m_Status = EnvironmentStatus::PlayMode;
		m_TimeSincePlayModeStarted = 0.0f;
	}

	void Environment::ExportMode()
	{
		assert(m_Status == EnvironmentStatus::None);
		m_Status = EnvironmentStatus::ExportMode;
		m_TimeSincePlayModeStarted = 0.0f;
	}

	void Environment::Stop()
	{
		assert(m_Status == EnvironmentStatus::PlayMode || m_Status == EnvironmentStatus::PauseMode || m_Status == EnvironmentStatus::ExportMode);
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
		m_InputManager.RegisterKey(GLFW_KEY_F5, "PlayMode/Resume", [this]() {
			if (m_Status == EnvironmentStatus::None)
				PlayMode();
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
		std::unique_ptr<RadialLight> light = std::make_unique<RadialLight>();
		Inspector_obj_ptr lightObj((InspectorInterface*)(light.release()));

		InspectorObjects.push_back(std::move(lightObj));
	}

	void Environment::AddSpotLight()
	{
		std::unique_ptr<SpotLight> light = std::make_unique<SpotLight>();
		Inspector_obj_ptr lightObj((InspectorInterface*)(light.release()));

		InspectorObjects.push_back(std::move(lightObj));
	}

	void Environment::CaptureFrameAndExport()
	{
		Renderer::BeginScene(m_ExportCamera.RealCamera);

		m_ExportCamera.m_RenderSurface.SetSize(m_ExportCamera.m_ExportCameraSize * GlobalScaleFactor);
		m_ExportCamera.m_RenderSurface.m_FrameBuffer->Bind();

		for (Inspector_obj_ptr& obj : InspectorObjects)
		{
			if (obj->Type == InspectorObjectType::RadiaLightType) {
				RadialLight* light = static_cast<RadialLight*>(obj.get());
				m_Background.SubmitLight(*light);
			}
		}

		m_Background.Draw();

		for (Inspector_obj_ptr& obj : InspectorObjects)
			obj->Draw();

		if (m_Settings.BlurEnabled)
			GaussianBlur::Blur(m_ExportCamera.m_RenderSurface, m_Settings.BlurRadius);

		Image image = Image::FromFrameBuffer(m_ExportCamera.m_RenderSurface, m_ExportCamera.m_RenderSurface.GetSize());

		std::string saveTarget = m_ExportCamera.ImageSavePath;

		//add a default name if none is chosen
		if (saveTarget.back() == '\\')
			saveTarget.append("default name");

		image.SaveToFile(saveTarget, m_ExportCamera.SaveImageFormat);
		m_SaveNextFrameAsImage = false;

		Renderer::EndScene();
	}

	void Environment::Duplicate(InspectorInterface& obj)
	{
		//if this object is a particle system
		if (obj.Type == InspectorObjectType::ParticleSystemType) 
		{
			//make a new particle system
			InspectorObjects.push_back(std::make_unique<ParticleSystem>(*static_cast<ParticleSystem*>(&obj)));

			//add a -copy to the name of the new particle system to indicate that it was copied
			InspectorObjects[InspectorObjects.size() - 1]->m_Name += "-copy";
		}

		//if this object is a radial light
		else if (obj.Type == InspectorObjectType::RadiaLightType) 
		{
			//make a new radial light
			InspectorObjects.push_back(std::make_unique<RadialLight>(*static_cast<RadialLight*>(&obj)));

			//add a -copy to the name of the new light to indicate that it was copied
			InspectorObjects[InspectorObjects.size() - 1]->m_Name += "-copy";
		}

		//if this object is a spot light
		else if (obj.Type == InspectorObjectType::SpotLightType)
		{
			//make a new radial light
			InspectorObjects.push_back(std::make_unique<SpotLight>(*static_cast<SpotLight*>(&obj)));

			//add a -copy to the name of the new light to indicate that it was copied
			InspectorObjects[InspectorObjects.size() - 1]->m_Name += "-copy";
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
					+ glm::vec3(Window::FramebufferSize.x / 2, Window::FramebufferSize.y / 2, 0.0f));
				break;

			case SpawnMode::SpawnOnCircle:
				m_Camera.SetPosition(glm::vec3(ps.Customizer.m_CircleOutline.Position.x, ps.Customizer.m_CircleOutline.Position.y, 0.0f)
								   + glm::vec3(Window::FramebufferSize.x / 2, Window::FramebufferSize.y / 2, 0.0f));
				break;

			case SpawnMode::SpawnInsideCircle:
				m_Camera.SetPosition(glm::vec3(ps.Customizer.m_CircleOutline.Position.x, ps.Customizer.m_CircleOutline.Position.y, 0.0f)
					+ glm::vec3(Window::FramebufferSize.x / 2, Window::FramebufferSize.y / 2, 0.0f));
				break;

			case SpawnMode::SpawnOnLine:

				m_Camera.SetPosition(glm::vec3(ps.Customizer.m_LinePosition.x * -GlobalScaleFactor, ps.Customizer.m_LinePosition.y * -GlobalScaleFactor, 0.0f)
								   + glm::vec3(Window::FramebufferSize.x / 2, Window::FramebufferSize.y / 2, 0.0f));
				break;
			}
		}
		else if (type == ALZ::RadiaLightType) {
			RadialLight& ps = *static_cast<RadialLight*>(&object);

			m_Camera.SetPosition(glm::vec3(ps.Position.x, ps.Position.y, 0.0f)
							   + glm::vec3(Window::FramebufferSize.x / 2, Window::FramebufferSize.y / 2, 0.0f));
		}
	}
}