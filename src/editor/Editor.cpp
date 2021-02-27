#include "Editor.h"

namespace Ainan 
{
	Editor::Editor():
		m_LoadEnvironmentBrowser(STARTING_BROWSER_DIRECTORY, "Load Environment"),
		m_Grid(1.0f),
		m_Preferences(EditorPreferences::LoadFromDefaultPath())
	{
		m_LoadEnvironmentBrowser.Filter.push_back(".env");
		m_LoadEnvironmentBrowser.OnCloseWindow = []() 
		{
			Window::SetSize(c_StartMenuWidth, c_StartMenuHeight);
			Window::CenterWindow();
		};

		m_PlayButtonTexture = Renderer::CreateTexture(Image::LoadFromFile("res/PlayButton.png"));
		m_PauseButtonTexture = Renderer::CreateTexture(Image::LoadFromFile("res/PauseButton.png"));
		m_StopButtonTexture = Renderer::CreateTexture(Image::LoadFromFile("res/StopButton.png"));
		m_SpriteIconTexture = Renderer::CreateTexture(Image::LoadFromFile("res/Sprite.png", TextureFormat::RGBA));
		m_LitSpriteIconTexture = Renderer::CreateTexture(Image::LoadFromFile("res/LitSprite.png", TextureFormat::RGBA));
		m_MeshIconTexture = Renderer::CreateTexture(Image::LoadFromFile("res/Model.png", TextureFormat::RGBA));
		m_ParticleSystemIconTexture = Renderer::CreateTexture(Image::LoadFromFile("res/ParticleSystem.png", TextureFormat::RGBA));
		m_RadialLightIconTexture = Renderer::CreateTexture(Image::LoadFromFile("res/RadialLight.png", TextureFormat::RGBA));
		m_SpotLightIconTexture = Renderer::CreateTexture(Image::LoadFromFile("res/SpotLight.png", TextureFormat::RGBA));

		UpdateTitle();
		SetEditorStyle(m_Preferences.Style);

		//initlize worker threads
		for (auto& thread : WorkerThreads)
		{
			thread = std::thread([this]() { WorkerThreadLoop(); });
		}

		m_GPUMemAllocated = Renderer::GetUsedGPUMemory();
	}

	Editor::~Editor()
	{
		Renderer::DestroyTexture(m_PlayButtonTexture);
		Renderer::DestroyTexture(m_PauseButtonTexture);
		Renderer::DestroyTexture(m_StopButtonTexture);
		Renderer::DestroyTexture(m_SpriteIconTexture);
		Renderer::DestroyTexture(m_LitSpriteIconTexture);
		Renderer::DestroyTexture(m_MeshIconTexture);
		Renderer::DestroyTexture(m_ParticleSystemIconTexture);
		Renderer::DestroyTexture(m_RadialLightIconTexture);
		Renderer::DestroyTexture(m_SpotLightIconTexture);

		delete m_Env;
		m_Preferences.SaveToDefaultPath();

		//terminate worker threads
		DestroyThreads = true;
		StartUpdating.notify_all();
		for (auto& thread : WorkerThreads)
		{
			thread.join();
		}
	}

	void Editor::Update()
	{
		auto io = ImGui::GetIO();

		bool mouseClicked = false;
		bool mouseReleased = false;
		for (size_t i = 0; i < 5; i++)
		{
			if (io.MouseClicked[i])
			{
				mouseClicked = true;
				break;
			}
			if (io.MouseReleased[i])
			{
				mouseReleased = true;
				break;
			}
		}

		bool keyPressed = false;
		for (size_t i = 0; i < sizeof(io.KeysDown) / sizeof(io.KeysDown[0]) ; i++)
		{
			if (io.KeysDown[i])
			{
				keyPressed = true;
				break;
			}
		}

		if (InputManager::MouseDelta.x != 0 || InputManager::MouseDelta.y != 0 || (io.MouseWheel != 0) || mouseClicked || mouseReleased || keyPressed)
		{
			m_RedrawUI = 3;
		}

		FrameCounter++;
		if (FrameCounter > c_ApplicationFramerate)
			FrameCounter = 0;
		if ((FrameCounter % c_ApplicationFramerate) == 0)
			m_GPUMemAllocated = Renderer::GetUsedGPUMemory();

		if (m_ShouldDeleteEnv)
		{
			delete m_Env;
			m_Env = nullptr;
			UpdateTitle();
			m_ShouldDeleteEnv = false;
		}

		m_SimulationDeltaTime = LastFrameDeltaTime * m_SimulationSpeedFactor;

		switch (m_State)
		{
		case State_EditorMode:
			Update_EditorMode(LastFrameDeltaTime);
			break;

		case State_PlayMode:
			Update_PlayMode(LastFrameDeltaTime);
			break;

		case State_PauseMode:
			Update_PauseMode(LastFrameDeltaTime);
			break;
		}

		m_Exporter.ExportIfScheduled(*this);
	}

	void Editor::Draw()
	{
		if (Window::Minimized)
			return;

		switch (m_State)
		{
		case State_NoEnvLoaded:
			DrawHomeWindow();
			return;

		case State_CreateEnv:
			DrawEnvironmentCreationWindow();
			return;

		case State_EditorMode:
		case State_PauseMode:
			DrawEnvironment(true);
			break;

		case State_PlayMode:
			DrawEnvironment(false);
			break;
		}

		Renderer::SetRenderTargetApplicationWindow();
		Renderer::ImGuiNewFrame();
		ImGuiWrapper::BeginGlobalDocking(true);
		DrawUI();
		ImGuiWrapper::EndGlobalDocking();

		if (m_RedrawUI > 0)
		{
			m_RedrawUI--;
			Renderer::ImGuiEndFrame(true);
		}
		else
		{
			Renderer::ImGuiEndFrame(false);

			//draw environment only and not ImGui
			if (m_State == State_PlayMode)
			{

				ImVec2 displayPos = ImGui::GetDrawData()->DisplayPos;
				ImVec2 displaySize = ImGui::GetDrawData()->DisplaySize;
				ImVec2 fbScale = ImGui::GetDrawData()->FramebufferScale;
				auto& windowsAboveViewport = Renderer::Rdata->WindowsAboveViewport;
				Renderer::PushCommand([this, displayPos, displaySize, fbScale, windowsAboveViewport]()
					{
						auto list = windowsAboveViewport;
						list.insert(list.begin(), m_ViewportWindow.pDrawEnvImGuiCmd);

						ImDrawData data;
						data.DisplayPos = displayPos;
						data.DisplaySize = displaySize;
						data.FramebufferScale = fbScale;
						data.CmdListsCount = list.size();
						data.CmdLists = list.data();
						Renderer::Rdata->CurrentActiveAPI->DrawImGui(&data);
					});
			}
		}
	}

