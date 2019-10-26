#include <pch.h>
#include "Environment.h"

namespace ALZ {

	Environment::Environment(const std::string& environmentFolderPath, const std::string& environmentName) :
		m_EnvironmentFolderPath(environmentFolderPath),
		m_EnvironmentName(environmentName)
	{
		m_PlayButtonTexture = Renderer::CreateTexture();
		m_PauseButtonTexture = Renderer::CreateTexture();
		m_ResumeButtonTexture = Renderer::CreateTexture();
		m_StopButtonTexture = Renderer::CreateTexture();

		m_PlayButtonTexture->SetImage(Image::LoadFromFile("res/PlayButton.png", 3));
		m_PauseButtonTexture->SetImage(Image::LoadFromFile("res/PauseButton.png", 3));
		m_ResumeButtonTexture->SetImage(Image::LoadFromFile("res/ResumeButton.png", 3));
		m_StopButtonTexture->SetImage(Image::LoadFromFile("res/StopButton.png", 3));

		//add starting particle system 
		AddInspectorObject(ParticleSystemType, "particle system"); //NOTE: if this environment was loaded from a file the loader will delete this object

		RegisterEnvironmentInputKeys();

		AssetManager::Init(environmentFolderPath);

		UpdateTitle();
	}

	Environment::~Environment()
	{
		InputManager::ClearKeys();
		AssetManager::Terminate();
		InspectorObjects.clear();
		Window::Restore();
		Window::SetSize(glm::ivec2(WINDOW_SIZE_ON_LAUNCH_X, WINDOW_SIZE_ON_LAUNCH_Y));
		Window::CenterWindow();
		Window::SetTitle("ALZ Particles");
	}

	void Environment::Update()
	{
		timeEnd = clock();
		float deltaTime = (timeEnd - timeStart) / 1000.0f;
		timeStart = timeEnd;

		m_Camera.Update(deltaTime, m_ViewportWindow.RenderViewport);
		m_ExportCamera.Update(deltaTime);
		m_AppStatusWindow.Update(deltaTime);

		//go through all the objects (regular and not a range based loop because we want to use std::vector::erase())
		for (int i = 0; i < InspectorObjects.size(); i++) {

			if(m_Status == Status_PlayMode || m_Status == Status_ExportMode)
				InspectorObjects[i]->Update(deltaTime);

			if (InspectorObjects[i]->ToBeDeleted) 
			{
				//display status that we are deleting the object (for 2 seconds)
				m_AppStatusWindow.SetText("Deleted Object : \"" + InspectorObjects[i]->m_Name + '"' + " of Type : \"" +
										  InspectorObjectTypeToString(InspectorObjects[i]->Type) + '"', 2.0f);

				//delete the object
				InspectorObjects.erase(InspectorObjects.begin() + i);
			}
		}

		if (Window::WindowSizeChangedSinceLastFrame)
			m_RenderSurface.SetSize(Window::FramebufferSize);

		if (m_Status == Status_PlayMode || m_Status == Status_ExportMode)
			m_TimeSincePlayModeStarted += deltaTime;

		//this code will be changed and refactored, that is why it is strcutured badly
		if (m_Status == Status_ExportMode) {
			if (m_TimeSincePlayModeStarted > m_ExportCamera.ImageCaptureTime && m_ExportCamera.NeedToExport == false && m_ExportCamera.AlreadyExportedFrame == false)
				m_ExportCamera.StartExporting();
			else if (m_TimeSincePlayModeStarted > m_ExportCamera.ImageCaptureTime && m_ExportCamera.NeedToExport == false && m_ExportCamera.ExportedEverything == false)
				m_ExportCamera.StartExporting();

			if (m_ExportCamera.m_ExportMode == ExportCamera::SingleFrame && m_ExportCamera.AlreadyExportedFrame)
			{
				Stop();
				m_HideGUI = false;
				m_ExportCamera.AlreadyExportedFrame = false;
			}
			else if (m_ExportCamera.m_ExportMode == ExportCamera::MultipleFramesAsSeperateImages && m_ExportCamera.ExportedEverything == true) {
				Stop();
				m_HideGUI = false;
			}
		}
	}

