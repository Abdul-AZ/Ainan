#include <pch.h>
#include "Environment.h"

namespace Ainan {

	Environment::Environment(const std::string& environmentFolderPath, const std::string& environmentName) :
		m_EnvironmentFolderPath(environmentFolderPath),
		m_EnvironmentName(environmentName)
	{
		std::memset(m_DeltaTimeHistory.data(), 0, m_DeltaTimeHistory.size() * sizeof(float));
		m_PlayButtonTexture = Renderer::CreateTexture();
		m_PauseButtonTexture = Renderer::CreateTexture();
		m_ResumeButtonTexture = Renderer::CreateTexture();
		m_StopButtonTexture = Renderer::CreateTexture();

		m_PlayButtonTexture->SetImage(Image::LoadFromFile("res/PlayButton.png", 3));
		m_PauseButtonTexture->SetImage(Image::LoadFromFile("res/PauseButton.png", 3));
		m_ResumeButtonTexture->SetImage(Image::LoadFromFile("res/ResumeButton.png", 3));
		m_StopButtonTexture->SetImage(Image::LoadFromFile("res/StopButton.png", 3));

		RegisterEnvironmentInputKeys();

		AssetManager::Init(environmentFolderPath);
		InputManager::Init();

		UpdateTitle();

		InputManager::m_ScrollFunctions.push_back([this](int scroll) {
				//change zoom factor
				m_Camera.ZoomFactor -= scroll * 25;
				//clamp zoom factor
				m_Camera.ZoomFactor = std::clamp(m_Camera.ZoomFactor, c_CameraZoomFactorMin, c_CameraZoomFactorMax);

				//display the new zoom factor in the bottom left of the screen
				std::stringstream stream;
				stream << std::setprecision(0);
				stream << "Zoom ";
				stream << (int)(c_CameraZoomFactorDefault * 100.0f / m_Camera.ZoomFactor);
				stream << "%%";

				m_AppStatusWindow.SetText(stream.str());
			});
	}

	Environment::~Environment()
	{
		InputManager::ClearKeys();
		InputManager::Terminate();
		AssetManager::Terminate();
		InspectorObjects.clear();
		Window::Restore();
		Window::SetSize(glm::ivec2(WINDOW_SIZE_ON_LAUNCH_X, WINDOW_SIZE_ON_LAUNCH_Y));
		Window::CenterWindow();
		Window::SetTitle("Ainan");
	}

	void Environment::StartFrame()
	{
		timeStart = clock();
	}

	void Environment::EndFrame()
	{
		timeEnd = clock();
		m_DeltaTime = (timeEnd - timeStart) / 1000.0f;
	}

