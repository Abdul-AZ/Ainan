#include <pch.h>

#include "Editor.h"

//TEMPORARY
#include "glad/glad.h"

namespace Ainan 
{
	Editor::Editor():
		m_LoadEnvironmentBrowser(STARTING_BROWSER_DIRECTORY, "Load Environment")
	{
		m_LoadEnvironmentBrowser.Filter.push_back(".env");
		m_LoadEnvironmentBrowser.OnCloseWindow = []() {
			Window::SetSize({ WINDOW_SIZE_FACTOR_ON_LAUNCH, WINDOW_SIZE_FACTOR_ON_LAUNCH * 9 / 16 });
			Window::CenterWindow();
		};

		m_PlayButtonTexture = Renderer::CreateTexture();
		m_PauseButtonTexture = Renderer::CreateTexture();
		m_StopButtonTexture = Renderer::CreateTexture();

		m_PlayButtonTexture->SetImage(Image::LoadFromFile("res/PlayButton.png", 4));
		m_PauseButtonTexture->SetImage(Image::LoadFromFile("res/PauseButton.png", 4));
		m_StopButtonTexture->SetImage(Image::LoadFromFile("res/StopButton.png", 4));
	}

	Editor::~Editor()
	{
		delete m_Env;
	}

	void Editor::Update()
	{
		switch (m_State)
		{
		case State_EditorMode:
			Update_EditorMode();
			break;

		case State_PlayMode:
			Update_PlayMode();
			break;

		case State_PauseMode:
			Update_PauseMode();
			break;

		case State_ExportMode:
			Update_ExportMode();
			break;
		}
	}

	void Editor::Draw()
	{
		switch (m_State)
		{
		case State_NoEnvLoaded:
			Draw_NoEnvLoaded();
			break;
		
		case State_CreateEnv:
			Draw_CreateEnv();
			break;

		case State_EditorMode:
			Draw_EditorMode();
			break;

		case State_PlayMode:
			Draw_PlayMode();
			break;

		case State_PauseMode:
			Draw_PauseMode();
			break;

		case State_ExportMode:
			Draw_ExportMode();
			break;
		}
	}

	void Editor::Update_EditorMode()
	{
		//if time passed is less than a frame time, we use the time of a single frame
		//because we are not going to start the next frame until the time of a single frame finishes
		float realDeltaTime = m_DeltaTime > 0.01666f ? m_DeltaTime : 0.01666f;

		m_Camera.Update(realDeltaTime, m_ViewportWindow.RenderViewport);
		m_AppStatusWindow.Update(realDeltaTime);

		//go through all the objects (regular and not a range based loop because we want to use std::vector::erase())
		for (int i = 0; i < m_Env->Objects.size(); i++) 
		{
			if (m_Env->Objects[i]->ToBeDeleted)
			{
				//display status that we are deleting the object (for 2 seconds)
				m_AppStatusWindow.SetText("Deleted Object : \"" + m_Env->Objects[i]->m_Name + '"' + " of Type : \"" +
					EnvironmentObjectTypeToString(m_Env->Objects[i]->Type) + '"', 2.0f);

				//delete the object
				m_Env->Objects.erase(m_Env->Objects.begin() + i);
			}
		}

		if (Window::WindowSizeChangedSinceLastFrame)
			m_RenderSurface.SetSize(Window::FramebufferSize);

		InputManager::HandleInput();
	}

	void Editor::Update_PlayMode()
	{
		//if time passed is less than a frame time, we use the time of a single frame
		//because we are not going to start the next frame until the time of a single frame finishes
		float realDeltaTime = m_DeltaTime > 0.01666f ? m_DeltaTime : 0.01666f;

		m_Camera.Update(realDeltaTime, m_ViewportWindow.RenderViewport);
		m_AppStatusWindow.Update(realDeltaTime);

		//go through all the objects (regular and not a range based loop because we want to use std::vector::erase())
		for (int i = 0; i < m_Env->Objects.size(); i++) 
		{
			m_Env->Objects[i]->Update(realDeltaTime);

			if (m_Env->Objects[i]->ToBeDeleted)
			{
				//display status that we are deleting the object (for 2 seconds)
				m_AppStatusWindow.SetText("Deleted Object : \"" + m_Env->Objects[i]->m_Name + '"' + " of Type : \"" +
					EnvironmentObjectTypeToString(m_Env->Objects[i]->Type) + '"', 2.0f);

				//delete the object
				m_Env->Objects.erase(m_Env->Objects.begin() + i);
			}
		}

		if (Window::WindowSizeChangedSinceLastFrame)
			m_RenderSurface.SetSize(Window::FramebufferSize);

		//this stuff is used for the profiler
		m_TimeSincePlayModeStarted += realDeltaTime;

		//save delta time for the profiler

		//move everything back
		std::memmove(m_DeltaTimeHistory.data(), m_DeltaTimeHistory.data() + 1, (m_DeltaTimeHistory.size() - 1) * sizeof(float));
		//register the new time
		m_DeltaTimeHistory[m_DeltaTimeHistory.size() - 1] = m_DeltaTime;

		if (m_State == State_ExportMode && m_ExportedFrame)
			Stop();
		InputManager::HandleInput();
	}

	void Editor::Update_PauseMode()
	{
		//if time passed is less than a frame time, we use the time of a single frame
		//because we are not going to start the next frame until the time of a single frame finishes
		float realDeltaTime = m_DeltaTime > 0.01666f ? m_DeltaTime : 0.01666f;

		m_Camera.Update(realDeltaTime, m_ViewportWindow.RenderViewport);
		m_AppStatusWindow.Update(realDeltaTime);

		//go through all the objects (regular and not a range based loop because we want to use std::vector::erase())
		for (int i = 0; i < m_Env->Objects.size(); i++) 
		{
			if (m_Env->Objects[i]->ToBeDeleted)
			{
				//display status that we are deleting the object (for 2 seconds)
				m_AppStatusWindow.SetText("Deleted Object : \"" + m_Env->Objects[i]->m_Name + '"' + " of Type : \"" +
					EnvironmentObjectTypeToString(m_Env->Objects[i]->Type) + '"', 2.0f);

				//delete the object
				m_Env->Objects.erase(m_Env->Objects.begin() + i);
			}
		}

		if (Window::WindowSizeChangedSinceLastFrame)
			m_RenderSurface.SetSize(Window::FramebufferSize);

		InputManager::HandleInput();
	}