	void Environment::Render()
	{
		Renderer::BeginScene(m_Camera);
		m_RenderSurface.SurfaceFrameBuffer->Bind();
		Renderer::ClearScreen();

		for (pEnvironmentObject& obj : InspectorObjects)
		{
			if (obj->Type == RadialLightType) {
				RadialLight* light = static_cast<RadialLight*>(obj.get());
				m_Background.SubmitLight(*light);
			}
			else if (obj->Type == SpotLightType) {
				SpotLight* light = static_cast<SpotLight*>(obj.get());
				m_Background.SubmitLight(*light);
			}
		}

		m_Background.Draw();

		for (pEnvironmentObject& obj : InspectorObjects)
		{
			if (obj->Type == SpriteType)
				obj->Draw();
		}

		if(m_Settings.ShowGrid && m_Status != Status_PlayMode)
			m_Grid.Draw();

		if(m_Status == Status_EditorMode)
			for (pEnvironmentObject& obj : InspectorObjects)
				if (obj->Selected) {
					//draw object position gizmo
					m_Gizmo.Draw(obj->GetPositionRef(), m_ViewportWindow.RenderViewport);

					//if particle system needs to edit a force target (a world point), use a gimzo for it
					if (obj->Type == EnvironmentObjectType::ParticleSystemType)
					{
						auto ps = static_cast<ParticleSystem*>(obj.get());
						if(ps->Customizer.m_ForceCustomizer.m_CurrentSelectedForceName != "")
							if (ps->Customizer.m_ForceCustomizer.m_Forces[ps->Customizer.m_ForceCustomizer.m_CurrentSelectedForceName].Type == Force::RelativeForce)
								m_Gizmo.Draw(ps->Customizer.m_ForceCustomizer.m_Forces[ps->Customizer.m_ForceCustomizer.m_CurrentSelectedForceName].RF_Target, m_ViewportWindow.RenderViewport);
					}
				}

		//Render world space gui here because we need camera information for that
		if (m_Status == Status_EditorMode) {
			for (pEnvironmentObject& obj : InspectorObjects)
			{
				if (obj->Selected)
					if (obj->Type == EnvironmentObjectType::ParticleSystemType)
					{
						ParticleSystem* ps = (ParticleSystem*)obj.get();
						if (ps->Customizer.Mode == SpawnMode::SpawnOnLine)
							ps->Customizer.m_Line.Draw();
						else if (ps->Customizer.Mode == SpawnMode::SpawnOnCircle || ps->Customizer.Mode == SpawnMode::SpawnInsideCircle && ps->Selected)
							ps->Customizer.m_CircleOutline.Draw();
					}
			}
		}

		if (m_Status == Status_EditorMode) {
			m_ExportCamera.DrawOutline();
			m_RenderSurface.RenderToScreen(m_ViewportWindow.RenderViewport);
			m_RenderSurface.SurfaceFrameBuffer->Unbind();
			return;
		}

		//stuff to only render in play mode and export mode
		for (pEnvironmentObject& obj : InspectorObjects) 
		{
			//because we already drawed all the sprites
			if (obj->Type == SpriteType)
				continue;
			obj->Draw();
		}

		if (m_Settings.BlurEnabled)
			GaussianBlur(m_RenderSurface, m_Settings.BlurRadius);

		//draw this after post processing because we do not want the line blured
		m_ExportCamera.DrawOutline();

		m_RenderSurface.RenderToScreen(m_ViewportWindow.RenderViewport);

		Renderer::EndScene();

		if (m_ExportCamera.NeedToExport) 
			m_ExportCamera.ExportFrame(m_Background, InspectorObjects,m_Settings.BlurEnabled ? m_Settings.BlurRadius : -1.0f );

		m_RenderSurface.SurfaceFrameBuffer->Unbind();
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

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		auto viewportDockID = ImGui::DockSpaceOverViewport(&viewport, ImGuiDockNodeFlags_PassthruCentralNode, 0);
		ImGui::PopStyleColor();


		m_AppStatusWindow.DisplayGUI(viewportDockID);
		
		AssetManager::DisplayGUI();
		DisplayEnvironmentControlsGUI();
		DisplayObjectInspecterGUI();
		m_Settings.DisplayGUI();
		m_ExportCamera.DisplayGUI();
		DisplayEnvironmentStatusGUI();
		m_Background.DisplayGUI();

		for (pEnvironmentObject& obj : InspectorObjects)
			obj->DisplayGUI();

		InputManager::DisplayGUI();
		m_ViewportWindow.DisplayGUI();

		ImGuiWrapper::Render();
	}