	void Environment::Update()
	{
		//if time passed is less than a frame time, we use the time of a single frame
		//because we are not going to start the next frame until the time of a single frame finishes
		float realDeltaTime = m_DeltaTime > 0.01666f ? m_DeltaTime : 0.01666f;

		m_Camera.Update(realDeltaTime, m_ViewportWindow.RenderViewport);
		m_AppStatusWindow.Update(realDeltaTime);

		//go through all the objects (regular and not a range based loop because we want to use std::vector::erase())
		for (int i = 0; i < InspectorObjects.size(); i++) {

			if(m_Status == Status_PlayMode || m_Status == Status_ExportMode)
				InspectorObjects[i]->Update(realDeltaTime);

			if (InspectorObjects[i]->ToBeDeleted) 
			{
				//display status that we are deleting the object (for 2 seconds)
				m_AppStatusWindow.SetText("Deleted Object : \"" + InspectorObjects[i]->m_Name + '"' + " of Type : \"" +
										  EnvironmentObjectTypeToString(InspectorObjects[i]->Type) + '"', 2.0f);

				//delete the object
				InspectorObjects.erase(InspectorObjects.begin() + i);
			}
		}

		if (Window::WindowSizeChangedSinceLastFrame)
			m_RenderSurface.SetSize(Window::FramebufferSize);

		//this stuff is used for the profiler
		if (m_Status == Status_PlayMode || m_Status == Status_ExportMode)
		{
			m_TimeSincePlayModeStarted += realDeltaTime;

			//save delta time for the profiler

			//move everything back
			std::memmove(m_DeltaTimeHistory.data(), m_DeltaTimeHistory.data() + 1, (m_DeltaTimeHistory.size() - 1) * sizeof(float));
			//register the new time
			m_DeltaTimeHistory[m_DeltaTimeHistory.size() - 1] = m_DeltaTime;
		}

		if (m_Status == Status_ExportMode && m_ExportedFrame)
			Stop();
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

		if (m_Status == Status_ExportMode && m_ExportCamera.ImageCaptureTime < m_TimeSincePlayModeStarted) 
		{
			m_ExportCamera.ExportFrame(m_Background, InspectorObjects, m_Settings.BlurEnabled ? m_Settings.BlurRadius : -1.0f);
			m_ExportedFrame = true;
		}

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
		
		AssetManager::DisplayGUI();
		DisplayEnvironmentControlsGUI();
		DisplayObjectInspecterGUI();
		DisplayProfilerGUI();
		m_AppStatusWindow.DisplayGUI(viewportDockID);
		m_Settings.DisplayGUI();
		m_ExportCamera.DisplayGUI();
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

		if (ImGui::Button("Add Object"))
			m_AddObjectWindowOpen = true;

		ImGui::Spacing();

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
		if (ImGui::BeginCombo("##NewObjectType", EnvironmentObjectTypeToString(m_AddObjectWindowObjectType).c_str()))
		{
			{
				bool selected = m_AddObjectWindowObjectType == ParticleSystemType;
				if (ImGui::Selectable(EnvironmentObjectTypeToString(ParticleSystemType).c_str(), &selected))
					m_AddObjectWindowObjectType = ParticleSystemType;
			}

			{
				bool selected = m_AddObjectWindowObjectType == SpriteType;
				if (ImGui::Selectable(EnvironmentObjectTypeToString(SpriteType).c_str(), &selected))
					m_AddObjectWindowObjectType = SpriteType;
			}

			{
				bool selected = m_AddObjectWindowObjectType == RadialLightType;
				if (ImGui::Selectable(EnvironmentObjectTypeToString(RadialLightType).c_str(), &selected))
					m_AddObjectWindowObjectType = RadialLightType;
			}

			{
				bool selected = m_AddObjectWindowObjectType == SpotLightType;
				if (ImGui::Selectable(EnvironmentObjectTypeToString(SpotLightType).c_str(), &selected))
					m_AddObjectWindowObjectType = SpotLightType;
			}

			ImGui::EndCombo();
		}

		if (ImGui::Button("Add Object"))
		{
			AddEnvironmentObject(m_AddObjectWindowObjectType, m_AddObjectWindowObjectName);

			//close the window after adding the object
			m_AddObjectWindowOpen = false;
		}

		ImGui::End();
	}

	void Environment::DisplayProfilerGUI()
	{
		if (!m_ProfilerWindowOpen)
			return;

		ImGui::Begin("Profiler");

		ImVec4 activeColor = { 0.6f,0.6f,0.6f,1.0f };
		ImVec4 inactiveColor = { 0.2f,0.2f,0.2f,1.0f };

		{
			if (m_ActiveProfiler == Profiler::RenderingProfiler) 
				ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
			else 
				ImGui::PushStyleColor(ImGuiCol_Button, inactiveColor);

			if (ImGui::Button("Rendering"))
				m_ActiveProfiler = Profiler::RenderingProfiler;

			ImGui::PopStyleColor();
		}

		ImGui::SameLine();

		{
			if (m_ActiveProfiler == Profiler::ParticleProfiler)
				ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
			else
				ImGui::PushStyleColor(ImGuiCol_Button, inactiveColor);

			if (ImGui::Button("Particles"))
				m_ActiveProfiler = Profiler::ParticleProfiler;

			ImGui::PopStyleColor();
		}

		ImGui::SameLine();

		{
			if (m_ActiveProfiler == Profiler::PlaymodeProfiler)
				ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
			else
				ImGui::PushStyleColor(ImGuiCol_Button, inactiveColor);

			if (ImGui::Button("Environment"))
				m_ActiveProfiler = Profiler::PlaymodeProfiler;

			ImGui::PopStyleColor();
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);

		switch (m_ActiveProfiler)
		{
		case Profiler::RenderingProfiler:
			{
				ImGui::Text("Draw Calls: ");
				ImGui::SameLine();
				ImGui::TextColored({ 0.0f,0.8f,0.0f,1.0f }, std::to_string(Renderer::NumberOfDrawCallsLastScene).c_str());
				ImGui::SameLine();


				ImGui::Text("        FPS: ");
				ImGui::SameLine();
				ImGui::TextColored({ 0.0f,0.8f,0.0f,1.0f }, std::to_string(static_cast<int>( 1.0f / m_DeltaTime)).c_str());

				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					ImGui::SetTooltip("NOTE: Frame rates do not exceed 60,\nthis is theoretical FPS given the time per frame");
					ImGui::EndTooltip();
				}

				ImGui::PlotLines("Frame Time(s)", m_DeltaTimeHistory.data(), m_DeltaTimeHistory.size(),
								 0, 0, 0.0f, 0.025f, ImVec2(0, 75));

				ImGui::Text("Textures: ");
				ImGui::SameLine();
				ImGui::Text(std::to_string(Renderer::m_ReservedTextures.size()).c_str());

				ImGui::SameLine();
				ImGui::Text("   VBO(s): ");
				ImGui::SameLine();
				ImGui::Text(std::to_string(Renderer::m_ReservedVertexBuffers.size()).c_str());

				ImGui::SameLine();
				ImGui::Text("   EBO(s): ");
				ImGui::SameLine();
				ImGui::Text(std::to_string(Renderer::m_ReservedIndexBuffers.size()).c_str());
			}
			break;

		case Profiler::ParticleProfiler:
			{
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

			}
			break;

		case Profiler::PlaymodeProfiler:
			{
				//this is to control how many decimal points we want to display
				std::stringstream stream;
				//we want 3 decimal places
				stream << std::setprecision(3) << m_TimeSincePlayModeStarted;
				ImGui::Text("Time Since PlayMode Mode Started :");
				ImGui::SameLine();
				ImGui::TextColored({ 0.0f,0.8f,0.0f,1.0f }, stream.str().c_str());
			}
			break;
		}