	void Editor::Update_ExportMode()
	{
		//if time passed is less than a frame time, we use the time of a single frame
		//because we are not going to start the next frame until the time of a single frame finishes
		float realDeltaTime = m_DeltaTime > 0.01666f ? m_DeltaTime : 0.01666f;

		m_Camera.Update(realDeltaTime, m_ViewportWindow.RenderViewport);
		m_AppStatusWindow.Update(realDeltaTime);

		//update all objects
		for (auto& obj : m_Env->Objects) 
			obj->Update(realDeltaTime);

		if (Window::WindowSizeChangedSinceLastFrame)
			m_RenderSurface.SetSize(Window::FramebufferSize);

		//this stuff is used for the profiler
		m_TimeSincePlayModeStarted += realDeltaTime;

		//save delta time for the profiler

		//move everything back
		std::memmove(m_DeltaTimeHistory.data(), m_DeltaTimeHistory.data() + 1, (m_DeltaTimeHistory.size() - 1) * sizeof(float));
		//register the new time
		m_DeltaTimeHistory[m_DeltaTimeHistory.size() - 1] = m_DeltaTime;

		if (m_ExportedFrame)
			Stop();

		InputManager::HandleInput();
	}

	void Editor::Draw_NoEnvLoaded()
	{
		if (m_LoadEnvironmentBrowser.OnCloseWindow == nullptr)
			m_LoadEnvironmentBrowser.OnCloseWindow = []() {
			Window::SetSize({ WINDOW_SIZE_FACTOR_ON_LAUNCH, WINDOW_SIZE_FACTOR_ON_LAUNCH * 9 / 16 });
			Window::CenterWindow();
		};

		ImGuiWrapper::NewFrame();

		ImGui::SetNextWindowPos({ 0,0 });
		ImGui::SetNextWindowSize({ (float)Window::FramebufferSize.x , Window::FramebufferSize.y });

		ImGui::SetNextWindowDockID(ImGui::DockSpaceOverViewport(), ImGuiCond_::ImGuiCond_Always);
		ImGui::Begin("Start Menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

		ImGui::SetWindowSize(ImVec2(Window::Size.x, Window::FramebufferSize.y));

		ImGui::SetCursorPosX((float)Window::FramebufferSize.x / 2 - (float)START_MENU_BUTTON_WIDTH / 2);

		if (ImGui::Button("Create New Environment", ImVec2(START_MENU_BUTTON_WIDTH, START_MENU_BUTTON_HEIGHT)))
		{
			m_State = State_CreateEnv;
			Window::SetSize({ WINDOW_SIZE_ON_CREATE_ENVIRONMENT_X, WINDOW_SIZE_ON_CREATE_ENVIRONMENT_Y });
			Window::CenterWindow();
		}

		ImGui::SetCursorPosX((float)Window::FramebufferSize.x / 2 - (float)START_MENU_BUTTON_WIDTH / 2);

		if (ImGui::Button("Load Environment", ImVec2(START_MENU_BUTTON_WIDTH, START_MENU_BUTTON_HEIGHT)))
		{
			m_LoadEnvironmentBrowser.OpenWindow();
		}

		ImGui::SetCursorPosX((float)Window::FramebufferSize.x / 2 - (float)START_MENU_BUTTON_WIDTH / 2);

		if (ImGui::Button("Exit App", ImVec2(START_MENU_BUTTON_WIDTH, START_MENU_BUTTON_HEIGHT)))
		{
			Window::SetShouldClose();
		}

		ImGui::End();

		m_LoadEnvironmentBrowser.DisplayGUI([this](const std::filesystem::path path)
			{
				//check if file is selected
				if (path.extension().u8string() == ".env") {
					assert(CheckEnvironmentFile(path.u8string()) == "");

					//remove minimizing event on file browser window close
					m_LoadEnvironmentBrowser.OnCloseWindow = nullptr;

					m_Env = LoadEnvironment(path.u8string());
					m_State = State_EditorMode;
					m_EnvironmentFolderPath = path.parent_path();
					OnEnvironmentLoad();
				}
			});

		ImGuiWrapper::Render();
	}

	void Editor::Draw_CreateEnv()
	{
		ImGuiWrapper::NewFrame();

		ImGui::SetNextWindowDockID(ImGui::DockSpaceOverViewport(), ImGuiCond_::ImGuiCond_Always);
		ImGui::Begin("Create Environment", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

		ImGui::Text("Environment Folder");
		ImGui::SameLine();
		ImGui::InputText("##Environment Folder", &m_EnvironmentCreateFolderPath);
		ImGui::SameLine();
		if (ImGui::Button("Browser"))
		{
			m_FolderBrowser.WindowOpen = true;
		}

		bool fileExists = false, fileIsDirectory = false, fileIsEmpty = false;
		ImGui::Text("Create Directory For Environment: ");
		ImGui::SameLine();
		ImGui::Checkbox("##Create Directory For Environment: ", &m_CreateEvironmentDirectory);
		bool canCreateDirectory = !std::filesystem::exists(m_EnvironmentCreateFolderPath + m_EnvironmentCreateName + "\\");

		fileExists = std::filesystem::exists(m_EnvironmentCreateFolderPath);

		if (fileExists)
		{
			fileIsDirectory = std::filesystem::is_directory(m_EnvironmentCreateFolderPath);
			if (fileIsDirectory)
				fileIsEmpty = std::filesystem::is_empty(m_EnvironmentCreateFolderPath);
		}

		bool unsupportedEnvironmentName =
			m_EnvironmentCreateName.find(" ") != std::string::npos
			|| m_EnvironmentCreateName.find(".") != std::string::npos
			|| m_EnvironmentCreateName.find("\\") != std::string::npos
			|| m_EnvironmentCreateName.find("/") != std::string::npos
			|| m_EnvironmentCreateName.find("\'") != std::string::npos
			|| m_EnvironmentCreateName.find("\"") != std::string::npos
			|| m_EnvironmentCreateName == "";

		ImGui::Text("Environment Name");
		ImGui::SameLine();
		ImGui::InputTextWithHint("##Environment Name", "MyEnvironment", &m_EnvironmentCreateName);

		bool canSaveEnvironment = false;

		if (fileExists     //the directory we create our environment in or the directory that we create the folder that holds our environment exists
			&& fileIsDirectory //make sure it is a directory
			&& (fileIsEmpty || m_CreateEvironmentDirectory)  //make sure the directory is not empty if aren't making a seperate directory for the environment
			&& (!m_CreateEvironmentDirectory || (m_CreateEvironmentDirectory && canCreateDirectory)) //if we are making a directory make sure there is no one like it
			&& !unsupportedEnvironmentName) //environment name is not valid
		{
			canSaveEnvironment = true;
		}
		else
		{
			ImVec4 redColor = { 0.8f, 0.0f, 0.0f, 1.0f };

			//display why the path is not valid
			if (!fileExists)
				ImGui::TextColored(redColor, "Directoy does not exists");
			else if (!fileIsDirectory)
				ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "File is not a Directory");
			else if (!fileIsEmpty && !m_CreateEvironmentDirectory)
				ImGui::TextColored(redColor, "Directory is not empty");
			else if (unsupportedEnvironmentName)
				ImGui::TextColored(redColor, "Name is not valid");
			else if (m_CreateEvironmentDirectory && !canCreateDirectory)
				ImGui::TextColored(redColor, "Directory already exists");
			else
				assert(false); //we should never get to here

			canSaveEnvironment = false;
		}

		ImGui::Text("Include Starter Assets");
		ImGui::SameLine();
		ImGui::Checkbox("##Include Starter Assets", &m_IncludeStarterAssets);

		ImGui::SetCursorPosY(ImGui::GetWindowSize().y - (START_MENU_BUTTON_HEIGHT + 10));

		if (ImGui::Button("Cancel", ImVec2(START_MENU_BUTTON_WIDTH, START_MENU_BUTTON_HEIGHT)))
		{
			m_State = State_NoEnvLoaded;
			Window::SetSize({ WINDOW_SIZE_ON_LAUNCH_X, WINDOW_SIZE_ON_LAUNCH_Y });
			Window::CenterWindow();
		}

		//change create button color to green to show that the path given is valid
		if (canSaveEnvironment)
			ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.8f, 0.0f, 1.0f });
		//change create button color to red to show that the path given is not valid
		else
			ImGui::PushStyleColor(ImGuiCol_Button, { 0.8f, 0.0f, 0.0f, 1.0f });