	void Editor::WorkerThreadLoop()
	{
		std::mutex mutex;
		while (true)
		{
			{
				std::unique_lock<std::mutex> lock(mutex);
				using namespace std::chrono_literals;
				StartUpdating.wait_for(lock, 3ms);
			}

			if (DestroyThreads)
				break;

			while (true)
			{
				EnvironmentObjectInterface* obj = nullptr;

				{
					std::lock_guard lock(UpdateMutex);
					if (UpdateQueue.size() > 0)
					{
						obj = UpdateQueue.front();
						UpdateQueue.pop();
					}
					else 
					{
						FinishedUpdating.notify_all();
						break;
					}
				}
				auto mutexPtr = obj->GetMutex();
				std::lock_guard lock(*mutexPtr);
				obj->Update(m_SimulationDeltaTime);
			}
		}
	}

	void Editor::Update_EditorMode(float deltaTime)
	{
		m_Camera.Update(deltaTime, m_ViewportWindow.RenderViewport);
		m_AppStatusWindow.Update(deltaTime);

		//go through all the objects (regular and not a range based loop because we want to use std::vector::erase())
		for (int i = 0; i < m_Env->Objects.size(); i++) 
		{
			auto mutexPtr = m_Env->Objects[i]->GetMutex();
			std::lock_guard lock(*mutexPtr);

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
	}

	void Editor::Update_PlayMode(float deltaTime)
	{
		m_Camera.Update(deltaTime, m_ViewportWindow.RenderViewport);
		m_AppStatusWindow.Update(deltaTime);

		{
			{
				std::lock_guard lock(UpdateMutex);
				for (auto& obj : m_Env->Objects)
				{
					UpdateQueue.push(obj.get());
				}
			}
			StartUpdating.notify_all();
		}
		{
			static std::mutex mutex;
			std::unique_lock<std::mutex> lock(mutex);
			using namespace std::chrono_literals;
			FinishedUpdating.wait_for(lock, 3ms);
		}

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

		//this stuff is used for the profiler
		m_TimeSincePlayModeStarted += deltaTime;

		//save delta time for the profiler

		//move everything back
		std::memmove(m_DeltaTimeHistory.data(), m_DeltaTimeHistory.data() + 1, (m_DeltaTimeHistory.size() - 1) * sizeof(float));
		//register the new time
		m_DeltaTimeHistory[m_DeltaTimeHistory.size() - 1] = LastFrameDeltaTime;
	}

	void Editor::Update_PauseMode(float deltaTime)
	{
		m_Camera.Update(deltaTime, m_ViewportWindow.RenderViewport);
		m_AppStatusWindow.Update(deltaTime);

		//go through all the objects (regular and not a range based loop because we want to use std::vector::erase())
		for (int i = 0; i < m_Env->Objects.size(); i++) 
		{
			auto mutexPtr = m_Env->Objects[i]->GetMutex();
			std::lock_guard lock(*mutexPtr);

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
	}

	void Editor::DrawHomeWindow()
	{
		if (m_LoadEnvironmentBrowser.OnCloseWindow == nullptr)
		{
			m_LoadEnvironmentBrowser.OnCloseWindow = []() 
			{
				Window::SetSize(c_StartMenuWidth, c_StartMenuHeight);
				Window::CenterWindow();
			};
		}

		Renderer::ImGuiNewFrame();
		ImGuiWrapper::BeginGlobalDocking(true);
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File")) 
			{
				if (ImGui::MenuItem("Preferences"))
				{
					m_PreferencesWindowOpen = true;
				}

				if (ImGui::MenuItem("Exit"))
					Window::SetShouldClose();

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		DisplayPreferencesGUI();

		ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
		ImGui::SetNextWindowDockID(ImGui::GetID("GlobalDockSpace"));
		ImGui::Begin("Start Menu", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus);

		ImGui::DockBuilderGetNode(ImGui::GetWindowDockID())->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;

		ImGui::SetCursorPosY(c_StartMenuBtnStartTop);
		ImGui::SetCursorPosX(c_StartMenuBtnStartLeft);

		if (ImGui::Button("Create New Environment", { c_StartMenuBtnWidth, c_StartMenuBtnHeight }))
		{
			m_State = State_CreateEnv;
			Window::SetSize(c_CreateEnvironmentWindowWidth, c_CreateEnvironmentWindowHeight);
			Window::CenterWindow();
		}

		ImGui::SetCursorPosX(c_StartMenuBtnStartLeft);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + c_StartMenuBtnMarginY);

		if (ImGui::Button("Load Environment", { c_StartMenuBtnWidth, c_StartMenuBtnHeight }))
		{
			m_LoadEnvironmentBrowser.OpenWindow();
		}

		ImGui::End();

		m_LoadEnvironmentBrowser.DisplayGUI([this](const std::filesystem::path path)
			{
				//check if file is selected
				if (path.extension().u8string() == ".env") 
				{
					//remove minimizing event on file browser window close
					m_LoadEnvironmentBrowser.OnCloseWindow = nullptr;

					m_Env = LoadEnvironment(path.u8string());
					m_State = State_EditorMode;
					m_EnvironmentFolderPath = path.parent_path();
					OnEnvironmentLoad();
				}
			});

		ImGuiWrapper::EndGlobalDocking();
		if (m_RedrawUI > 0)
		{
			m_RedrawUI--;
			Renderer::ImGuiEndFrame(true);
		}
		else
			Renderer::ImGuiEndFrame(false);
	}

	void Editor::DrawEnvironmentCreationWindow()
	{
		Renderer::ImGuiNewFrame();

		ImGuiWrapper::BeginGlobalDocking(false);
		ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
		ImGui::SetNextWindowDockID(ImGui::GetID("GlobalDockSpace"));

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

		ImGui::SetCursorPosY(ImGui::GetWindowSize().y - (c_CreateEnvBtnHeight + 10));

		if (ImGui::Button("Cancel", { c_CreateEnvBtnWidth, c_CreateEnvBtnHeight }))
		{
			m_State = State_NoEnvLoaded;
			Window::SetSize(c_StartMenuWidth, c_StartMenuHeight);
			Window::CenterWindow();
		}

		//change create button color to green to show that the path given is valid
		if (canSaveEnvironment)
			ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.8f, 0.0f, 1.0f });
		//change create button color to red to show that the path given is not valid
		else
			ImGui::PushStyleColor(ImGuiCol_Button, { 0.8f, 0.0f, 0.0f, 1.0f });

		ImGui::SetCursorPosY(ImGui::GetWindowSize().y - (c_CreateEnvBtnHeight + 10));
		ImGui::SetCursorPosX(ImGui::GetWindowSize().x - (c_CreateEnvBtnWidth + 10));

		if (ImGui::Button("Create", { c_CreateEnvBtnWidth, c_CreateEnvBtnHeight }))
		{
			if (canSaveEnvironment) 
			{
				Window::Maximize();

				//make sure the folder path has a backslash at the end
				if (m_EnvironmentCreateFolderPath[m_EnvironmentCreateFolderPath.size() - 1] != '\\')
					m_EnvironmentCreateFolderPath = m_EnvironmentCreateFolderPath + "\\";

				if (m_CreateEvironmentDirectory)
				{
					m_EnvironmentCreateFolderPath = m_EnvironmentCreateFolderPath + m_EnvironmentCreateName + "\\";
					std::filesystem::create_directory(m_EnvironmentCreateFolderPath);
				}

				m_Env = new Environment;
				m_Env->Name = m_EnvironmentCreateName;

				if (m_IncludeStarterAssets)
					std::filesystem::copy("res\\StarterAssets", m_EnvironmentCreateFolderPath + "\\StarterAssets");
				m_EnvironmentFolderPath = m_EnvironmentCreateFolderPath;

				m_State = State_EditorMode;
				OnEnvironmentLoad();
			}
		}

		ImGui::PopStyleColor();

		ImGui::End();

		m_FolderBrowser.DisplayGUI([this](const std::filesystem::path& dir) 
			{
			m_EnvironmentCreateFolderPath = dir.u8string();
			});

		ImGuiWrapper::EndGlobalDocking();
		if (m_RedrawUI > 0)
		{
			m_RedrawUI--;
			Renderer::ImGuiEndFrame(true);
		}
		else
			Renderer::ImGuiEndFrame(false);
	}

