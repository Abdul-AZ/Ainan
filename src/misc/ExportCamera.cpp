#include <pch.h>

#include "ExportCamera.h"

namespace ALZ {

	constexpr const char* ExportModeToString(ExportCamera::ExportMode mode)
	{
		switch (mode)
		{
		case ExportCamera::SingleFrame:
			return "Single Frame Export";
			break;

		case ExportCamera::MultipleFramesAsSeperateImages:
			return "Multiple Frames As Seperate Images";
			break;

		default:
			assert(false);
			return "";
			break;
		}
	}

	ExportCamera::ExportCamera() :
		m_ImageLocationBrowser(FileManager::ApplicationFolder, "Save Image")
	{
		memset(m_Edges, 0, sizeof(m_Edges));
		memset(m_OutlineLines, 0, sizeof(m_OutlineLines));
		for (size_t i = 0; i < 4; i++)
			m_OutlineLines[i].Color = { 1.0f,1.0f,1.0f,1.0f };

		ImageSavePath = FileManager::ApplicationFolder + '\\';
		SetSize();
	}

	void ExportCamera::DrawOutline()
	{
		if (!m_DrawExportCamera)
			return;

		for (size_t i = 0; i < 4; i++)
			m_OutlineLines[i].Draw();
	}

	void ExportCamera::SetSize()
	{
		m_Edges[0] = m_ExportCameraPosition;                                                                                          //bottom left
		m_Edges[1] = glm::vec2(m_ExportCameraPosition.x, m_ExportCameraPosition.y + m_ExportCameraSize.y);                            //top left
		m_Edges[2] = glm::vec2(m_ExportCameraPosition.x + m_ExportCameraSize.x, m_ExportCameraPosition.y + m_ExportCameraSize.y);     //top right
		m_Edges[3] = glm::vec2(m_ExportCameraPosition.x + m_ExportCameraSize.x, m_ExportCameraPosition.y);                            //bottom right

		m_OutlineLines[0].SetPoints(m_Edges[0], m_Edges[1]); //bottom left to top left
		m_OutlineLines[1].SetPoints(m_Edges[1], m_Edges[2]); //top left to top right
		m_OutlineLines[2].SetPoints(m_Edges[2], m_Edges[3]); //top right to bottom right
		m_OutlineLines[3].SetPoints(m_Edges[3], m_Edges[0]); //bottom right to bottom left

		RealCamera.Update(0.0f);
		glm::vec2 reversedPos = glm::vec2(-m_ExportCameraPosition.x, -m_ExportCameraPosition.y);
		RealCamera.SetPosition(reversedPos * GlobalScaleFactor);
	}

	void ExportCamera::DisplayGUI()
	{
		if (!SettingsWindowOpen)
			return;

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
			if (ImGui::DragFloat2("##Size", &m_ExportCameraSize.x, 0.01f))
				SetSize();

			//clamp the size, so it is always positive
			m_ExportCameraSize.x = std::clamp(m_ExportCameraSize.x, 0.0f, 100000.0f);
			m_ExportCameraSize.y = std::clamp(m_ExportCameraSize.y, 0.0f, 100000.0f);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Image Saving:"))
		{
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
		
			ImGui::Text("Image Resolution");
			ImGui::Text("Width");
			ImGui::SameLine();
			ImGui::SetCursorPosX(85);
			ImGui::TextColored(ImVec4(0.0f,0.8f,0.0f,1.0f), std::to_string(static_cast<int>(m_ExportCameraSize.x * GlobalScaleFactor)).c_str());
			ImGui::Text("Height");
			ImGui::SameLine();
			ImGui::SetCursorPosX(85);
			ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), std::to_string(static_cast<int>(m_ExportCameraSize.y * GlobalScaleFactor)).c_str());
		
