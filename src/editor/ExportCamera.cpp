#include <pch.h>

#include "ExportCamera.h"

namespace Ainan {

	//TODO use environment directory instead of default
	ExportCamera::ExportCamera() :
		m_ImageLocationBrowser(STARTING_BROWSER_DIRECTORY, "Save Image"), 
		RealCamera(CameraMode::CentreIsMidPoint)
	{
		memset(m_Edges, 0, sizeof(m_Edges));

		ImageSavePath = STARTING_BROWSER_DIRECTORY;
		SetSize();
	}

	void ExportCamera::DrawOutline()
	{
		if (!m_DrawExportCamera)
			return;

		m_Outline.Draw();
	}

	void ExportCamera::SetSize()
	{
		glm::vec2 size = glm::vec2(RealCamera.ZoomFactor * m_AspectRatio, RealCamera.ZoomFactor) / c_GlobalScaleFactor;
		m_Edges[0] = m_ExportCameraPosition - (size / 2.0f); //bottom left
		m_Edges[1] = m_ExportCameraPosition + glm::vec2(-size.x, size.y) / 2.0f; //top left
		m_Edges[2] = m_ExportCameraPosition + (size / 2.0f);     //top right
		m_Edges[3] = m_ExportCameraPosition + glm::vec2(size.x, -size.y) / 2.0f; //bottom right

		std::array<glm::vec2, 8> vertices =
		{
			m_Edges[0], m_Edges[1], //bottom left to top left
			m_Edges[1], m_Edges[2], //top left to top right
			m_Edges[2], m_Edges[3], //top right to bottom right
			m_Edges[3], m_Edges[0]  //bottom right to bottom left
		};
		m_Outline.SetVertices(std::vector<glm::vec2>(vertices.begin(), vertices.end()));

		RealCamera.Update(0.0f, Renderer::GetCurrentViewport());
		glm::vec2 reversedPos = glm::vec2(-m_ExportCameraPosition.x, -m_ExportCameraPosition.y);

		RealCamera.SetPosition(reversedPos * c_GlobalScaleFactor);
	}