	void Editor::DrawEnvironment(bool drawWorldSpaceUI)
	{
		m_RenderSurface.SurfaceFramebuffer.Bind();
		Renderer::ClearScreen();

		for (pEnvironmentObject& obj : m_Env->Objects)
		{
			auto mutexPtr = obj->GetMutex();

			if (obj->Type == RadialLightType)
			{
				RadialLight* light = static_cast<RadialLight*>(obj.get());
				Renderer::AddRadialLight(light->ModelMatrix[3], light->Color, light->Intensity);
			}
			else if (obj->Type == SpotLightType) 
			{
				SpotLight* light = static_cast<SpotLight*>(obj.get());
				glm::vec3 scale;
				glm::quat rotation;
				glm::vec3 translation;
				glm::vec3 skew;
				glm::vec4 perspective;
				glm::decompose(light->ModelMatrix, scale, rotation, translation, skew, perspective);
				Renderer::AddSpotLight(light->ModelMatrix[3], light->Color, glm::eulerAngles(rotation).z, light->InnerCutoff, light->OuterCutoff, light->Intensity);
			}
		}

		SceneDescription desc;
		desc.SceneCamera = m_Camera;
		desc.SceneDrawTarget = m_RenderSurface.SurfaceFramebuffer;
		desc.Blur = m_Env->BlurEnabled;
		desc.BlurRadius = m_Env->BlurRadius;
		Renderer::BeginScene(desc);

		for (pEnvironmentObject& obj : m_Env->Objects)
		{
			auto mutexPtr = obj->GetMutex();
			std::lock_guard lock(*mutexPtr);
			obj->Draw();
		}

		Renderer::EndScene();
		m_DrawCalls = Renderer::Rdata->NumberOfDrawCallsLastScene;

		//draw the UI as a different scene on top of the environment scene
		SceneDescription descUI;
		descUI.SceneCamera = m_Camera;
		descUI.SceneDrawTarget = m_RenderSurface.SurfaceFramebuffer;
		descUI.Blur = false;
		descUI.BlurRadius = 0;
		Renderer::SetBlendMode(RenderingBlendMode::Screen);

		Renderer::BeginScene(descUI);

		m_RenderSurface.SurfaceFramebuffer.Bind();

		if (drawWorldSpaceUI)
		{
			if (m_ShowGrid)
				m_Grid.Draw(m_Camera);

			if (m_ShowObjectIcons)
			{
				for (pEnvironmentObject& obj : m_Env->Objects)
				{
					const float scale = 0.1f;
					const glm::vec2 position = glm::vec2(obj->ModelMatrix[3]) - glm::vec2(scale, scale) / 2.0f;
					const glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

					switch (obj->Type)
					{
					case ParticleSystemType:
						Renderer::DrawQuad(position, color, scale, m_ParticleSystemIconTexture);
						if (obj->Selected)
						{
							ParticleSystem* ps = (ParticleSystem*)obj.get();
							ps->Customizer.DrawWorldSpaceUI();
						}
						break;

					case RadialLightType:
						Renderer::DrawQuad(position, color, scale, m_RadialLightIconTexture);
						break;

					case SpotLightType:
						Renderer::DrawQuad(position, color, scale, m_SpotLightIconTexture);
						break;

					case SpriteType:
						Renderer::DrawQuad(position, color, scale, m_SpriteIconTexture);
						break;

					case LitSpriteType:
						Renderer::DrawQuad(position, color, scale, m_LitSpriteIconTexture);
						break;

					case ModelType:
						Renderer::DrawQuad(position, color, scale, m_MeshIconTexture);
						break;
					}
				}
			}

			m_Exporter.DrawOutline();
		}
		Renderer::EndScene();
		//revert to the scene rendering mode
		Renderer::SetBlendMode(m_Env->BlendMode);
	}

	void Editor::DrawUI()
	{
		DisplayMainMenuBarGUI();
		AssetManager::DisplayGUI();
		DisplayEnvironmentControlsGUI();
		DisplayObjectInspecterGUI();
		DisplayProfilerGUI();
		DisplayPreferencesGUI();
		DisplayPropertiesGUI();
		m_AppStatusWindow.DisplayGUI();

		//Settings window
		if (m_EnvironmentSettingsWindowOpen)
		{
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

							Renderer::SetBlendMode(m_Env->BlendMode);
						}
					}