	void Environment::HandleInput()
	{
		InputManager::HandleInput();
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

				if (ImGui::Selectable((InspectorObjects[i]->m_Name.size() > 0) ? InspectorObjects[i]->m_Name.c_str() : "No Name", &InspectorObjects[i]->Selected)) 
				{
					//if this is selected. deselect all other particle systems
					for (auto& particle : InspectorObjects) {
						if (particle.get() != InspectorObjects[i].get())
							particle->Selected = false;
					}
				}

				if (ImGui::BeginDragDropSource())
				{
					ImGui::Text(("Moving: " + InspectorObjects[i]->m_Name).c_str());
					ImGui::SetDragDropPayload("re-order", &InspectorObjects[i]->Order, sizeof(int), ImGuiCond_Once);
					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload * payload = ImGui::AcceptDragDropPayload("re-order"))
					{
						size_t switchTarget1 = *(int*)payload->Data;
						size_t switchTarget2 = i;

						EnvironmentObjectInterface* temp = InspectorObjects[switchTarget1].release();
						InspectorObjects[switchTarget1] = std::unique_ptr<EnvironmentObjectInterface>(InspectorObjects[switchTarget2].release());
						InspectorObjects[switchTarget2] = std::unique_ptr<EnvironmentObjectInterface>(temp);

						RefreshObjectOrdering();
					}

					ImGui::EndDragDropTarget();
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

		if (ImGui::Button("Add Object"))
			m_AddObjectWindowOpen = true;

		ImGui::End();

		if (!m_AddObjectWindowOpen)
			return;

		//if we are adding a new object display a window for it's settings
		ImGui::Begin("Add Object", &m_AddObjectWindowOpen, ImGuiWindowFlags_NoDocking);

		ImGui::Text("NewObjectName");
		ImGui::SameLine();
		ImGui::InputText("##NewObjectName", &m_AddObjectWindowObjectName);

		ImGui::Text("NewObjectType");
		ImGui::SameLine();
		if (ImGui::BeginCombo("##NewObjectType", InspectorObjectTypeToString(m_AddObjectWindowObjectType).c_str()))
		{
			{
				bool selected = m_AddObjectWindowObjectType == ParticleSystemType;
				if (ImGui::Selectable(InspectorObjectTypeToString(ParticleSystemType).c_str(), &selected))
					m_AddObjectWindowObjectType = ParticleSystemType;
			}

			{
				bool selected = m_AddObjectWindowObjectType == SpriteType;
				if (ImGui::Selectable(InspectorObjectTypeToString(SpriteType).c_str(), &selected))
					m_AddObjectWindowObjectType = SpriteType;
			}

			{
				bool selected = m_AddObjectWindowObjectType == RadialLightType;
				if (ImGui::Selectable(InspectorObjectTypeToString(RadialLightType).c_str(), &selected))
					m_AddObjectWindowObjectType = RadialLightType;
			}

			{
				bool selected = m_AddObjectWindowObjectType == SpotLightType;
				if (ImGui::Selectable(InspectorObjectTypeToString(SpotLightType).c_str(), &selected))
					m_AddObjectWindowObjectType = SpotLightType;
			}

			ImGui::EndCombo();
		}

		if (ImGui::Button("Add Object"))
		{
			AddInspectorObject(m_AddObjectWindowObjectType, m_AddObjectWindowObjectName);

			//close the window after adding the object
			m_AddObjectWindowOpen = false;
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
		for (pEnvironmentObject& object : InspectorObjects)
		{
			//if object is a particle system
			if (object->Type == EnvironmentObjectType::ParticleSystemType) {
				//cast it to a particle system pointer
				ParticleSystem* ps = static_cast<ParticleSystem*>(object.get());

				//increment active particles by how many particles are active in this particle system
				activeParticleCount += ps->ActiveParticleCount;
			}
		}
		ImGui::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, std::to_string(activeParticleCount).c_str());

		ImGui::Separator();

		for (pEnvironmentObject& pso : InspectorObjects)
		{
			if (pso->Type == EnvironmentObjectType::ParticleSystemType) {

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

		{
			ImGui::SetNextWindowDockID(id, ImGuiCond_Always);

			ImGui::Begin("PlayMode Mode Status", nullptr, ImGuiWindowFlags_NoSavedSettings);

			//this is to control how many decimal points we want to display
			std::stringstream stream;
			//we want 3 decimal places
			stream << std::setprecision(3) << m_TimeSincePlayModeStarted;
			ImGui::Text("Time Since PlayMode Mode Started :");
			ImGui::SameLine();
			ImGui::TextColored({ 0.0f,0.8f,0.0f,1.0f }, stream.str().c_str());

			ImGui::End();
		}

		{
			ImGui::SetNextWindowDockID(id, ImGuiCond_Always);

			ImGui::Begin("Rendering Status", nullptr, ImGuiWindowFlags_NoSavedSettings);

			ImGui::Text("Draw Calls: ");
			ImGui::SameLine();
			ImGui::TextColored({ 0.0f,0.8f,0.0f,1.0f }, std::to_string(Renderer::NumberOfDrawCallsLastScene).c_str());

			ImGui::End();
		}
	}

	void Environment::DisplayEnvironmentControlsGUI()
	{
		//return if window is not open
		if (!m_EnvironmentControlsWindowOpen)
			return;

		ImGui::Begin("Controls", &m_EnvironmentControlsWindowOpen);
		if (m_Status == Status_ExportMode)
		{
			ImGui::End();
			return;
		}

		int width = (int)ImGui::GetWindowSize().x;
		ImGui::SetCursorPosX((float)width / 2 - 20);

		if (m_Status == Status_PlayMode || m_Status == Status_PauseMode) {
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
				m_ExportCamera.ExportedEverything = false;
			}
		}

		ImGui::SameLine();
		if (m_Status == Status_PlayMode) {
			if (ImGui::ImageButton((void*)(uintptr_t)m_PauseButtonTexture->GetRendererID(), ImVec2(30, 20), ImVec2(0, 0), ImVec2(1, 1), 1))
				Pause();
		}
		else if (m_Status == Status_PauseMode) {
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
					SaveEnvironment(*this, m_EnvironmentFolderPath + m_EnvironmentName + ".env");

				if (ImGui::MenuItem("Close Environment")) 
					ShouldDelete = true; //this means environment be closed when the time is right

				if (ImGui::MenuItem("Exit"))
					exit(0);

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
				ImGui::MenuItem("General Settings", nullptr, &m_Settings.GeneralSettingsWindowOpen);
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
					InputManager::ControlsWindowOpen = !InputManager::ControlsWindowOpen;
					
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void Environment::RefreshObjectOrdering()
	{
		for (size_t i = 0; i < InspectorObjects.size(); i++)
			InspectorObjects[i]->Order = i;
	}

	void Environment::PlayMode()
	{
		m_Status = Status_PlayMode;
		m_TimeSincePlayModeStarted = 0.0f;
	}

	void Environment::ExportMode()
	{
		m_Status = Status_ExportMode;
		m_TimeSincePlayModeStarted = 0.0f;
		m_ExportCamera.BeginExportScene();
	}

	void Environment::Stop()
	{
		m_Status = Status_EditorMode;

		for (pEnvironmentObject& obj : InspectorObjects) {
			if (obj->Type == EnvironmentObjectType::ParticleSystemType) {
				ParticleSystem* ps = static_cast<ParticleSystem*>(obj.get());
				ps->ClearParticles();
			}
		}
	}

	void Environment::Pause()
	{
		m_Status = Status_PauseMode;
	}

	void Environment::Resume()
	{
		m_Status = Status_PlayMode;
	}

	void Environment::RegisterEnvironmentInputKeys()
	{
		InputManager::RegisterKey(GLFW_KEY_F5, "PlayMode/Resume", [this]() {
			if (m_Status == Status_EditorMode)
				PlayMode();
			if (m_Status == Status_PauseMode)
				Resume();
		});

		InputManager::RegisterKey(GLFW_KEY_F1, "Hide Menus", [this]() { m_HideGUI = !m_HideGUI; });

		InputManager::RegisterKey(GLFW_KEY_SPACE, "Clear All Particles", [this]()
		{
			for (pEnvironmentObject& obj : InspectorObjects) {
				if (obj->Type == EnvironmentObjectType::ParticleSystemType) {
					ParticleSystem* ps = static_cast<ParticleSystem*>(obj.get());
					ps->ClearParticles();
				}
			}
		});

		//so we dont have to repeat those 2 lines
#define DISPLAY_CAMERA_POSITION_IN_APP_STATUS_WINDOW  std::string posStr = "(" + std::to_string(m_Camera.Position.x) + ", " + std::to_string(m_Camera.Position.y) + ")";\
													  m_AppStatusWindow.SetText(("Moving Camera to Coordinates :" + posStr).c_str(), 0.1f)

		//map WASD keys to move the camera in the environment

		InputManager::RegisterKey(GLFW_KEY_W, "Move Camera Up", [this]() {
			//actually move the camera
			m_Camera.SetPosition(m_Camera.Position + glm::vec2(0.0f, -10.0f));
			DISPLAY_CAMERA_POSITION_IN_APP_STATUS_WINDOW; },
			//set mode as repeat because we want the camera to move smoothly
			GLFW_REPEAT);
		//the rest are the same with only a diffrent move direction, that is why they arent commented

		InputManager::RegisterKey(GLFW_KEY_S, "Move Camera Down", [this]() {
			m_Camera.SetPosition(m_Camera.Position + glm::vec2(0.0f, 10.0f));
			DISPLAY_CAMERA_POSITION_IN_APP_STATUS_WINDOW;
			},
			GLFW_REPEAT);

		InputManager::RegisterKey(GLFW_KEY_D, "Move Camera To The Right", [this]() {
			m_Camera.SetPosition(m_Camera.Position + glm::vec2(-10.0f, 0.0f));
			DISPLAY_CAMERA_POSITION_IN_APP_STATUS_WINDOW;
			},
			GLFW_REPEAT);

		InputManager::RegisterKey(GLFW_KEY_A, "Move Camera To The Left", [this]() {
			m_Camera.SetPosition(m_Camera.Position + glm::vec2(10.0f, 0.0f));
			DISPLAY_CAMERA_POSITION_IN_APP_STATUS_WINDOW;
			},
			GLFW_REPEAT);

#undef DISPLAY_CAMERA_POSITION_IN_APP_STATUS_WINDOW

		//delete keyboard shortcut
		InputManager::RegisterKey(GLFW_KEY_DELETE, "Delete Object", [this]() {

			for (int i = 0; i < InspectorObjects.size(); i++)
			{
				if (InspectorObjects[i]->Selected) {
					InspectorObjects[i]->ToBeDeleted = true;
					break;
				}
			}
		});

	}

	void Environment::AddInspectorObject(EnvironmentObjectType type, const std::string& name)
	{
		//interface for the object to be added
		pEnvironmentObject obj;

		//create the object depending on it's type
		switch (type)
		{
		case ParticleSystemType: 
			{
			auto ps = std::make_unique<ParticleSystem>();
			obj.reset(((EnvironmentObjectInterface*)(ps.release())));
			}
			break;

		case RadialLightType:
			{
			auto light = std::make_unique<RadialLight>();
			obj.reset(((EnvironmentObjectInterface*)(light.release())));
			}
			break;

		case SpotLightType:
			{
			auto light = std::make_unique<SpotLight>();
			obj.reset(((EnvironmentObjectInterface*)(light.release())));
			}
			break;

		case SpriteType:
			{
			auto sprite = std::make_unique<Sprite>();
			obj.reset(((EnvironmentObjectInterface*)(sprite.release())));
			}
			break;

		default:
			//we should never reach here
			assert(false);
			return;
		}
		
		obj->m_Name = name;

		//display text that we created the object (for 2 seconds)
		m_AppStatusWindow.SetText("Created Object : \"" + obj->m_Name + '"' + " of Type : \"" +InspectorObjectTypeToString(obj->Type) + '"', 2.0f);

		//add the object to the list of the environment objects
		InspectorObjects.push_back(std::move(obj));

		RefreshObjectOrdering();
	}

	void Environment::Duplicate(EnvironmentObjectInterface& obj)
	{
		//if this object is a particle system
		if (obj.Type == EnvironmentObjectType::ParticleSystemType) 
		{
			//make a new particle system
			InspectorObjects.push_back(std::make_unique<ParticleSystem>(*static_cast<ParticleSystem*>(&obj)));

			//add a -copy to the name of the new particle system to indicate that it was copied
			InspectorObjects[InspectorObjects.size() - 1]->m_Name += "-copy";
		}

		//if this object is a radial light
		else if (obj.Type == EnvironmentObjectType::RadialLightType) 
		{
			//make a new radial light
			InspectorObjects.push_back(std::make_unique<RadialLight>(*static_cast<RadialLight*>(&obj)));

			//add a -copy to the name of the new light to indicate that it was copied
			InspectorObjects[InspectorObjects.size() - 1]->m_Name += "-copy";
		}

		//if this object is a spot light
		else if (obj.Type == EnvironmentObjectType::SpotLightType)
		{
			//make a new radial light
			InspectorObjects.push_back(std::make_unique<SpotLight>(*static_cast<SpotLight*>(&obj)));

			//add a -copy to the name of the new light to indicate that it was copied
			InspectorObjects[InspectorObjects.size() - 1]->m_Name += "-copy";
		}
	}

	void Environment::FocusCameraOnObject(EnvironmentObjectInterface& object)
	{
		EnvironmentObjectType type = object.Type;

		if (type == ALZ::ParticleSystemType) {
			ParticleSystem& ps = *static_cast<ParticleSystem*>(&object);

			switch (ps.Customizer.Mode)
			{
			case SpawnMode::SpawnOnPoint:

				m_Camera.SetPosition(glm::vec3(ps.Customizer.m_SpawnPosition.x * -GlobalScaleFactor, ps.Customizer.m_SpawnPosition.y * -GlobalScaleFactor, 0.0f)
					+ glm::vec3(m_ViewportWindow.RenderViewport.width / 2, m_ViewportWindow.RenderViewport.height / 2, 0.0f));
				break;

			case SpawnMode::SpawnOnCircle:
				m_Camera.SetPosition(glm::vec3(ps.Customizer.m_CircleOutline.Position.x, ps.Customizer.m_CircleOutline.Position.y, 0.0f)
								   + glm::vec3(m_ViewportWindow.RenderViewport.width / 2, m_ViewportWindow.RenderViewport.height / 2, 0.0f));
				break;

			case SpawnMode::SpawnInsideCircle:
				m_Camera.SetPosition(glm::vec3(ps.Customizer.m_CircleOutline.Position.x, ps.Customizer.m_CircleOutline.Position.y, 0.0f)
					+ glm::vec3(m_ViewportWindow.RenderViewport.width / 2, m_ViewportWindow.RenderViewport.height / 2, 0.0f));
				break;

			case SpawnMode::SpawnOnLine:

				m_Camera.SetPosition(glm::vec3(ps.Customizer.m_LinePosition.x * -GlobalScaleFactor, ps.Customizer.m_LinePosition.y * -GlobalScaleFactor, 0.0f)
								   + glm::vec3(m_ViewportWindow.RenderViewport.width / 2, m_ViewportWindow.RenderViewport.height / 2, 0.0f));
				break;
			}
		}
		else {
			m_Camera.SetPosition(glm::vec3(object.GetPositionRef().x, object.GetPositionRef().y, 0.0f) * -GlobalScaleFactor
							   + glm::vec3(m_ViewportWindow.RenderViewport.width / 2, m_ViewportWindow.RenderViewport.height / 2, 0.0f));
		}
	}

	void Environment::UpdateTitle()
	{
		RendererType currentRendererType = Renderer::m_CurrentActiveAPI->GetType();

		switch (currentRendererType)
		{
		case RendererType::OpenGL:
			Window::SetTitle("ALZ Particles - OpenGL (3.3) - " + m_EnvironmentName);
			break;
		}
	}
}