	void ExportCamera::DisplayGUI()
	{
		if (SettingsWindowOpen) {

			ImGui::PushID(this);

			ImGui::Begin("ExportMode Settings", &SettingsWindowOpen);

			if (ImGui::TreeNode("ExportMode Camera Settings"))
			{
				ImGui::Text("Draw ExportMode Camera Outline");
				ImGui::SameLine();
				ImGui::Checkbox("##Draw ExportMode Camera Outline", &m_DrawExportCamera);

				//update position if any of these are changed, using SetSize().

				ImGui::Text("Position");
				ImGui::SameLine();
				ImGui::SetCursorPosX(100);
				if (ImGui::DragFloat2("##Position", &m_ExportCameraPosition.x, 0.01f))
					SetSize();
				ImGui::Text("Size");
				ImGui::SameLine();
				ImGui::SetCursorPosX(100);

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Image Saving:"))
			{
				if (ImGui::DragFloat("Aspect Ratio", &m_AspectRatio, 0.001f, 0.0f, 1000.0f))
					SetSize();
				if (ImGui::DragFloat("Zoom Factor", &RealCamera.ZoomFactor, 1.0f, 0.0f, 5000.0f))
					SetSize();

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Image Capture Settings:"))
			{
				ImGui::Text("Capture After :");

				bool textHovered = ImGui::IsItemHovered();

				ImGui::SameLine();
				ImGui::PushItemWidth(150);
				if (ImGui::DragFloat("##Capture After :", &ImageCaptureTime, 0.1f))
					ImageCaptureTime = std::clamp(ImageCaptureTime, 0.0f, 3600.0f);

				bool dragFloatHovered = ImGui::IsItemHovered();

				if (textHovered || dragFloatHovered) {
					ImGui::BeginTooltip();
					ImGui::Text("This specifies how many seconds since \nplay mode started before we capture an \nimage and export it if we are running \nin export mode");
					ImGui::EndTooltip();
				}

				ImGui::TreePop();
			}

			ImGui::End();

			m_ImageLocationBrowser.DisplayGUI([this](const std::string& path) 
				{
				ImageSavePath = path;
				m_ImageLocationBrowser.CloseWindow();
				});
		}

		if (m_FinalizeExportWindowOpen) 
		{
			ImGui::Begin("Finalize Export", &m_FinalizeExportWindowOpen);
			ImGui::Columns(2);

			ImGui::Text("Image Format");
			ImGui::SameLine();
			if (ImGui::BeginCombo("##Image Format", Image::GetFormatString(SaveImageFormat).c_str()))
			{
				bool is_png = SaveImageFormat == ImageFormat::png ? true : false;
				if (ImGui::Selectable(Image::GetFormatString(ImageFormat::png).c_str(), &is_png)) {

					ImGui::SetItemDefaultFocus();
					SaveImageFormat = ImageFormat::png;
				}

				bool is_jpeg = SaveImageFormat == ImageFormat::jpeg ? true : false;
				if (ImGui::Selectable(Image::GetFormatString(ImageFormat::jpeg).c_str(), &is_jpeg)) {

					ImGui::SetItemDefaultFocus();
					SaveImageFormat = ImageFormat::jpeg;
				}

				bool is_bmp = SaveImageFormat == ImageFormat::bmp ? true : false;
				if (ImGui::Selectable(Image::GetFormatString(ImageFormat::bmp).c_str(), &is_bmp)) {

					ImGui::SetItemDefaultFocus();
					SaveImageFormat = ImageFormat::bmp;
				}

				ImGui::EndCombo();
			}

			if (ImGui::Button("Save Location"))
				m_ImageLocationBrowser.OpenWindow();


			ImGui::Text("Selected Save Path: ");
			ImGui::SameLine();
			std::string saveTargetWithFormat = ImageSavePath;
			if (saveTargetWithFormat.back() == '\\')
				saveTargetWithFormat.append("default name");
			saveTargetWithFormat.append("." + Image::GetFormatString(SaveImageFormat));

			ImGui::TextColored({ 0.0f,0.8f,0.0f,1.0f }, saveTargetWithFormat.data());

			if (ImGui::Button("Save"))
			{
				std::string saveTarget = ImageSavePath;
				if(ImageSavePath.back() == '\\')
					saveTarget.append("default name");

				m_ExportTargetImage->SaveToFile(saveTarget, SaveImageFormat);
				m_FinalizeExportWindowOpen = false;
			}

			ImGui::NextColumn();

			if (m_ExportTargetTexture)
			{
				ImGui::Text("Preview");

				int scale = 250;
				ImVec2 size = ImVec2(scale * m_ExportTargetImage->m_Width / m_ExportTargetImage->m_Height, scale);
				ImGui::Image(m_ExportTargetTexture->GetTextureID(), size);
			}

			ImGui::End();
		}

		ImGui::PopID();
	}

	void ExportCamera::ExportFrame(Environment& env)
	{
		RealCamera.Update(0.0f, { 0,0,(int)Window::FramebufferSize.x,(int)Window::FramebufferSize.y });
		SceneDescription desc;
		desc.SceneCamera = RealCamera;
		desc.SceneDrawTarget = &m_RenderSurface.SurfaceFrameBuffer;
		desc.Blur = env.BlurEnabled;
		desc.BlurRadius = env.BlurRadius;
		Renderer::BeginScene(desc);

		m_RenderSurface.SetSize(glm::ivec2(RealCamera.ZoomFactor * m_AspectRatio, RealCamera.ZoomFactor));
		m_RenderSurface.SurfaceFrameBuffer->Bind();

		for (pEnvironmentObject& obj : env.Objects)
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

		m_Background.Draw(env);

		for (pEnvironmentObject& obj : env.Objects)
			obj->Draw();

		Renderer::EndScene();

		delete m_ExportTargetImage;

		m_ExportTargetImage = new Image(m_RenderSurface.SurfaceFrameBuffer->ReadPixels());

		if(Renderer::Rdata->CurrentActiveAPI->GetContext()->GetType() == RendererType::OpenGL)
			m_ExportTargetImage->FlipHorizontally();

		m_ExportTargetTexture = Renderer::CreateTexture(*m_ExportTargetImage);

		m_FinalizeExportWindowOpen = true;
	}
}