		ImGui::SetCursorPosY(ImGui::GetWindowSize().y - (START_MENU_BUTTON_HEIGHT + 10));
		ImGui::SetCursorPosX(ImGui::GetWindowSize().x - (START_MENU_BUTTON_WIDTH + 10));

		if (ImGui::Button("Create", ImVec2(START_MENU_BUTTON_WIDTH, START_MENU_BUTTON_HEIGHT)))
		{
			if (canSaveEnvironment) {
				Window::Maximize();

				//make sure the folder path has a backslash at the end
				if (m_EnvironmentCreateFolderPath[m_EnvironmentCreateFolderPath.size() - 1] != '\\')
					m_EnvironmentCreateFolderPath = m_EnvironmentCreateFolderPath + "\\";

				std::string dirPath = "";
				if (m_CreateEvironmentDirectory)
				{
					dirPath = m_EnvironmentCreateFolderPath + m_EnvironmentCreateName + "\\";
					std::filesystem::create_directory(dirPath);
				}
				else
				{
					dirPath = m_EnvironmentCreateFolderPath;
				}

				m_Env = new Environment;
				m_Env->Name = m_EnvironmentCreateName;

				if (m_IncludeStarterAssets)
					std::filesystem::copy("res\\StarterAssets", dirPath + "\\StarterAssets");

				m_State = State_EditorMode;
				OnEnvironmentLoad();
			}
		}

		ImGui::PopStyleColor();

		ImGui::End();

		m_FolderBrowser.DisplayGUI([this](const std::string& dir) {
			m_EnvironmentCreateFolderPath = dir;
			});

