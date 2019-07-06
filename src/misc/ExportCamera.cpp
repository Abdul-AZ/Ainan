#include <pch.h>

#include "ExportCamera.h"

namespace ALZ {

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

		ImGui::Begin("Export Settings", &SettingsWindowOpen);

		ImGui::Checkbox("Draw Export Camera Outline", &m_DrawExportCamera);

		//update position if any of these are changed, using SetSize().
		if(ImGui::DragFloat2("Position", &m_ExportCameraPosition.x, 0.01f))
			SetSize();
		if(ImGui::DragFloat2("Size", &m_ExportCameraSize.x, 0.01f))
			SetSize();

		//clamp the size, so it is always positive
		m_ExportCameraSize.x = std::clamp(m_ExportCameraSize.x, 0.0f, 100000.0f);
		m_ExportCameraSize.y = std::clamp(m_ExportCameraSize.y, 0.0f, 100000.0f);

		if (ImGui::TreeNode("Image Saving:"))
	{
		if (ImGui::BeginCombo("Image Format", Image::GetFormatString(SaveImageFormat).c_str()))
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
		ImGui::InputScalar("width", ImGuiDataType_::ImGuiDataType_U32, &ImageResolution.x);
		ImGui::InputScalar("height", ImGuiDataType_::ImGuiDataType_U32, &ImageResolution.y);
	
		if (ImGui::Button("Save Location"))
			m_ImageLocationBrowser.OpenWindow();
	
		ImGui::TreePop();
	}
		ImGui::End();

	    m_ImageLocationBrowser.DisplayGUI([this](const std::string& path) {
		ImageSavePath = path;
		m_ImageLocationBrowser.CloseWindow();
		});

	}
}