		ImGui::End();
	}

	void Environment::DisplayEnvironmentControlsGUI()
	{
		//return if window is not open
		if (!m_EnvironmentControlsWindowOpen)
			return;
		
		ImGui::Begin("Controls", &m_EnvironmentControlsWindowOpen, ImGuiWindowFlags_NoScrollbar);
		if (m_Status == Status_ExportMode)
		{
			ImGui::End();
			return;
		}

		int width = (int)ImGui::GetWindowSize().x;
		ImGui::SetCursorPosX((float)width / 2 - 20);

		if (m_Status == Status_PlayMode || m_Status == Status_PauseMode) {
			if (ImGui::ImageButton((void*)(uintptr_t)m_StopButtonTexture->GetRendererID(), ImVec2(30, 20), ImVec2(0, 0), ImVec2(1, 1), 1))
				Stop();
		}
		else 
		{
			if (ImGui::ImageButton((void*)(uintptr_t)m_PlayButtonTexture->GetRendererID(), ImVec2(30, 20), ImVec2(0, 0), ImVec2(1, 1), 1))
				PlayMode();
			
			if (ImGui::Button("Export")) 
				ExportMode();
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

				if (ImGui::MenuItem("Clear Particle Systems"))
					InspectorObjects.clear();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Window")) {

				ImGui::MenuItem("Environment Controls", nullptr, &m_EnvironmentControlsWindowOpen);
				ImGui::MenuItem("Object Inspector", nullptr, &m_ObjectInspectorWindowOpen);
				ImGui::MenuItem("General Settings", nullptr, &m_Settings.GeneralSettingsWindowOpen);
				ImGui::MenuItem("Profiler", nullptr, &m_ProfilerWindowOpen);
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
		//reset profiler
		m_TimeSincePlayModeStarted = 0.0f;
		std::memset(m_DeltaTimeHistory.data(), 0, m_DeltaTimeHistory.size() * sizeof(float));
	}

	void Environment::ExportMode()
	{
		m_Status = Status_ExportMode;
		m_TimeSincePlayModeStarted = 0.0f;
		m_ExportedFrame = false;
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

		InputManager::RegisterMouseKey(GLFW_MOUSE_BUTTON_MIDDLE, "Change Camera Zoom to Default", [this]() 
			{
				m_Camera.ZoomFactor = c_CameraZoomFactorDefault; 
				//display the new zoom factor in the bottom left of the screen
				std::stringstream stream;
				stream << std::setprecision(0);
				stream << "Zoom ";
				stream << (int)(c_CameraZoomFactorDefault * 100.0f / m_Camera.ZoomFactor);
				stream << "%%";

				m_AppStatusWindow.SetText(stream.str());
			});

		//shortcut cut to use in all the mapped buttons
		//it displays the camera position in the status window(the blue coloured stripe at the bottom)
		auto displayCameraPosFunc = [this]()
		{
			std::stringstream messageString;

			messageString << std::fixed << std::setprecision(2);
			messageString << "Moving Camera to Coordinates :";
			messageString << "(" << -m_Camera.Position.x / c_GlobalScaleFactor;
			messageString << ", " << -m_Camera.Position.y / c_GlobalScaleFactor << ")";

			m_AppStatusWindow.SetText(messageString.str());
		};

		//map WASD keys to move the camera in the environment
		InputManager::RegisterKey(GLFW_KEY_W, "Move Camera Up", [this, displayCameraPosFunc]() 
			{
			//move the camera's position
			m_Camera.SetPosition(m_Camera.Position + glm::vec2(0.0f, -m_Camera.ZoomFactor / 100.0f));
			displayCameraPosFunc();
			},
			//set mode as repeat because we want the camera to move smoothly
			GLFW_REPEAT);
		//the rest are the same with only a diffrent move direction, that is why they arent commented

		InputManager::RegisterKey(GLFW_KEY_S, "Move Camera Down", [this, displayCameraPosFunc]() {
			m_Camera.SetPosition(m_Camera.Position + glm::vec2(0.0f, m_Camera.ZoomFactor / 100.0f));
			displayCameraPosFunc();
			},
			GLFW_REPEAT);

		InputManager::RegisterKey(GLFW_KEY_D, "Move Camera To The Right", [this, displayCameraPosFunc]() {
			m_Camera.SetPosition(m_Camera.Position + glm::vec2(-m_Camera.ZoomFactor / 100.0f, 0.0f));
			displayCameraPosFunc();
			},
			GLFW_REPEAT);

		InputManager::RegisterKey(GLFW_KEY_A, "Move Camera To The Left", [this, displayCameraPosFunc]() {
			m_Camera.SetPosition(m_Camera.Position + glm::vec2(m_Camera.ZoomFactor / 100.0f, 0.0f));
			displayCameraPosFunc();
			},
			GLFW_REPEAT);

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

	void Environment::AddEnvironmentObject(EnvironmentObjectType type, const std::string& name)
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
		m_AppStatusWindow.SetText("Created Object : \"" + obj->m_Name + '"' + " of Type : \"" +EnvironmentObjectTypeToString(obj->Type) + '"', 2.0f);

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

		//if this object is a Sprite
		else if (obj.Type == EnvironmentObjectType::SpriteType)
		{
			//make a new radial light
			InspectorObjects.push_back(std::make_unique<Sprite>(*static_cast<Sprite*>(&obj)));

			//add a -copy to the name of the new light to indicate that it was copied
			InspectorObjects[InspectorObjects.size() - 1]->m_Name += "-copy";
		}
	}

	void Environment::FocusCameraOnObject(EnvironmentObjectInterface& object)
	{
		EnvironmentObjectType type = object.Type;

		if (type == Ainan::ParticleSystemType) {
			ParticleSystem& ps = *static_cast<ParticleSystem*>(&object);

			switch (ps.Customizer.Mode)
			{
			case SpawnMode::SpawnOnPoint:

				m_Camera.SetPosition(glm::vec3(ps.Customizer.m_SpawnPosition.x * -c_GlobalScaleFactor, ps.Customizer.m_SpawnPosition.y * -c_GlobalScaleFactor, 0.0f)
					+ glm::vec3(m_ViewportWindow.RenderViewport.Width / 2, m_ViewportWindow.RenderViewport.Height / 2, 0.0f));
				break;

			case SpawnMode::SpawnOnCircle:
				m_Camera.SetPosition(glm::vec3(ps.Customizer.m_CircleOutline.Position.x, ps.Customizer.m_CircleOutline.Position.y, 0.0f)
								   + glm::vec3(m_ViewportWindow.RenderViewport.Width / 2, m_ViewportWindow.RenderViewport.Height / 2, 0.0f));
				break;

			case SpawnMode::SpawnInsideCircle:
				m_Camera.SetPosition(glm::vec3(ps.Customizer.m_CircleOutline.Position.x, ps.Customizer.m_CircleOutline.Position.y, 0.0f)
					+ glm::vec3(m_ViewportWindow.RenderViewport.Width / 2, m_ViewportWindow.RenderViewport.Height / 2, 0.0f));
				break;

			case SpawnMode::SpawnOnLine:

				m_Camera.SetPosition(glm::vec3(ps.Customizer.m_LinePosition.x * -c_GlobalScaleFactor, ps.Customizer.m_LinePosition.y * -c_GlobalScaleFactor, 0.0f)
								   + glm::vec3(m_ViewportWindow.RenderViewport.Width / 2, m_ViewportWindow.RenderViewport.Height / 2, 0.0f));
				break;
			}
		}
		else {
			m_Camera.SetPosition(glm::vec3(object.GetPositionRef().x, object.GetPositionRef().y, 0.0f) * -c_GlobalScaleFactor
							   + glm::vec3(m_ViewportWindow.RenderViewport.Width / 2, m_ViewportWindow.RenderViewport.Height / 2, 0.0f));
		}
	}

	void Environment::UpdateTitle()
	{
		RendererType currentRendererType = Renderer::m_CurrentActiveAPI->GetType();

		switch (currentRendererType)
		{
		case RendererType::OpenGL:
			Window::SetTitle("Ainan - OpenGL (3.3) - " + m_EnvironmentName);
			break;
		}
	}
}