					{
						bool is_Active = m_Env->BlendMode == RenderingBlendMode::Screen;
						if (ImGui::Selectable("Screen", &is_Active)) {

							ImGui::SetItemDefaultFocus();
							m_Env->BlendMode = RenderingBlendMode::Screen;

							Renderer::SetBlendMode(m_Env->BlendMode);
						}
					}
					ImGui::EndCombo();

				}

				ImGui::TreePop();
			}
			ImGui::Text("Show Object Icons");
			ImGui::SameLine();
			ImGui::SetCursorPosX(125.0f);
			ImGui::Checkbox("##Show Object Icons", &m_ShowObjectIcons);

			ImGui::Text("Show Grid");
			ImGui::SameLine();
			ImGui::SetCursorPosX(100.0f);
			ImGui::Checkbox("##Show Grid", &m_ShowGrid);
			if (m_ShowGrid)
			{
				ImGui::Text("Grid Orientation");
				ImGui::SameLine();
				ImGui::SetCursorPosX(100.0f);

				if (ImGui::BeginCombo("##Grid Orientation", Grid::GridPlaneToStr(m_Grid.Orientation)))
				{
					{
						bool selected = m_Grid.Orientation == Grid::XY;
						if (ImGui::Selectable(Grid::GridPlaneToStr(Grid::XY), &selected))
							m_Grid.Orientation = Grid::XY;
					}

					{
						bool selected = m_Grid.Orientation == Grid::XZ;
						if (ImGui::Selectable(Grid::GridPlaneToStr(Grid::XZ), &selected))
							m_Grid.Orientation = Grid::XZ;
					}

					{
						bool selected = m_Grid.Orientation == Grid::YZ;
						if (ImGui::Selectable(Grid::GridPlaneToStr(Grid::YZ), &selected))
							m_Grid.Orientation = Grid::YZ;
					}

					ImGui::EndCombo();
				}
			}

			ImGui::Text("Blur");
			ImGui::SameLine();
			ImGui::SetCursorPosX(100.0f);
			ImGui::Checkbox("##Blur", &m_Env->BlurEnabled);

			if (m_Env->BlurEnabled)
			{
				if (ImGui::TreeNode("Blur Settings: "))
				{
					ImGui::Text("Blur Radius: ");
					ImGui::SameLine();
					ImGui::DragFloat("##Blur Radius: ", &m_Env->BlurRadius, 0.01f, 0.0f, 5.0f);

					ImGui::TreePop();
				}
			}
			ImGui::End();
		}

		m_Exporter.DisplayGUI();
		InputManager::DisplayGUI();
		m_ViewportWindow.DisplayGUI(m_RenderSurface.SurfaceFramebuffer);

		static glm::mat4 cube(1.0f);
		if (m_Camera.Mode == ProjectionMode::Orthographic)
			ImGuizmo::SetOrthographic(true);
		else
			ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetRect(m_ViewportWindow.WindowPosition.x, m_ViewportWindow.WindowPosition.y, m_ViewportWindow.WindowSize.x, m_ViewportWindow.WindowSize.y);

		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetDrawlist(ImGui::GetOverlayDrawList());

		if (m_State == EditorState::State_EditorMode)
		{
			for (auto& obj : m_Env->Objects)
			{
				if (obj->Selected)
				{
					ImGuizmo::Manipulate(glm::value_ptr(m_Camera.ViewMatrix), glm::value_ptr(m_Camera.ProjectionMatrix),
						(ImGuizmo::OPERATION)obj->GetAllowedGizmoOperation(m_GizmoOperation), ImGuizmo::MODE::WORLD, glm::value_ptr(obj->ModelMatrix));
					break;
				}
			}
		}
	}

	void Editor::OnEnvironmentLoad()
	{
		AssetManager::Init(m_EnvironmentFolderPath.u8string());

		if (m_Preferences.WindowMaximized)
			Window::Maximize();
		else
			Window::SetSize(m_Preferences.WindowSize);

		Window::WindowSizeChangedSinceLastFrame = true;
		m_RenderSurface.SetSize(Window::FramebufferSize);
		RegisterEnvironmentInputKeys();

		std::memset(m_DeltaTimeHistory.data(), 0, m_DeltaTimeHistory.size() * sizeof(float));

		UpdateTitle();
	}

	void Editor::OnEnvironmentDestroy()
	{
		AssetManager::Terminate();
		Window::Restore();
		Window::SetSize(c_StartMenuWidth, c_StartMenuHeight);
		Window::CenterWindow();
		m_ShouldDeleteEnv = true;
	}

	void Editor::DisplayMainMenuBarGUI()
	{
		if (ImGui::BeginMainMenuBar()) 
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save")) 
				{
					std::string name = m_EnvironmentFolderPath.u8string() + "\\" + m_Env->Name + ".env";
					SaveEnvironment(*m_Env, name);
					m_AppStatusWindow.SetText("Saved Environment To: " + name);
				}

				if (ImGui::MenuItem("Close Environment"))
				{
					OnEnvironmentDestroy();
					m_State = State_NoEnvLoaded;
				}

				if (ImGui::MenuItem("Preferences"))
					m_PreferencesWindowOpen = true;

				if (ImGui::MenuItem("Exit"))
					Window::SetShouldClose();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit")) {

				if (ImGui::MenuItem("Delete All Objects"))
					m_Env->Objects.clear();


				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Window")) 
			{
				ImGui::MenuItem("Properties", nullptr, &m_PropertiesWindowOpen);
				ImGui::MenuItem("Environment Controls", nullptr, &m_EnvironmentControlsWindowOpen);
				ImGui::MenuItem("Object Inspector", nullptr, &m_ObjectInspectorWindowOpen);
				ImGui::MenuItem("Settings", nullptr, &m_EnvironmentSettingsWindowOpen);
				ImGui::MenuItem("Profiler", nullptr, &m_ProfilerWindowOpen);
				ImGui::MenuItem("Exporter", nullptr, &m_Exporter.m_ExporterWindowOpen);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help")) 
			{
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
			if (ImGui::ImageButton((void*)m_PlayButtonTexture.GetTextureID(),
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
			if (ImGui::ImageButton((void*)m_StopButtonTexture.GetTextureID(),
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
			if (ImGui::ImageButton((void*)m_PauseButtonTexture.GetTextureID(),
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

		ImGui::Text("Simulation Speed: ");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Determines how fast time moves in the environment\n1 means 1x realtime and so on");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(75.0f);
		ImGui::DragFloat("##Simulation Speed", &m_SimulationSpeedFactor, 0.1f, 0.1f, 10.0f, "%.1fx");
		m_SimulationSpeedFactor = std::clamp(m_SimulationSpeedFactor, 0.1f, 10.0f);
		
		ImGui::SameLine();

		if (ImGui::Button("Export"))
			m_Exporter.OpenExporterWindow();

		ImGui::SameLine(ImGui::GetWindowSize().x - 250);
		ImGui::RadioButton("Orthographic", (int32_t*)&m_Camera.Mode, (int32_t)ProjectionMode::Orthographic);
		ImGui::SameLine();
		ImGui::RadioButton("Perspective", (int32_t*)&m_Camera.Mode, (int32_t)ProjectionMode::Perspective);

		Renderer::RegisterWindowThatCanCoverViewport();
		ImGui::End();
	}

	void Editor::DisplayPropertiesGUI()
	{
		if (m_PropertiesWindowOpen == false)
			return;

		ImGui::Begin("Properties", &m_PropertiesWindowOpen);

		//find selected object
		pEnvironmentObject* selectedObj = nullptr;
		for (auto& obj : m_Env->Objects)
		{
			if (obj->Selected)
			{
				selectedObj = &obj;
				break;
			}
		}

		//render controls if there is an object selected
		if (selectedObj != nullptr)
		{
			ImGui::Text(selectedObj->get()->m_Name.c_str());
			ImGui::SameLine(ImGui::GetWindowSize().x - 100);
			ImGui::Text(EnvironmentObjectTypeToString(selectedObj->get()->Type).c_str());
			ImGui::Separator();
			ImGui::Spacing();
			selectedObj->get()->DisplayGuiControls();
		}

		ImGui::End();
	}

	void Editor::Stop()
	{
		m_State = State_EditorMode;

		for (pEnvironmentObject& obj : m_Env->Objects) 
		{
			auto mutexPtr = obj->GetMutex();
			std::lock_guard lock(*mutexPtr);
			if (obj->Type == EnvironmentObjectType::ParticleSystemType) 
			{
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

		auto flags = ImGuiWindowFlags_AlwaysUseWindowPadding;
		ImGui::Begin("Object Inspector", &m_ObjectInspectorWindowOpen, flags);

		if (ImGui::Button("Add Object"))
			m_AddObjectWindowOpen = true;

		ImGui::Spacing();

		ImGui::PushItemWidth(ImGui::GetWindowWidth() - 30);
		if (ImGui::ListBoxHeader("##Inspector", -1)) {

			for (int i = 0; i < m_Env->Objects.size(); i++)
			{
				auto mutexPtr = m_Env->Objects[i]->GetMutex();
				std::lock_guard lock(*mutexPtr);
				ImGui::PushID(m_Env->Objects[i].get());

				//the vertical size of each row
				const int32_t elementSize = 25;
				
				if (ImGui::Selectable("##object", &m_Env->Objects[i]->Selected, 0, ImVec2(0, elementSize)))
				{
					//if this is selected. deselect all other particle systems
					for (auto& particle : m_Env->Objects) {
						if (particle.get() != m_Env->Objects[i].get())
							particle->Selected = false;
					}
				}
				//show menu when right clicking
				if (ImGui::BeginPopupContextItem("Object Popup"))
				{
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

				ImTextureID icon = nullptr;

				//choose icon that displays the object type
				switch (m_Env->Objects[i]->Type)
				{
				case SpriteType:
					icon = (void*)m_SpriteIconTexture.GetTextureID();
					break;

				case LitSpriteType:
					icon = (void*)m_LitSpriteIconTexture.GetTextureID();
					break;

				case ParticleSystemType:
					icon = (void*)m_ParticleSystemIconTexture.GetTextureID();
					break;

				case RadialLightType:
					icon = (void*)m_RadialLightIconTexture.GetTextureID();
					break;

				case SpotLightType:
					icon = (void*)m_SpotLightIconTexture.GetTextureID();
					break;

				case ModelType:
					icon = (void*)m_MeshIconTexture.GetTextureID();
					break;
				}
				//display the image in the same line
				ImGui::SameLine();
				ImGui::Image(icon, ImVec2(25, elementSize), { 0, 1 }, { 1, 0 });
				//add tooltip for the image
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip(EnvironmentObjectTypeToString(m_Env->Objects[i]->Type).c_str());

				//add text displaying the objects name to the right of the image
				ImGui::SameLine();
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetItemRectSize().y / 4.0f);
				ImGui::Text(m_Env->Objects[i]->m_Name.c_str());

				ImGui::Spacing();

				if (m_Env->Objects[i]->RenameTextOpen) 
				{
					if (ImGui::InputText("Name", &m_Env->Objects[i]->m_Name, ImGuiInputTextFlags_EnterReturnsTrue))
					{
						m_Env->Objects[i]->RenameTextOpen = !m_Env->Objects[i]->RenameTextOpen;
					}
				}

				ImGui::PopID();
			}

			ImGui::ListBoxFooter();
		}
		Renderer::RegisterWindowThatCanCoverViewport();
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
				bool selected = m_AddObjectWindowObjectType == LitSpriteType;
				if (ImGui::Selectable(EnvironmentObjectTypeToString(LitSpriteType).c_str(), &selected))
					m_AddObjectWindowObjectType = LitSpriteType;
			}

			{
				bool selected = m_AddObjectWindowObjectType == ModelType;
				if (ImGui::Selectable(EnvironmentObjectTypeToString(ModelType).c_str(), &selected))
					m_AddObjectWindowObjectType = ModelType;
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

		Renderer::RegisterWindowThatCanCoverViewport();
		ImGui::End();
	}

	void Editor::RefreshObjectOrdering()
	{
		for (size_t i = 0; i < m_Env->Objects.size(); i++) 
		{
			auto mutexPtr = m_Env->Objects[i]->GetMutex();
			std::lock_guard lock(*mutexPtr);
			m_Env->Objects[i]->Order = i;
		}
	}

	void Editor::Duplicate(EnvironmentObjectInterface& obj)
	{
		//if this object is a particle system
		if (obj.Type == EnvironmentObjectType::ParticleSystemType)
		{
			//make a new particle system
			m_Env->Objects.push_back(std::make_unique<ParticleSystem>(*static_cast<ParticleSystem*>(&obj)));

			//add a -copy to the name of the new particle system to indicate that it was copied
			auto mutexPtr2 = m_Env->Objects[m_Env->Objects.size() - 1]->GetMutex();
			std::lock_guard lock(*mutexPtr2);
			m_Env->Objects[m_Env->Objects.size() - 1]->m_Name += "-copy";
		}

		//if this object is a radial light
		else if (obj.Type == EnvironmentObjectType::RadialLightType)
		{
			//make a new radial light
			m_Env->Objects.push_back(std::make_unique<RadialLight>(*static_cast<RadialLight*>(&obj)));

			//add a -copy to the name of the new light to indicate that it was copied
			auto mutexPtr2 = m_Env->Objects[m_Env->Objects.size() - 1]->GetMutex();
			std::lock_guard lock(*mutexPtr2);
			m_Env->Objects[m_Env->Objects.size() - 1]->m_Name += "-copy";
		}

		//if this object is a spot light
		else if (obj.Type == EnvironmentObjectType::SpotLightType)
		{
			//make a new radial light
			m_Env->Objects.push_back(std::make_unique<SpotLight>(*static_cast<SpotLight*>(&obj)));

			//add a -copy to the name of the new light to indicate that it was copied
			auto mutexPtr2 = m_Env->Objects[m_Env->Objects.size() - 1]->GetMutex();
			std::lock_guard lock(*mutexPtr2);
			m_Env->Objects[m_Env->Objects.size() - 1]->m_Name += "-copy";
		}

		//if this object is a Sprite
		else if (obj.Type == EnvironmentObjectType::SpriteType)
		{
			//make a new radial light
			m_Env->Objects.push_back(std::make_unique<Sprite>(*static_cast<Sprite*>(&obj)));

			//add a -copy to the name of the new light to indicate that it was copied
			auto mutexPtr2 = m_Env->Objects[m_Env->Objects.size() - 1]->GetMutex();
			std::lock_guard lock(*mutexPtr2);
			m_Env->Objects[m_Env->Objects.size() - 1]->m_Name += "-copy";
		}
	}

	void Editor::FocusCameraOnObject(EnvironmentObjectInterface& object)
	{
		m_Camera.SetPosition(glm::vec3(object.ModelMatrix[3][0], object.ModelMatrix[3][1], m_Camera.Position.z));
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
			break;
		}

		case SpriteType:
		{
			auto sprite = std::make_unique<Sprite>();
			obj.reset(((EnvironmentObjectInterface*)(sprite.release())));
			break;
		}

		case ModelType:
		{
			auto mesh = std::make_unique<Model>();
			obj.reset(((EnvironmentObjectInterface*)(mesh.release())));
			break;
		}

		case LitSpriteType:
		{
			auto sprite = std::make_unique<LitSprite>();
			obj.reset(((EnvironmentObjectInterface*)(sprite.release())));
			break;
		}

		case RadialLightType:
		{
			auto light = std::make_unique<RadialLight>();
			obj.reset(((EnvironmentObjectInterface*)(light.release())));
			break;
		}

		case SpotLightType:
		{
			auto light = std::make_unique<SpotLight>();
			obj.reset(((EnvironmentObjectInterface*)(light.release())));
			break;
		}
		}

		obj->m_Name = name;

		//display text that we created the object (for 2 seconds)
		m_AppStatusWindow.SetText("Created Object : \"" + obj->m_Name + '"' + " of Type : \"" + EnvironmentObjectTypeToString(obj->Type) + '"', 2.0f);

		//add the object to the list of the environment objects
		m_Env->Objects.push_back(std::move(obj));

		RefreshObjectOrdering();
		m_GPUMemAllocated = Renderer::GetUsedGPUMemory();
	}

	void Editor::RegisterEnvironmentInputKeys()
	{
		InputManager::RegisterKey(GLFW_KEY_F5, "PlayMode/Resume", [this](int32_t mods) 
			{
				if (m_State == State_EditorMode)
					PlayMode();
				if (m_State == State_PauseMode)
					Resume();
			});

		InputManager::RegisterKey(GLFW_KEY_S, "Save Project (while CONTROL key down)", [this](int32_t mods)
			{
				if (mods & GLFW_MOD_CONTROL)
				{
					std::string name = m_EnvironmentFolderPath.u8string() + "\\" + m_Env->Name + ".env";
					SaveEnvironment(*m_Env, name);
					m_AppStatusWindow.SetText("Saved Environment To: " + name);
				}
			});

		InputManager::RegisterKey(GLFW_KEY_Z, "Set Gizmo to Translate Mode", [this](int32_t mods)
			{
				m_GizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
			});

		InputManager::RegisterKey(GLFW_KEY_X, "Set Gizmo to Roation Mode", [this](int32_t mods)
			{
				m_GizmoOperation = ImGuizmo::OPERATION::ROTATE;
			});

		InputManager::RegisterKey(GLFW_KEY_C, "Set Gizmo to Scale Mode", [this](int32_t mods)
			{
				m_GizmoOperation = ImGuizmo::OPERATION::SCALE;
			});

		InputManager::RegisterKey(GLFW_KEY_R, "Clear All Particles", [this](int32_t mods)
			{
				if ((mods & GLFW_MOD_CONTROL) == false)
					return;

				for (pEnvironmentObject& obj : m_Env->Objects)
				{
					auto mutexPtr = obj->GetMutex();
					std::lock_guard lock(*mutexPtr);
					if (obj->Type == EnvironmentObjectType::ParticleSystemType)
					{
						ParticleSystem* ps = static_cast<ParticleSystem*>(obj.get());
						ps->ClearParticles();
					}
				}

				//reset camera
				m_Camera.Position = c_CameraStartingPosition;
				m_Camera.CameraForward = c_CameraStartingForwardDirection;
				CameraPitch = 0.0f;
				CameraYaw = 90.0f;
				m_Camera.CalculateMatrices();
			});

		//shortcut cut to use in all the mapped buttons
		//it displays the camera position in the status window(the blue coloured stripe at the bottom)
		auto displayCameraPosFunc = [this]()
		{
			std::stringstream messageString;

			messageString << std::fixed << std::setprecision(2);
			messageString << "Moving Camera to Coordinates :";
			messageString << "(" <<   m_Camera.Position.x;
			messageString << ", " <<  m_Camera.Position.y;
			messageString << ", " <<  m_Camera.Position.z << ")";

			m_AppStatusWindow.SetText(messageString.str());
		};

		//map WASD keys to move the camera in the environment
		InputManager::RegisterKey(GLFW_KEY_W, "Move Camera", [this, displayCameraPosFunc](int32_t mods)
			{
				//we don't want to zoom if the focus is not set on the viewport or if a mod key like CONTROL is down
				if (m_ViewportWindow.IsFocused == false || mods != 0)
					return;

				//move the camera's position
				if (m_Camera.Mode == ProjectionMode::Orthographic)
				{
					glm::vec3 cameraRight = glm::normalize(glm::cross(m_Camera.CameraUp, m_Camera.CameraForward));
					glm::vec3 cameraUp = glm::cross(m_Camera.CameraForward, cameraRight);
					m_Camera.SetPosition(m_Camera.Position + cameraUp * m_Camera.ZoomFactor * (float)LastFrameDeltaTime * c_CameraOrthoMoveSpeedFactor);
				}
				else
					m_Camera.SetPosition(m_Camera.Position + m_Camera.CameraForward * (float)LastFrameDeltaTime * c_CameraPerspMoveSpeedFactor);

				//display text in the bottom right of the screen stating the new position of the camera
				displayCameraPosFunc();
			},
			//set mode as repeat because we want the camera to move smoothly
				GLFW_REPEAT);

		//the rest are the same with only a diffrent move direction, that is why they arent commented
		InputManager::RegisterKey(GLFW_KEY_S, "Move Camera", [this, displayCameraPosFunc](int32_t mods)
			{
				if (m_ViewportWindow.IsFocused == false || mods != 0)
					return;
				if (m_Camera.Mode == ProjectionMode::Orthographic)
				{
					glm::vec3 cameraRight = glm::normalize(glm::cross(m_Camera.CameraUp, m_Camera.CameraForward));
					glm::vec3 cameraDown = -glm::cross(m_Camera.CameraForward, cameraRight);

					m_Camera.SetPosition(m_Camera.Position + cameraDown * m_Camera.ZoomFactor * (float)LastFrameDeltaTime * c_CameraOrthoMoveSpeedFactor);
				}
				else
					m_Camera.SetPosition(m_Camera.Position - m_Camera.CameraForward * (float)LastFrameDeltaTime * c_CameraPerspMoveSpeedFactor);
				displayCameraPosFunc();
			},
			GLFW_REPEAT);

		InputManager::RegisterKey(GLFW_KEY_D, "Move Camera", [this, displayCameraPosFunc](int32_t mods)
			{
				if (m_ViewportWindow.IsFocused == false || mods != 0)
					return;
				glm::vec3 cameraRight = glm::normalize(glm::cross(m_Camera.CameraUp, m_Camera.CameraForward));

				if (m_Camera.Mode == ProjectionMode::Orthographic)
					m_Camera.SetPosition(m_Camera.Position + cameraRight * m_Camera.ZoomFactor * (float)LastFrameDeltaTime * c_CameraOrthoMoveSpeedFactor);
				else
				{
					m_Camera.SetPosition(m_Camera.Position + cameraRight * (float)LastFrameDeltaTime * c_CameraPerspMoveSpeedFactor);
				}
				displayCameraPosFunc();
			},
			GLFW_REPEAT);

		InputManager::RegisterKey(GLFW_KEY_A, "Move Camera", [this, displayCameraPosFunc](int32_t mods)
			{
				if (m_ViewportWindow.IsFocused == false || mods != 0)
					return;
				glm::vec3 cameraLeft = -glm::normalize(glm::cross(m_Camera.CameraUp, m_Camera.CameraForward));

				if (m_Camera.Mode == ProjectionMode::Orthographic)
					m_Camera.SetPosition(m_Camera.Position + cameraLeft * m_Camera.ZoomFactor * (float)LastFrameDeltaTime * c_CameraOrthoMoveSpeedFactor);
				else
				{
					m_Camera.SetPosition(m_Camera.Position + cameraLeft * (float)LastFrameDeltaTime * c_CameraPerspMoveSpeedFactor);
				}
				displayCameraPosFunc();
			},
			GLFW_REPEAT);

		InputManager::RegisterKey(GLFW_KEY_SPACE, "Move Camera", [this, displayCameraPosFunc](int32_t mods)
			{
				if (m_ViewportWindow.IsFocused == false || mods != 0)
					return;

				if (m_Camera.Mode != ProjectionMode::Perspective)
					return;

				m_Camera.SetPosition(m_Camera.Position + glm::vec3(0.0f, 1.0f, 0.0f) * (float)LastFrameDeltaTime * c_CameraPerspMoveSpeedFactor);

				displayCameraPosFunc();
			},
			GLFW_REPEAT);

		InputManager::RegisterKey(GLFW_KEY_LEFT_SHIFT, "Move Camera", [this, displayCameraPosFunc](int32_t mods)
			{
				if (m_ViewportWindow.IsFocused == false)
					return;

				if (m_Camera.Mode != ProjectionMode::Perspective)
					return;

				m_Camera.SetPosition(m_Camera.Position + glm::vec3(0.0f, -1.0f, 0.0f) * (float)LastFrameDeltaTime * c_CameraPerspMoveSpeedFactor);

				displayCameraPosFunc();
			},
			GLFW_REPEAT);

		//delete object keyboard shortcut
		InputManager::RegisterKey(GLFW_KEY_DELETE, "Delete Object", [this](int32_t mods)
			{
				for (int i = 0; i < m_Env->Objects.size(); i++)
				{
					auto mutexPtr = m_Env->Objects[i]->GetMutex();
					std::lock_guard lock(*mutexPtr);
					if (m_Env->Objects[i]->Selected)
					{
						m_Env->Objects[i]->ToBeDeleted = true;
						break;
					}
				}
			});

		//zoom in and out with mouse scroll wheel
		InputManager::m_ScrollHandlers.push_back([this](double xoffset, double yoffset)
			{
				//we don't want to zoom if the focus is not set on the viewport
				if (m_ViewportWindow.IsHovered == false || m_ViewportWindow.IsFocused == false || m_Camera.Mode == ProjectionMode::Perspective)
					return;

				//change zoom factor
				m_Camera.ZoomFactor -= yoffset * 2.0f;
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

		InputManager::RegisterMouseKey(GLFW_MOUSE_BUTTON_MIDDLE, "Change Camera Zoom to Default", [this](int32_t mods)
			{
				//we don't want to zoom if the focus is not set on the viewport
				if (m_ViewportWindow.IsHovered == false)
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

		InputManager::RegisterMouseKey(GLFW_MOUSE_BUTTON_RIGHT, "Hold to rotate camera while in Perspective mode", [this](int32_t mods)
			{
				if (m_ViewportWindow.IsHovered && m_Camera.Mode == ProjectionMode::Perspective && (InputManager::MouseDelta.x != 0 || InputManager::MouseDelta.y != 0))
				{
					CameraYaw -= LastFrameDeltaTime * InputManager::MouseDelta.x * 50.f;
					CameraPitch -= LastFrameDeltaTime * InputManager::MouseDelta.y * 50.f;
					if (CameraPitch > 89.0f)
						CameraPitch = 89.0f;

					glm::vec3 direction;
					direction.x = cos(glm::radians(CameraYaw)) * cos(glm::radians(CameraPitch));
					direction.y = sin(glm::radians(CameraPitch));
					direction.z = sin(glm::radians(CameraYaw)) * cos(glm::radians(CameraPitch));
					m_Camera.CameraForward = glm::normalize(direction);
					m_Camera.CalculateMatrices();
				}
			});
	}

	void Editor::DisplayProfilerGUI()
	{
		if (!m_ProfilerWindowOpen || m_State != State_PlayMode)
			return;

		ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
		ImGui::Begin("Profiler", &m_ProfilerWindowOpen, ImGuiWindowFlags_NoDocking);

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
			ImGui::TextColored({ 0.0f,0.8f,0.0f,1.0f }, std::to_string(m_DrawCalls).c_str());
			ImGui::SameLine();

			//update framerate every 30 frames
			static int32_t frameCounter = 1;
			if (frameCounter % 30 == 0)
			{
				//calculate fps when we dont have uninitilized values
				if (std::find(m_DeltaTimeHistory.begin(), m_DeltaTimeHistory.end(), 0.0f) == m_DeltaTimeHistory.end())
					m_AverageFPS = std::round(1.0f / (std::accumulate(m_DeltaTimeHistory.begin(), m_DeltaTimeHistory.end(), 0.0f) / m_DeltaTimeHistory.size()));
				else
					m_AverageFPS = 0;
				frameCounter = 1;
			}
			frameCounter++;

			ImGui::Text("        FPS: ");
			ImGui::SameLine();
			ImGui::TextColored({ 0.0f,0.8f,0.0f,1.0f }, std::to_string(m_AverageFPS).c_str());

			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::SetTooltip("NOTE: Frame rates do not exceed 60,\nthis is theoretical FPS given the time per frame");
				ImGui::EndTooltip();
			}

			ImGui::PlotLines("Frame Time(s)", m_DeltaTimeHistory.data(), m_DeltaTimeHistory.size(),
				0, 0, 0.0f, 0.025f, ImVec2(0, 50));

			ImGui::Text("Textures: ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(Renderer::Rdata->Textures.size()).c_str());

			ImGui::SameLine();
			ImGui::Text("   VBO(s): ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(Renderer::Rdata->VertexBuffers.size()).c_str());

			ImGui::SameLine();
			ImGui::Text("   EBO(s): ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(Renderer::Rdata->IndexBuffers.size()).c_str());

			ImGui::SameLine();
			ImGui::Text("   UBO(s): ");
			ImGui::SameLine();
			ImGui::Text(std::to_string(Renderer::Rdata->UniformBuffers.size()).c_str());

			bool displayTooltip = false;
			ImGui::SameLine();
			ImGui::Text("   Used GPU Memory: ");
			displayTooltip |= ImGui::IsItemHovered();
			ImGui::SameLine();
			ImGui::Text(std::to_string(m_GPUMemAllocated / (1024 * 1024)).c_str());
			displayTooltip |= ImGui::IsItemHovered();
			ImGui::SameLine();
			ImGui::Text("Mb");
			displayTooltip |= ImGui::IsItemHovered();

			if(displayTooltip)
				ImGui::SetTooltip((std::to_string(m_GPUMemAllocated / (1024)) + " KB").c_str());
		}
		break;

		case Profiler::ParticleProfiler:
		{
			ImGui::Text("Global Particle Count :");
			ImGui::SameLine();

			uint32_t activeParticleCount = 0;
			for (pEnvironmentObject& obj : m_Env->Objects)
			{
				auto mutexPtr = obj->GetMutex();
				std::lock_guard lock(*mutexPtr);
				//if object is a particle system
				if (obj->Type == EnvironmentObjectType::ParticleSystemType)
				{
					//cast it to a particle system pointer
					ParticleSystem* ps = static_cast<ParticleSystem*>(obj.get());

					//increment active particles by how many particles are active in this particle system
					activeParticleCount += ps->ActiveParticleCount;
				}
			}
			ImGui::TextColored({ 0.0f, 1.0f, 0.0f, 1.0f }, std::to_string(activeParticleCount).c_str());

			ImGui::Separator();

			for (pEnvironmentObject& pso : m_Env->Objects)
			{
				auto mutexPtr = pso->GetMutex();
				std::lock_guard lock(*mutexPtr);
				if (pso->Type == EnvironmentObjectType::ParticleSystemType)
				{
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

		Renderer::RegisterWindowThatCanCoverViewport();
		ImGui::End();
	}

	void Editor::DisplayPreferencesGUI()
	{
		if (m_PreferencesWindowOpen == false)
			return;

		ImGui::Begin("Preferences", &m_PreferencesWindowOpen);

		ImGui::Checkbox("Maximize Editor On Load", &m_Preferences.WindowMaximized);

		if (ImGui::BeginCombo("Editor Style", EditorStyleStr(m_Preferences.Style).c_str()))
		{
			auto style = EditorStyle::DarkTransparent;
			bool selected = m_Preferences.Style == style;
			if (ImGui::Selectable(EditorStyleStr(style).c_str(), &selected))
			{
				m_Preferences.Style = style;
				SetEditorStyle(style);
			}

			style = EditorStyle::Dark_Gray;
			selected = m_Preferences.Style == style;
			if (ImGui::Selectable(EditorStyleStr(style).c_str(), &selected))
			{
				m_Preferences.Style = style;
				SetEditorStyle(style);
			}

			style = EditorStyle::Dark;
			selected = m_Preferences.Style == style;
			if (ImGui::Selectable(EditorStyleStr(style).c_str(), &selected))
			{
				m_Preferences.Style = style;
				SetEditorStyle(style);
			}

			style = EditorStyle::Light;
			selected = m_Preferences.Style == style;
			if (ImGui::Selectable(EditorStyleStr(style).c_str(), &selected))
			{
				m_Preferences.Style = style;
				SetEditorStyle(style);
			}

			style = EditorStyle::Classic;
			selected = m_Preferences.Style == style;
			if (ImGui::Selectable(EditorStyleStr(style).c_str(), &selected))
			{
				m_Preferences.Style = style;
				SetEditorStyle(style);
			}
			
			ImGui::EndCombo();
		}

		if (ImGui::BeginCombo("Backend", RendererTypeStr(m_Preferences.RenderingBackend).c_str()))
		{
			auto backend = RendererType::OpenGL;
			auto selected = m_Preferences.RenderingBackend == backend;

			if (ImGui::Selectable(RendererTypeStr(backend).c_str(), &selected))
				m_Preferences.RenderingBackend = backend;

#ifdef PLATFORM_WINDOWS
			backend = RendererType::D3D11;
			selected = m_Preferences.RenderingBackend == backend;
			if (ImGui::Selectable(RendererTypeStr(backend).c_str(), &selected))
				m_Preferences.RenderingBackend = backend;
#endif // PLATFORM_WINDOWS

			ImGui::EndCombo();
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Backend will change only when the app is restarted");

		Renderer::RegisterWindowThatCanCoverViewport();
		ImGui::End();
	}

	void Editor::UpdateTitle()
	{
		RendererType currentRendererType = Renderer::Rdata->API;

		std::string environmentName = "";
		if (m_Env)
			environmentName = m_Env->Name;
		else
			environmentName = "Start Menu";

		Window::SetTitle("Ainan - " + RendererTypeStr(currentRendererType) + " - " + environmentName);
	}
}