		ImGuiWrapper::Render();
	}

	void Editor::Draw_EditorMode()
	{
		SceneDescription desc;
		desc.SceneCamera = m_Camera;
		desc.SceneDrawTarget = m_RenderSurface.SurfaceFrameBuffer;
		desc.SceneDrawTargetTexture = m_RenderSurface.m_Texture;
		desc.Blur = m_Env->BlurEnabled;
		desc.BlurRadius = m_Env->BlurRadius;
		Renderer::BeginScene(desc);

		for (pEnvironmentObject& obj : m_Env->Objects)
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

		m_Background.Draw(*m_Env);

		for (pEnvironmentObject& obj : m_Env->Objects)
		{
			if (obj->Type == SpriteType)
				obj->Draw();
		}

		for (pEnvironmentObject& obj : m_Env->Objects)
			if (obj->Selected) {
				//draw object position gizmo
				m_Gizmo.Draw(obj->GetPositionRef(), m_ViewportWindow.RenderViewport);

				//if particle system needs to edit a force target (a world point), use a gimzo for it
				if (obj->Type == EnvironmentObjectType::ParticleSystemType)
				{
					auto ps = static_cast<ParticleSystem*>(obj.get());
					if (ps->Customizer.m_ForceCustomizer.m_CurrentSelectedForceName != "")
						if (ps->Customizer.m_ForceCustomizer.m_Forces[ps->Customizer.m_ForceCustomizer.m_CurrentSelectedForceName].Type == Force::RelativeForce)
							m_Gizmo.Draw(ps->Customizer.m_ForceCustomizer.m_Forces[ps->Customizer.m_ForceCustomizer.m_CurrentSelectedForceName].RF_Target, m_ViewportWindow.RenderViewport);
				}
			}

		//Render world space gui here because we need camera information for that
		for (pEnvironmentObject& obj : m_Env->Objects)
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

		Renderer::EndScene();
		m_RenderSurface.RenderToScreen(m_ViewportWindow.RenderViewport);
		if (m_ShowGrid)
			m_Grid.Draw();
		m_ExportCamera.DrawOutline();

		//GUI
		ImGuiWrapper::NewFrame();

		DisplayMainMenuBarGUI();

		float menuBarHeight = ImGui::GetFrameHeightWithSpacing();
		ImGuiViewport viewport;
		viewport.Size = ImVec2(Window::FramebufferSize.x, Window::FramebufferSize.y - menuBarHeight);
		viewport.Pos = ImVec2(Window::Position.x, Window::Position.y + menuBarHeight);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		auto viewportDockID = ImGui::DockSpaceOverViewport(&viewport, ImGuiDockNodeFlags_PassthruCentralNode, 0);
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);

		AssetManager::DisplayGUI();
		DisplayEnvironmentControlsGUI();
		DisplayObjectInspecterGUI();
		DisplayProfilerGUI();
		m_AppStatusWindow.DisplayGUI(viewportDockID);

		//Settings window
		ImGui::Begin("Settings", &m_EnvironmentSettingsWindowOpen);

		if (ImGui::TreeNode("Blend Settings:"))
		{
			ImGui::Text("Mode");
			ImGui::SameLine();
			if (ImGui::BeginCombo("##Mode", (m_Env->BlendMode == RenderingBlendMode::Additive) ? "Additive" : "Screen"))
			{
				{
					bool is_Active = m_Env->BlendMode == RenderingBlendMode::Additive;
					if (ImGui::Selectable("Additive", &is_Active)) {

						ImGui::SetItemDefaultFocus();
						m_Env->BlendMode = RenderingBlendMode::Additive;
					}
				}

				{
					bool is_Active = m_Env->BlendMode == RenderingBlendMode::Screen;
					if (ImGui::Selectable("Screen", &is_Active)) {

						ImGui::SetItemDefaultFocus();
						m_Env->BlendMode = RenderingBlendMode::Screen;
					}
				}
				glEnable(GL_BLEND);

				if (m_Env->BlendMode == RenderingBlendMode::Additive)
					glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
				else if (m_Env->BlendMode == RenderingBlendMode::Screen)
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);

				ImGui::EndCombo();

			}

			ImGui::TreePop();
		}
		ImGui::Text("Show Grid");
		ImGui::SameLine();
		ImGui::SetCursorPosX(100.0f);
		ImGui::Checkbox("##Show Grid", &m_ShowGrid);

		ImGui::Text("Blur");
		ImGui::SameLine();
		ImGui::SetCursorPosX(100.0f);
		ImGui::Checkbox("##Blur", &m_Env->BlurEnabled);

		if (m_Env->BlurEnabled) {
			if (ImGui::TreeNode("Blur Settings: ")) {

				ImGui::Text("Blur Radius: ");
				ImGui::SameLine();
				ImGui::DragFloat("##Blur Radius: ", &m_Env->BlurRadius, 0.01f, 0.0f, 5.0f);

				ImGui::TreePop();
			}
		}

		ImGui::End();

		m_ExportCamera.DisplayGUI();
		m_Background.DisplayGUI(*m_Env);

		for (pEnvironmentObject& obj : m_Env->Objects)
			obj->DisplayGUI();

		InputManager::DisplayGUI();
		m_ViewportWindow.DisplayGUI();
		ImGui::PopStyleColor();

		ImGuiWrapper::Render();
	}

	void Editor::Draw_PlayMode()
	{
		SceneDescription desc;
		desc.SceneCamera = m_Camera;
		desc.SceneDrawTarget = m_RenderSurface.SurfaceFrameBuffer;
		desc.SceneDrawTargetTexture = m_RenderSurface.m_Texture;
		desc.Blur = m_Env->BlurEnabled;
		desc.BlurRadius = m_Env->BlurRadius;
		Renderer::BeginScene(desc);

		for (pEnvironmentObject& obj : m_Env->Objects)
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

		m_Background.Draw(*m_Env);

		for (pEnvironmentObject& obj : m_Env->Objects)
		{
			if (obj->Type == SpriteType)
				obj->Draw();
		}

		//stuff to only render in play mode and export mode
		for (pEnvironmentObject& obj : m_Env->Objects)
		{
			//because we already drawed all the sprites
			if (obj->Type == SpriteType)
				continue;
			obj->Draw();
		}

		Renderer::EndScene();

		//draw this after the scene is drawn so that post processing effects do not apply to it
		m_ExportCamera.DrawOutline();

		m_RenderSurface.RenderToScreen(m_ViewportWindow.RenderViewport);

		m_RenderSurface.SurfaceFrameBuffer->Unbind();

		//GUI
		ImGuiWrapper::NewFrame();

		DisplayMainMenuBarGUI();

		float menuBarHeight = ImGui::GetFrameHeightWithSpacing();
		ImGuiViewport viewport;
		viewport.Size = ImVec2(Window::FramebufferSize.x, Window::FramebufferSize.y - menuBarHeight);
		viewport.Pos = ImVec2(Window::Position.x, Window::Position.y + menuBarHeight);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		auto viewportDockID = ImGui::DockSpaceOverViewport(&viewport, ImGuiDockNodeFlags_PassthruCentralNode, 0);
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);

		AssetManager::DisplayGUI();
		DisplayEnvironmentControlsGUI();
		DisplayObjectInspecterGUI();
		DisplayProfilerGUI();
		m_AppStatusWindow.DisplayGUI(viewportDockID);

		//Settings window
		ImGui::Begin("Settings", &m_EnvironmentSettingsWindowOpen);

		if (ImGui::TreeNode("Blend Settings:"))
		{
			ImGui::Text("Mode");
			ImGui::SameLine();
			if (ImGui::BeginCombo("##Mode", (m_Env->BlendMode == RenderingBlendMode::Additive) ? "Additive" : "Screen"))
			{
				{
					bool is_Active = m_Env->BlendMode == RenderingBlendMode::Additive;
					if (ImGui::Selectable("Additive", &is_Active)) {

						ImGui::SetItemDefaultFocus();
						m_Env->BlendMode = RenderingBlendMode::Additive;
					}
				}

				{
					bool is_Active = m_Env->BlendMode == RenderingBlendMode::Screen;
					if (ImGui::Selectable("Screen", &is_Active)) {

						ImGui::SetItemDefaultFocus();
						m_Env->BlendMode = RenderingBlendMode::Screen;
					}
				}
				glEnable(GL_BLEND);

				if (m_Env->BlendMode == RenderingBlendMode::Additive)
					glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
				else if (m_Env->BlendMode == RenderingBlendMode::Screen)
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);

				ImGui::EndCombo();

			}

			ImGui::TreePop();
		}
		ImGui::Text("Show Grid");
		ImGui::SameLine();
		ImGui::SetCursorPosX(100.0f);
		ImGui::Checkbox("##Show Grid", &m_ShowGrid);

		ImGui::Text("Blur");
		ImGui::SameLine();
		ImGui::SetCursorPosX(100.0f);
		ImGui::Checkbox("##Blur", &m_Env->BlurEnabled);

		if (m_Env->BlurEnabled) {
			if (ImGui::TreeNode("Blur Settings: ")) {

				ImGui::Text("Blur Radius: ");
				ImGui::SameLine();
				ImGui::DragFloat("##Blur Radius: ", &m_Env->BlurRadius, 0.01f, 0.0f, 5.0f);

				ImGui::TreePop();
			}
		}

		ImGui::End();

		m_ExportCamera.DisplayGUI();
		m_Background.DisplayGUI(*m_Env);

		for (pEnvironmentObject& obj : m_Env->Objects)
			obj->DisplayGUI();

		InputManager::DisplayGUI();
		m_ViewportWindow.DisplayGUI();
		ImGui::PopStyleColor();

		ImGuiWrapper::Render();
	}

	void Editor::Draw_PauseMode()
	{
		SceneDescription desc;
		desc.SceneCamera = m_Camera;
		desc.SceneDrawTarget = m_RenderSurface.SurfaceFrameBuffer;
		desc.SceneDrawTargetTexture = m_RenderSurface.m_Texture;
		desc.Blur = m_Env->BlurEnabled;
		desc.BlurRadius = m_Env->BlurRadius;
		Renderer::BeginScene(desc);

		for (pEnvironmentObject& obj : m_Env->Objects)
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

		m_Background.Draw(*m_Env);

		for (pEnvironmentObject& obj : m_Env->Objects)
		{
			if (obj->Type == SpriteType)
				obj->Draw();
		}

		//stuff to only render in play mode and export mode
		for (pEnvironmentObject& obj : m_Env->Objects)
		{
			//because we already drawed all the sprites
			if (obj->Type == SpriteType)
				continue;
			obj->Draw();
		}

		Renderer::EndScene();

		//draw this after the scene is drawn so that post processing effects do not apply to it
		m_ExportCamera.DrawOutline();

		m_RenderSurface.RenderToScreen(m_ViewportWindow.RenderViewport);

		m_RenderSurface.SurfaceFrameBuffer->Unbind();

		//GUI
		ImGuiWrapper::NewFrame();

		DisplayMainMenuBarGUI();

		float menuBarHeight = ImGui::GetFrameHeightWithSpacing();
		ImGuiViewport viewport;
		viewport.Size = ImVec2(Window::FramebufferSize.x, Window::FramebufferSize.y - menuBarHeight);
		viewport.Pos = ImVec2(Window::Position.x, Window::Position.y + menuBarHeight);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		auto viewportDockID = ImGui::DockSpaceOverViewport(&viewport, ImGuiDockNodeFlags_PassthruCentralNode, 0);
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);

		AssetManager::DisplayGUI();
		DisplayEnvironmentControlsGUI();
		DisplayObjectInspecterGUI();
		DisplayProfilerGUI();
		m_AppStatusWindow.DisplayGUI(viewportDockID);

		//Settings window
		ImGui::Begin("Settings", &m_EnvironmentSettingsWindowOpen);

		if (ImGui::TreeNode("Blend Settings:"))
		{
			ImGui::Text("Mode");
			ImGui::SameLine();
			if (ImGui::BeginCombo("##Mode", (m_Env->BlendMode == RenderingBlendMode::Additive) ? "Additive" : "Screen"))
			{
				{
					bool is_Active = m_Env->BlendMode == RenderingBlendMode::Additive;
					if (ImGui::Selectable("Additive", &is_Active)) {

						ImGui::SetItemDefaultFocus();
						m_Env->BlendMode = RenderingBlendMode::Additive;
					}
				}

				{
					bool is_Active = m_Env->BlendMode == RenderingBlendMode::Screen;
					if (ImGui::Selectable("Screen", &is_Active)) {

						ImGui::SetItemDefaultFocus();
						m_Env->BlendMode = RenderingBlendMode::Screen;
					}
				}
				glEnable(GL_BLEND);

				if (m_Env->BlendMode == RenderingBlendMode::Additive)
					glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
				else if (m_Env->BlendMode == RenderingBlendMode::Screen)
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);

				ImGui::EndCombo();

			}

			ImGui::TreePop();
		}
		ImGui::Text("Show Grid");
		ImGui::SameLine();
		ImGui::SetCursorPosX(100.0f);
		ImGui::Checkbox("##Show Grid", &m_ShowGrid);

		ImGui::Text("Blur");
		ImGui::SameLine();
		ImGui::SetCursorPosX(100.0f);
		ImGui::Checkbox("##Blur", &m_Env->BlurEnabled);

		if (m_Env->BlurEnabled) {
			if (ImGui::TreeNode("Blur Settings: ")) {

				ImGui::Text("Blur Radius: ");
				ImGui::SameLine();
				ImGui::DragFloat("##Blur Radius: ", &m_Env->BlurRadius, 0.01f, 0.0f, 5.0f);

				ImGui::TreePop();
			}
		}

		ImGui::End();

		m_ExportCamera.DisplayGUI();
		m_Background.DisplayGUI(*m_Env);

		for (pEnvironmentObject& obj : m_Env->Objects)
			obj->DisplayGUI();

		InputManager::DisplayGUI();
		m_ViewportWindow.DisplayGUI();
		ImGui::PopStyleColor();

		ImGuiWrapper::Render();
	}

	void Editor::Draw_ExportMode()
	{
		SceneDescription desc;
		desc.SceneCamera = m_Camera;
		desc.SceneDrawTarget = m_RenderSurface.SurfaceFrameBuffer;
		desc.SceneDrawTargetTexture = m_RenderSurface.m_Texture;
		desc.Blur = m_Env->BlurEnabled;
		desc.BlurRadius = m_Env->BlurRadius;
		Renderer::BeginScene(desc);

		for (pEnvironmentObject& obj : m_Env->Objects)
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

		m_Background.Draw(*m_Env);

		for (pEnvironmentObject& obj : m_Env->Objects)
		{
			if (obj->Type == SpriteType)
				obj->Draw();
		}

		//stuff to only render in play mode and export mode
		for (pEnvironmentObject& obj : m_Env->Objects)
		{
			//because we already drawed all the sprites
			if (obj->Type == SpriteType)
				continue;
			obj->Draw();
		}

		Renderer::EndScene();

		//draw this after the scene is drawn so that post processing effects do not apply to it
		m_ExportCamera.DrawOutline();

		m_RenderSurface.RenderToScreen(m_ViewportWindow.RenderViewport);

		if (m_ExportCamera.ImageCaptureTime < m_TimeSincePlayModeStarted)
		{
			m_ExportCamera.ExportFrame(*m_Env);
			m_ExportedFrame = true;
		}

		m_RenderSurface.SurfaceFrameBuffer->Unbind();

		//GUI
		ImGuiWrapper::NewFrame();

		DisplayMainMenuBarGUI();

		float menuBarHeight = ImGui::GetFrameHeightWithSpacing();
		ImGuiViewport viewport;
		viewport.Size = ImVec2(Window::FramebufferSize.x, Window::FramebufferSize.y - menuBarHeight);
		viewport.Pos = ImVec2(Window::Position.x, Window::Position.y + menuBarHeight);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		auto viewportDockID = ImGui::DockSpaceOverViewport(&viewport, ImGuiDockNodeFlags_PassthruCentralNode, 0);
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);

		AssetManager::DisplayGUI();
		DisplayEnvironmentControlsGUI();
		DisplayObjectInspecterGUI();
		DisplayProfilerGUI();
		m_AppStatusWindow.DisplayGUI(viewportDockID);

		//Settings window
		ImGui::Begin("Settings", &m_EnvironmentSettingsWindowOpen);

		if (ImGui::TreeNode("Blend Settings:"))
		{
			ImGui::Text("Mode");
			ImGui::SameLine();
			if (ImGui::BeginCombo("##Mode", (m_Env->BlendMode == RenderingBlendMode::Additive) ? "Additive" : "Screen"))
			{
				{
					bool is_Active = m_Env->BlendMode == RenderingBlendMode::Additive;
					if (ImGui::Selectable("Additive", &is_Active)) {

						ImGui::SetItemDefaultFocus();
						m_Env->BlendMode = RenderingBlendMode::Additive;
					}
				}

				{
					bool is_Active = m_Env->BlendMode == RenderingBlendMode::Screen;
					if (ImGui::Selectable("Screen", &is_Active)) {

						ImGui::SetItemDefaultFocus();
						m_Env->BlendMode = RenderingBlendMode::Screen;
					}
				}
				glEnable(GL_BLEND);

				if (m_Env->BlendMode == RenderingBlendMode::Additive)
					glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
				else if (m_Env->BlendMode == RenderingBlendMode::Screen)
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);

				ImGui::EndCombo();

			}

			ImGui::TreePop();
		}
		ImGui::Text("Show Grid");
		ImGui::SameLine();
		ImGui::SetCursorPosX(100.0f);
		ImGui::Checkbox("##Show Grid", &m_ShowGrid);

		ImGui::Text("Blur");
		ImGui::SameLine();
		ImGui::SetCursorPosX(100.0f);
		ImGui::Checkbox("##Blur", &m_Env->BlurEnabled);

		if (m_Env->BlurEnabled) {
			if (ImGui::TreeNode("Blur Settings: ")) {

				ImGui::Text("Blur Radius: ");
				ImGui::SameLine();
				ImGui::DragFloat("##Blur Radius: ", &m_Env->BlurRadius, 0.01f, 0.0f, 5.0f);

				ImGui::TreePop();
			}
		}

		ImGui::End();

		m_ExportCamera.DisplayGUI();
		m_Background.DisplayGUI(*m_Env);

		for (pEnvironmentObject& obj : m_Env->Objects)
			obj->DisplayGUI();

		InputManager::DisplayGUI();
		m_ViewportWindow.DisplayGUI();
		ImGui::PopStyleColor();

		ImGuiWrapper::Render();
	}

	void Editor::OnEnvironmentLoad()
	{
		AssetManager::Init(m_EnvironmentFolderPath.u8string());
		InputManager::Init();

		Window::Maximize();
		Window::WindowSizeChangedSinceLastFrame = true;
		m_RenderSurface.SetSize(Window::FramebufferSize);
		RegisterEnvironmentInputKeys();

		std::memset(m_DeltaTimeHistory.data(), 0, m_DeltaTimeHistory.size() * sizeof(float));

		UpdateTitle();
	}

	void Editor::OnEnvironmentDestroy()
	{
		AssetManager::Terminate();
		InputManager::Terminate();
		Window::Restore();
		Window::SetSize({ WINDOW_SIZE_FACTOR_ON_LAUNCH, WINDOW_SIZE_FACTOR_ON_LAUNCH * 9 / 16 });
		Window::CenterWindow();
	}

	void Editor::DisplayMainMenuBarGUI()
	{
		if (ImGui::BeginMainMenuBar()) {

			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Save"))
					SaveEnvironment(*m_Env, m_EnvironmentFolderPath.u8string() + "\\" + m_Env->Name + ".env");

				if (ImGui::MenuItem("Close Environment"))
				{
					m_ShouldDeleteEnv = true;
					OnEnvironmentDestroy();
					m_State = State_NoEnvLoaded;
				}

				if (ImGui::MenuItem("Exit"))
					exit(0);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit")) {

				if (ImGui::MenuItem("Clear Particle Systems"))
					m_Env->Objects.clear();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Window")) {

				ImGui::MenuItem("Environment Controls", nullptr, &m_EnvironmentControlsWindowOpen);
				ImGui::MenuItem("Object Inspector", nullptr, &m_ObjectInspectorWindowOpen);
				ImGui::MenuItem("General Settings", nullptr, &m_EnvironmentSettingsWindowOpen);
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

	void Editor::DisplayEnvironmentControlsGUI()
	{
		//return if window is not open
		if (!m_EnvironmentControlsWindowOpen)
			return;

		ImGui::Begin("Controls", &m_EnvironmentControlsWindowOpen, ImGuiWindowFlags_NoScrollbar);
		if (m_State == State_ExportMode)
		{
			ImGui::End();
			return;
		}

		//because ImGui doesnt accept different colors for different image button states, we have to use data from last frame
		static bool s_playButtonHovered = false;
		static bool s_stopButtonHovered = false;
		static bool s_pauseButtonHovered = false;

		//we keep track of how many frames have passed since the button is hovered so that we can display a tool tip if 
		//it is hovered for too long
		static int s_playButtonHoverFrameCount = 0;
		static int s_stopButtonHoverFrameCount = 0;
		static int s_pauseButtonHoverFrameCount = 0;

		ImVec4& bgColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
		ImVec4& buttonColor = ImGui::GetStyle().Colors[ImGuiCol_Button];
		ImVec4& buttonHoveredColor = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];

		const ImVec2 c_buttonSize = { 50, 50 };

		//code for displaying each of the control buttons in lambdas so that we can reuse it
		auto displayPlayButton = [this, &bgColor, &buttonColor, &buttonHoveredColor, &c_buttonSize]() {
			ImVec4 playButtonoTint = s_playButtonHovered ? buttonHoveredColor : buttonColor;
			if (ImGui::ImageButton((void*)(uintptr_t)m_PlayButtonTexture->GetRendererID(),
				c_buttonSize,
				ImVec2(0, 0),
				ImVec2(1, 1),
				0,
				bgColor,
				playButtonoTint))
			{
				Resume();
			}

			s_playButtonHovered = ImGui::IsItemHovered();
			if (s_playButtonHovered)
			{
				s_playButtonHoverFrameCount++;
				if (s_playButtonHoverFrameCount > 30)
					ImGui::SetTooltip("Play");
			}
			else
				s_playButtonHoverFrameCount = 0;
		};

		auto displayStopButton = [this, &bgColor, &buttonColor, &buttonHoveredColor, &c_buttonSize]() {
			ImVec4 stopButtonoTint = s_stopButtonHovered ? buttonHoveredColor : buttonColor;
			if (ImGui::ImageButton((void*)(uintptr_t)m_StopButtonTexture->GetRendererID(),
				ImVec2(50, 50),
				ImVec2(0, 0),
				ImVec2(1, 1),
				0,
				bgColor,
				stopButtonoTint))
			{
				Stop();
			}

			s_stopButtonHovered = ImGui::IsItemHovered();
			if (s_stopButtonHovered)
			{
				s_stopButtonHoverFrameCount++;
				if (s_stopButtonHoverFrameCount > 30)
					ImGui::SetTooltip("Stop");
			}
			else
				s_stopButtonHoverFrameCount = 0;
		};

		auto displayPauseButton = [this, &bgColor, &buttonColor, &buttonHoveredColor, &c_buttonSize]() {
			ImVec4 pauseButtonoTint = s_pauseButtonHovered ? buttonHoveredColor : buttonColor;
			if (ImGui::ImageButton((void*)(uintptr_t)m_PauseButtonTexture->GetRendererID(),
				ImVec2(50, 50),
				ImVec2(0, 0),
				ImVec2(1, 1),
				0,
				bgColor,
				pauseButtonoTint))
			{
				Pause();
			}

			s_pauseButtonHovered = ImGui::IsItemHovered();
			static int hoverFrameCount;
			if (s_pauseButtonHovered)
			{
				s_pauseButtonHoverFrameCount++;
				if (s_pauseButtonHoverFrameCount > 30)
					ImGui::SetTooltip("Pause");
			}
			else
				s_pauseButtonHoverFrameCount = 0;
		};

		int windowcentre = ImGui::GetWindowSize().x / 2.0f;

		//display the buttons depending on the state of the environment
		switch (m_State)
		{
		case State_EditorMode:
			ImGui::SetCursorPosX(windowcentre - c_buttonSize.x / 2.0f);
			displayPlayButton();
			break;

		case State_PlayMode:
			ImGui::SetCursorPosX(windowcentre - c_buttonSize.x);
			displayStopButton();
			ImGui::SameLine();
			displayPauseButton();
			break;

		case State_PauseMode:
			ImGui::SetCursorPosX(windowcentre - c_buttonSize.x);
			displayStopButton();
			ImGui::SameLine();
			displayPlayButton();
			break;

		default:
			break;
		}

		if (ImGui::Button("Export"))
			ExportMode();

		ImGui::End();
	}

	void Editor::Stop()
	{
		m_State = State_EditorMode;

		for (pEnvironmentObject& obj : m_Env->Objects) {
			if (obj->Type == EnvironmentObjectType::ParticleSystemType) {
				ParticleSystem* ps = static_cast<ParticleSystem*>(obj.get());
				ps->ClearParticles();
			}
		}
	}

	void Editor::Pause()
	{
		m_State = State_PauseMode;
	}

	void Editor::Resume()
	{
		m_State = State_PlayMode;
	}

	void Editor::ExportMode()
	{
		m_State = State_ExportMode;
		m_TimeSincePlayModeStarted = 0.0f;
		m_ExportedFrame = false;
	}

	void Editor::PlayMode()
	{
		m_State = State_PlayMode;
		//reset profiler
		m_TimeSincePlayModeStarted = 0.0f;
		std::memset(m_DeltaTimeHistory.data(), 0, m_DeltaTimeHistory.size() * sizeof(float));
	}

	void Editor::DisplayObjectInspecterGUI()
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

			for (int i = 0; i < m_Env->Objects.size(); i++)
			{
				ImGui::PushID(m_Env->Objects[i].get());

				if (ImGui::Selectable((m_Env->Objects[i]->m_Name.size() > 0) ? m_Env->Objects[i]->m_Name.c_str() : "No Name", &m_Env->Objects[i]->Selected))
				{
					//if this is selected. deselect all other particle systems
					for (auto& particle : m_Env->Objects) {
						if (particle.get() != m_Env->Objects[i].get())
							particle->Selected = false;
					}
				}

				if (ImGui::BeginDragDropSource())
				{
					ImGui::Text(("Moving: " + m_Env->Objects[i]->m_Name).c_str());
					ImGui::SetDragDropPayload("re-order", &m_Env->Objects[i]->Order, sizeof(int), ImGuiCond_Once);
					ImGui::EndDragDropSource();
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("re-order"))
					{
						size_t switchTarget1 = *(int*)payload->Data;
						size_t switchTarget2 = i;

						EnvironmentObjectInterface* temp = m_Env->Objects[switchTarget1].release();
						m_Env->Objects[switchTarget1] = std::unique_ptr<EnvironmentObjectInterface>(m_Env->Objects[switchTarget2].release());
						m_Env->Objects[switchTarget2] = std::unique_ptr<EnvironmentObjectInterface>(temp);

						RefreshObjectOrdering();
					}

					ImGui::EndDragDropTarget();
				}

				//show menu when right clicking
				if (ImGui::BeginPopupContextItem("Object Popup"))
				{
					if (ImGui::Selectable("Edit"))
						m_Env->Objects[i]->EditorOpen = !m_Env->Objects[i]->EditorOpen;

					if (ImGui::Selectable("Delete")) {
						m_Env->Objects[i]->ToBeDeleted = true;
					}

					if (ImGui::Selectable("Duplicate"))
					{
						Duplicate(*m_Env->Objects[i]);
						ImGui::EndPopup();
						ImGui::PopID();
						continue;
					}

					if (ImGui::Selectable("Rename"))
						m_Env->Objects[i]->RenameTextOpen = !m_Env->Objects[i]->RenameTextOpen;


					ImGui::EndPopup();
				}

				//display particle system buttons only if it is selected
				if (m_Env->Objects[i]->Selected)
				{
					if (ImGui::Button("Edit"))
						m_Env->Objects[i]->EditorOpen = !m_Env->Objects[i]->EditorOpen;

					ImGui::SameLine();
					if (ImGui::Button("Delete"))
						m_Env->Objects[i]->ToBeDeleted = true;

					ImGui::SameLine();
					if (ImGui::Button("Rename"))
						m_Env->Objects[i]->RenameTextOpen = !m_Env->Objects[i]->RenameTextOpen;

					ImGui::SameLine();

					if (ImGui::Button("Find"))
						FocusCameraOnObject(*m_Env->Objects[i]);
				}

				ImGui::Spacing();

				if (m_Env->Objects[i]->RenameTextOpen) {
					auto flags = ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue;
					if (ImGui::InputText("Name", &m_Env->Objects[i]->m_Name, flags)) {
						m_Env->Objects[i]->RenameTextOpen = !m_Env->Objects[i]->RenameTextOpen;
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

	void Editor::RefreshObjectOrdering()
	{
		for (size_t i = 0; i < m_Env->Objects.size(); i++)
			m_Env->Objects[i]->Order = i;
	}

	void Editor::Duplicate(EnvironmentObjectInterface& obj)
	{

		//if this object is a particle system
		if (obj.Type == EnvironmentObjectType::ParticleSystemType)
		{
			//make a new particle system
			m_Env->Objects.push_back(std::make_unique<ParticleSystem>(*static_cast<ParticleSystem*>(&obj)));

			//add a -copy to the name of the new particle system to indicate that it was copied
			m_Env->Objects[m_Env->Objects.size() - 1]->m_Name += "-copy";
		}

		//if this object is a radial light
		else if (obj.Type == EnvironmentObjectType::RadialLightType)
		{
			//make a new radial light
			m_Env->Objects.push_back(std::make_unique<RadialLight>(*static_cast<RadialLight*>(&obj)));

			//add a -copy to the name of the new light to indicate that it was copied
			m_Env->Objects[m_Env->Objects.size() - 1]->m_Name += "-copy";
		}

		//if this object is a spot light
		else if (obj.Type == EnvironmentObjectType::SpotLightType)
		{
			//make a new radial light
			m_Env->Objects.push_back(std::make_unique<SpotLight>(*static_cast<SpotLight*>(&obj)));

			//add a -copy to the name of the new light to indicate that it was copied
			m_Env->Objects[m_Env->Objects.size() - 1]->m_Name += "-copy";
		}

		//if this object is a Sprite
		else if (obj.Type == EnvironmentObjectType::SpriteType)
		{
			//make a new radial light
			m_Env->Objects.push_back(std::make_unique<Sprite>(*static_cast<Sprite*>(&obj)));

			//add a -copy to the name of the new light to indicate that it was copied
			m_Env->Objects[m_Env->Objects.size() - 1]->m_Name += "-copy";
		}
	}
	void Editor::FocusCameraOnObject(EnvironmentObjectInterface& object)
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

	void Editor::AddEnvironmentObject(EnvironmentObjectType type, const std::string& name)
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
		m_AppStatusWindow.SetText("Created Object : \"" + obj->m_Name + '"' + " of Type : \"" + EnvironmentObjectTypeToString(obj->Type) + '"', 2.0f);

		//add the object to the list of the environment objects
		m_Env->Objects.push_back(std::move(obj));

		RefreshObjectOrdering();
	}

	void Editor::RegisterEnvironmentInputKeys()
	{
		InputManager::RegisterKey(GLFW_KEY_F5, "PlayMode/Resume", [this]() {
			if (m_State == State_EditorMode)
				PlayMode();
			if (m_State == State_PauseMode)
				Resume();
			});

		//InputManager::RegisterKey(GLFW_KEY_F1, "Hide Menus", [this]() { m_HideGUI = !m_HideGUI; });

		InputManager::RegisterKey(GLFW_KEY_SPACE, "Clear All Particles", [this]()
			{
				for (pEnvironmentObject& obj : m_Env->Objects) {
					if (obj->Type == EnvironmentObjectType::ParticleSystemType) {
						ParticleSystem* ps = static_cast<ParticleSystem*>(obj.get());
						ps->ClearParticles();
					}
				}
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
				//we don't want to zoom if the focus is not set on the viewport
				if (m_ViewportWindow.IsFocused == false)
					return;

				//move the camera's position
				m_Camera.SetPosition(m_Camera.Position + glm::vec2(0.0f, -m_Camera.ZoomFactor / 100.0f));

				//display text in the bottom right of the screen stating the new position of the camera
				displayCameraPosFunc();
			},
			//set mode as repeat because we want the camera to move smoothly
				GLFW_REPEAT);

		//the rest are the same with only a diffrent move direction, that is why they arent commented
		InputManager::RegisterKey(GLFW_KEY_S, "Move Camera Down", [this, displayCameraPosFunc]()
			{
				if (m_ViewportWindow.IsFocused == false)
					return;
				m_Camera.SetPosition(m_Camera.Position + glm::vec2(0.0f, m_Camera.ZoomFactor / 100.0f));
				displayCameraPosFunc();
			},
			GLFW_REPEAT);

		InputManager::RegisterKey(GLFW_KEY_D, "Move Camera To The Right", [this, displayCameraPosFunc]()
			{
				if (m_ViewportWindow.IsFocused == false)
					return;
				m_Camera.SetPosition(m_Camera.Position + glm::vec2(-m_Camera.ZoomFactor / 100.0f, 0.0f));
				displayCameraPosFunc();
			},
			GLFW_REPEAT);

		InputManager::RegisterKey(GLFW_KEY_A, "Move Camera To The Left", [this, displayCameraPosFunc]()
			{
				if (m_ViewportWindow.IsFocused == false)
					return;
				m_Camera.SetPosition(m_Camera.Position + glm::vec2(m_Camera.ZoomFactor / 100.0f, 0.0f));
				displayCameraPosFunc();
			},
			GLFW_REPEAT);

		//delete object keyboard shortcut
		InputManager::RegisterKey(GLFW_KEY_DELETE, "Delete Object", [this]()
			{
				for (int i = 0; i < m_Env->Objects.size(); i++)
				{
					if (m_Env->Objects[i]->Selected) 
					{
						m_Env->Objects[i]->ToBeDeleted = true;
						break;
					}
				}
			});

		//zoom in and out with mouse scroll wheel
		InputManager::m_ScrollFunctions.push_back([this](int scroll)
			{
				//we don't want to zoom if the focus is not set on the viewport
				if (m_ViewportWindow.IsFocused == false)
					return;

				//change zoom factor
				m_Camera.ZoomFactor -= scroll * 30;
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


		InputManager::RegisterMouseKey(GLFW_MOUSE_BUTTON_MIDDLE, "Change Camera Zoom to Default", [this]()
			{
				//we don't want to zoom if the focus is not set on the viewport
				if (m_ViewportWindow.IsFocused == false)
					return;

				m_Camera.ZoomFactor = c_CameraZoomFactorDefault;
				//display the new zoom factor in the bottom left of the screen
				std::stringstream stream;
				stream << std::setprecision(0);
				stream << "Zoom ";
				stream << (int)(c_CameraZoomFactorDefault * 100.0f / m_Camera.ZoomFactor);
				stream << "%%";

				m_AppStatusWindow.SetText(stream.str());
			});
	}

	void Editor::DisplayProfilerGUI()
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
			ImGui::TextColored({ 0.0f,0.8f,0.0f,1.0f }, std::to_string(static_cast<int>(1.0f / m_DeltaTime)).c_str());

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
			for (pEnvironmentObject& object : m_Env->Objects)
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

			for (pEnvironmentObject& pso : m_Env->Objects)
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

	void Editor::UpdateTitle()
	{
		RendererType currentRendererType = Renderer::m_CurrentActiveAPI->GetType();

		switch (currentRendererType)
		{
		case RendererType::OpenGL:
			Window::SetTitle("Ainan - OpenGL (3.3) - " + m_Env->Name);
			break;
		}
	}

	void Editor::StartFrame()
	{
		if (m_ShouldDeleteEnv)
		{
			delete m_Env;
			m_Env = nullptr;
			m_ShouldDeleteEnv = false;
		}

		m_TimeStart = clock();
	}

	void Editor::EndFrame()
	{
		m_TimeEnd = clock();
		m_DeltaTime = (m_TimeEnd - m_TimeStart) / 1000.0f;
	}
}