			if (ImGui::Button("Save Location"))
				m_ImageLocationBrowser.OpenWindow();
		
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Image Capture Settings:")) 
		{
			ImGui::Text("Capture After :");

			bool textHovered = ImGui::IsItemHovered();

			ImGui::SameLine();
			ImGui::PushItemWidth(150);
			if(ImGui::DragFloat("##Capture After :", &ImageCaptureTime, 0.1f))
				ImageCaptureTime = std::clamp(ImageCaptureTime, 0.0f, 3600.0f);

			bool dragFloatHovered = ImGui::IsItemHovered();

			if(textHovered || dragFloatHovered) {
				ImGui::BeginTooltip();
				ImGui::Text("This specifies how many seconds since \nplay mode started before we capture an \nimage and export it if we are running \nin export mode");
				ImGui::EndTooltip();
			}

			ImGui::Text("Export Mode");
			ImGui::SameLine();
			ImGui::PushItemWidth(250);
			if (ImGui::BeginCombo("##Export Mode", ExportModeToString(m_ExportMode)))
			{
				//single frame capture
				{
					bool selected = m_ExportMode == SingleFrame;
					if (ImGui::Selectable(ExportModeToString(SingleFrame), &selected))
					{
						ImGui::SetItemDefaultFocus();
						m_ExportMode = SingleFrame;
					}
				}

				//multiple frame seperate images capture
				{
					bool selected = m_ExportMode == MultipleFramesAsSeperateImages;
					if (ImGui::Selectable(ExportModeToString(MultipleFramesAsSeperateImages), &selected))
					{
						ImGui::SetItemDefaultFocus();
						m_ExportMode = MultipleFramesAsSeperateImages;
					}
				}

				ImGui::EndCombo();
			}

			if (m_ExportMode == MultipleFramesAsSeperateImages) 
			{
				ImGui::Text("Time Between Captures");
				ImGui::SameLine();
				ImGui::PushItemWidth(75);
				ImGui::DragFloat("##Time Between Captures", &m_TimeBetweenCaptures, 0.01f);

				m_TimeBetweenCaptures = std::clamp(m_TimeBetweenCaptures, 0.01f, 10.0f);

				ImGui::Text("Number Of Frames");
				ImGui::SameLine();
				ImGui::PushItemWidth(75);
				ImGui::DragInt("##Number Of Frames", &m_CaptureFrameCount, 0.01f);

				m_CaptureFrameCount = std::clamp(m_CaptureFrameCount, 1, 20);
			}

			ImGui::TreePop();
		}

		ImGui::End();
		ImGui::PopID();

	    m_ImageLocationBrowser.DisplayGUI([this](const std::string& path) {
		ImageSavePath = path;
		m_ImageLocationBrowser.CloseWindow();
		});
	}

	void ExportCamera::Update(float deltaTime)
	{
		if(m_ExportMode == SingleFrame || (m_ExportMode == MultipleFramesAsSeperateImages && StartedMultiFrameExport))
			m_TimeSinceLastCapture += deltaTime;

		if (m_TimeSinceLastCapture > m_TimeBetweenCaptures && RemainingFramesToBeCaptured > 0)
		{
			NeedToExport = true;
			m_TimeSinceLastCapture = 0.0f;
		}

		if (m_ExportMode == MultipleFramesAsSeperateImages && RemainingFramesToBeCaptured <= 0)
			NeedToExport = false;

		ExportedEverything = RemainingFramesToBeCaptured <= 0;
		if (ExportedEverything)
			StartedMultiFrameExport = false;
	}

	void ExportCamera::StartExporting()
	{
		NeedToExport = true;

		if (m_ExportMode == MultipleFramesAsSeperateImages)
			StartedMultiFrameExport = true;
	}

	void ExportCamera::BeginExportScene() 
	{
		if (m_ExportMode == SingleFrame) {
			AlreadyExportedFrame = false;
		}
		else if (m_ExportMode == MultipleFramesAsSeperateImages) {
			RemainingFramesToBeCaptured = m_CaptureFrameCount;
		}
	}

	void ExportCamera::ExportFrame(Background& background, std::vector<Inspector_obj_ptr>& objects, float blurRadius)
	{
		Renderer::BeginScene(RealCamera);

		m_RenderSurface.SetSize(m_ExportCameraSize * GlobalScaleFactor);
		m_RenderSurface.SurfaceFrameBuffer->Bind();

		for (Inspector_obj_ptr& obj : objects)
		{
			if (obj->Type == InspectorObjectType::RadiaLightType) {
				RadialLight* light = static_cast<RadialLight*>(obj.get());
				background.SubmitLight(*light);
			}
		}

		background.Draw();

		for (Inspector_obj_ptr& obj : objects)
			obj->Draw();

		if (blurRadius > 0.0f)
			GaussianBlur::Blur(m_RenderSurface, blurRadius);

		Renderer::EndScene();

		Image image = Image::FromFrameBuffer(m_RenderSurface, m_RenderSurface.GetSize());

		std::string saveTarget = ImageSavePath;

		//add a default name if none is chosen
		if (saveTarget.back() == '\\')
			saveTarget.append("default name");

		//append a number at the end for each image if we are exporting multiple ones
		if (m_ExportMode == MultipleFramesAsSeperateImages)
		{
			saveTarget.append(std::to_string(m_CaptureFrameCount - RemainingFramesToBeCaptured));
		}

		image.SaveToFile(saveTarget, SaveImageFormat);

		NeedToExport = false;
		if (m_ExportMode == SingleFrame) {
			AlreadyExportedFrame = true;
		}

		RemainingFramesToBeCaptured--;
	}
}