#include "Exporter.h"

//#ifdef PLATFORM_WINDOWS
extern "C"
{
//#endif

#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

//#ifdef PLATFORM_WINDOWS
}
//#endif

#include "Editor.h"

namespace Ainan {

	Exporter::Exporter() :
		m_Camera(ProjectionMode::Orthographic, glm::mat4(1.0f), 16.0f / 9.0f)
	{
		memset(m_OutlineVertices.data(), 0, m_OutlineVertices.size() * sizeof(glm::vec2));

		VideoSettings.ExportTargetLocation.m_FileName = "Example Name";
		VideoSettings.ExportTargetLocation.FileExtension = ".mp4";

		PictureSettings.ExportTargetLocation.m_FileName = "Example Name";
		PictureSettings.ExportTargetLocation.FileExtension = ".png";
		PictureSettings.ExportTargetPath = PictureSettings.ExportTargetLocation.GetSelectedSavePath();
	}

	Exporter::~Exporter()
	{
		if (m_ExportTargetTexture.IsValid())
			Renderer::DestroyTexture(m_ExportTargetTexture);
	}

	void Exporter::ExportIfScheduled(Editor& editor)
	{
		if (!m_ExporterScheduled)
			return;
		m_ExporterScheduled = false;

		if (m_Mode == ExportMode::Picture)
			ExportImage(editor);
		else if (m_Mode == ExportMode::Video)
			ExportVideo(editor);
	}

	void Exporter::DrawEnvToExportSurface(Environment& env)
	{
		m_Camera.SetAspectRatio(16.0f / 9.0f);

		for (pEnvironmentObject& obj : env.Objects)
		{
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
		int32_t index = env.FindObjectByID(ExportCameraID);
		if (index == -1)
			AINAN_LOG_FATAL("Cannot find export camera");

		CameraObject* cameraObj = (CameraObject*)env.Objects[index].get();
		desc.SceneCamera = cameraObj->m_Camera;
		desc.SceneDrawTarget = m_RenderSurface.SurfaceFramebuffer;
		desc.Blur = env.BlurEnabled;
		desc.BlurRadius = env.BlurRadius;
		Renderer::BeginScene(desc);
		float width = 1920;
		float height = width / desc.SceneCamera.GetAspectRatio();
		m_RenderSurface.SetSize(glm::ivec2(width, std::round(height / 2.0f) * 2.0f));
		m_RenderSurface.SurfaceFramebuffer.Bind();
		Renderer::ClearScreen();
		//render skybox if in perspective projection
		if (desc.SceneCamera.GetProjectionMode() == ProjectionMode::Perspective)
			env.EnvSkybox.Draw(desc.SceneCamera);
		for (pEnvironmentObject& obj : env.Objects)
			obj->Draw();

		Renderer::EndScene();
	}

	void Exporter::GetImageFromExportSurfaceToRAM()
	{
		delete m_ExportTargetImage;
		m_ExportTargetImage = m_RenderSurface.SurfaceFramebuffer.ReadPixels();
	}

	void Exporter::DisplayGUI(Environment& env)
	{
		ImGui::PushID(this);
		if (m_ExporterWindowOpen)
		{
			ImGui::Begin("Exporter", &m_ExporterWindowOpen);

			//get all the camers
			std::vector<CameraObject*> cameras;
			for (size_t i = 0; i < env.Objects.size(); i++)
			{
				if (env.Objects[i]->Type == CameraType)
				{
					cameras.push_back((CameraObject*)env.Objects[i].get());
				}
			}

			//display an empty dropdown if there isn't any cameras
			if (cameras.size() == 0)
			{
				IMGUI_DROPDOWN_START("Camera", "There are no cameras in the Environment");
				IMGUI_DROPDOWN_END();
			}
			else
			{
				//get the name of the currently selected camera from the object pool so that we are sure it isn't deleted
				std::string selectedObjName = "";
				int32_t index = env.FindObjectByID(ExportCameraID);
				if (index != -1)
				{
					selectedObjName = env.Objects[index]->m_Name;
				}

				IMGUI_DROPDOWN_START("Camera", selectedObjName.c_str());
				for (size_t i = 0; i < cameras.size(); i++)
				{
					IMGUI_DROPDOWN_SELECTABLE(ExportCameraID, cameras[i]->ID, cameras[i]->m_Name.c_str());
				}
				IMGUI_DROPDOWN_END();
			}

			IMGUI_DROPDOWN_START("Export Mode", GetModeString(m_Mode).c_str());
			IMGUI_DROPDOWN_SELECTABLE(m_Mode, Video, GetModeString(Video).c_str());
			IMGUI_DROPDOWN_SELECTABLE(m_Mode, Picture, GetModeString(Picture).c_str());
			IMGUI_DROPDOWN_END();


			if (m_Mode == ExportMode::Video)
				DisplayVideoExportSettingsControls();
			{
				ImGui::Text("Capture After :");

				bool textHovered = ImGui::IsItemHovered();

				ImGui::SameLine();
				ImGui::PushItemWidth(150);
				if (ImGui::DragFloat("##Capture After :", &ExportStartTime, 0.1f))
					ExportStartTime = std::clamp(ExportStartTime, 0.0f, 3600.0f);

				bool dragFloatHovered = ImGui::IsItemHovered();

				if (textHovered || dragFloatHovered) 
				{
					ImGui::BeginTooltip();
					ImGui::Text("This specifies when to start exporting");
					ImGui::EndTooltip();
				}
			}

			const float ExportButtonWidth = 300.0f;
			const float ExportButtonHeight = 75.0f;
			ImGui::SetCursorPosY(ImGui::GetWindowSize().y - ExportButtonHeight - ImGui::GetFrameHeightWithSpacing() / 2.0f);
			ImGui::SetCursorPosX(ImGui::GetWindowSize().x / 2.0f - ExportButtonWidth / 2.0f);
			if (ImGui::Button("Export", ImVec2(ExportButtonWidth, ExportButtonHeight)))
			{
				if(ExportCameraID != UUID())
					m_ExporterScheduled = true;
			}

			Renderer::RegisterWindowThatCanCoverViewport();
			ImGui::End();

			PictureSettings.ExportTargetLocation.DisplayGUI([this](const std::string& path)
				{
					PictureSettings.ExportTargetPath = path;
					PictureSettings.ExportTargetLocation.CloseWindow();
				});
		}

		if (m_FinalizePictureExportWindowOpen)
			DisplayFinalizePictureExportSettingsWindow();

		ImGui::PopID();
	}

	void Exporter::DisplayVideoExportSettingsControls()
	{
		if (ImGui::Button("Save Location"))
			VideoSettings.ExportTargetLocation.OpenWindow();

		VideoSettings.ExportTargetLocation.DisplayGUI([this](const std::string& path)
			{
				VideoSettings.ExportTargetPath = path + ".mp4";
				VideoSettings.ExportTargetLocation.CloseWindow();
			});

		ImGui::PushItemWidth(25);
		ImGui::Text("Length: ");
		ImGui::SameLine();
		ImGui::DragInt("##Minutes", &VideoSettings.LengthMinutes, 1, 0, 60);
		ImGui::SameLine();
		ImGui::Text("Minutes");
		ImGui::SameLine();
		ImGui::Text(":");
		ImGui::SameLine();
		ImGui::DragInt("##Seconds", &VideoSettings.LengthSeconds, 1, 0, 60);
		ImGui::SameLine();
		ImGui::Text("Seconds");
		ImGui::PopItemWidth();

		ImGui::TextColored({ 0.0f, 0.8f, 0.0f, 1.0f }, VideoSettings.ExportTargetLocation.GetSelectedSavePath().c_str());
	}

	void Exporter::DisplayFinalizePictureExportSettingsWindow()
	{
		ImGui::Begin("Finalize Export", &m_FinalizePictureExportWindowOpen);
		ImGui::Columns(2);

		IMGUI_DROPDOWN_START("Image Format", Image::GetFormatString(PictureSettings.Format).c_str());
		IMGUI_DROPDOWN_SELECTABLE(PictureSettings.Format, ImageFormat::png, Image::GetFormatString(ImageFormat::png).c_str());
		IMGUI_DROPDOWN_SELECTABLE(PictureSettings.Format, ImageFormat::jpeg, Image::GetFormatString(ImageFormat::jpeg).c_str());
		IMGUI_DROPDOWN_SELECTABLE(PictureSettings.Format, ImageFormat::bmp, Image::GetFormatString(ImageFormat::bmp).c_str());
		IMGUI_DROPDOWN_END();

		PictureSettings.ExportTargetLocation.FileExtension = "." + Image::GetFormatString(PictureSettings.Format);

		if (ImGui::Button("Save Location"))
			PictureSettings.ExportTargetLocation.OpenWindow();

		ImGui::Text("Selected Save Path: ");
		ImGui::SameLine();
		PictureSettings.ExportTargetPath = PictureSettings.ExportTargetLocation.GetSelectedSavePath();

		ImGui::TextColored({ 0.0f,0.8f,0.0f,1.0f }, PictureSettings.ExportTargetPath.u8string().c_str());

		if (ImGui::Button("Save"))
		{
			m_ExportTargetImage->SaveToFile(PictureSettings.ExportTargetPath.u8string(), PictureSettings.Format);
			m_FinalizePictureExportWindowOpen = false;
		}

		ImGui::NextColumn();

		if (m_ExportTargetTexture.IsValid())
		{
			ImGui::Text("Preview");

			int scale = 250;
			ImVec2 size = ImVec2(scale * m_ExportTargetImage->m_Width / m_ExportTargetImage->m_Height, scale);
			ImGui::Image((void*)m_ExportTargetTexture.GetTextureID(), size);
		}

		Renderer::RegisterWindowThatCanCoverViewport();
		ImGui::End();
	}

	void Exporter::DisplayProgressBarWindow(int32_t operationNum, int32_t operationCount, float fraction)
	{
		//set position be in the centre of the screen
		ImGui::SetNextWindowPos(ImVec2(Window::FramebufferSize.x, Window::FramebufferSize.y) / 2, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(400, 75));
		ImGui::Begin("Exporting...", 0, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize);
		ImGui::Text("Operation %i of %i", operationNum, operationCount);
		ImGui::ProgressBar(fraction, ImVec2(-1,-1));
		Renderer::RegisterWindowThatCanCoverViewport();
		ImGui::End();
	}

	void Exporter::OpenExporterWindow()
	{
		m_ExporterWindowOpen = true;
	}

	void Exporter::ExportImage(Editor& editor)
	{
		editor.PlayMode();

		while (editor.m_TimeSincePlayModeStarted < ExportStartTime)
			editor.Update();

		DrawEnvToExportSurface(*editor.m_Env);
		GetImageFromExportSurfaceToRAM();

		if (Renderer::Rdata->API == RendererType::OpenGL)
			m_ExportTargetImage->FlipHorizontally();

		if (m_ExportTargetTexture.IsValid())
			Renderer::DestroyTexture(m_ExportTargetTexture);
		m_ExportTargetTexture = Renderer::CreateTexture(*m_ExportTargetImage);

		m_FinalizePictureExportWindowOpen = true;

		editor.Stop();
	}

#define CHECK(x) if((x) < 0) { AINAN_LOG_ERROR("Error while exporting video"); editor.Stop(); return; } 
#define CHECKP(x) if((x) == 0) { AINAN_LOG_ERROR("Error while exporting video"); editor.Stop(); return; } 

	void Exporter::ExportVideo(Editor& editor)
	{
		editor.PlayMode();

		auto updateUI = [this, &editor](int32_t operationIndex, int32_t operationCount, float fraction)
		{
			Renderer::SetRenderTargetApplicationWindow();
			Renderer::ImGuiNewFrame();
			ImGuiWrapper::BeginGlobalDocking(true);
			DisplayProgressBarWindow(operationIndex, operationCount, fraction);
			ImGuiWrapper::EndGlobalDocking();
			Renderer::ImGuiEndFrame(true);
			Renderer::Present();
		};

		while (editor.m_TimeSincePlayModeStarted < ExportStartTime)
		{
			editor.Update();
			updateUI(1, 2, editor.m_TimeSincePlayModeStarted / ExportStartTime);
		}

		DrawEnvToExportSurface(*editor.m_Env);
		GetImageFromExportSurfaceToRAM();
		if (Renderer::Rdata->API == RendererType::OpenGL)
			m_ExportTargetImage->FlipHorizontally();
		const AVPixelFormat fmt = AV_PIX_FMT_YUV420P;

		AVFormatContext* fContext = nullptr;
		AVCodecContext* cContext = nullptr;
		AVStream* vStream = nullptr;
		AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
		cContext = avcodec_alloc_context3(codec);
		int32_t result = 0;
		result = avformat_alloc_output_context2(&fContext, nullptr, nullptr, VideoSettings.ExportTargetLocation.GetSelectedSavePath().c_str());
		CHECK(result);
		int32_t totalFrameCount = 60 * VideoSettings.LengthSeconds + 60 * VideoSettings.LengthSeconds * VideoSettings.LengthMinutes;
		fContext->duration = totalFrameCount;

		vStream = avformat_new_stream(fContext, codec);
		vStream->codecpar->format = fmt;
		vStream->codecpar->width = std::round(m_ExportTargetImage->m_Width / 2.0f) * 2;
		vStream->codecpar->height = std::round(m_ExportTargetImage->m_Height / 2.0f) * 2;
		vStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
		vStream->codecpar->codec_id = AV_CODEC_ID_H264;
		CHECKP(vStream);

		result = avio_open(&fContext->pb, fContext->filename, AVIO_FLAG_WRITE);
		CHECK(result);

		cContext->width = vStream->codecpar->width;
		cContext->height = vStream->codecpar->height;
		cContext->pix_fmt = fmt;
		cContext->framerate = AVRational{ 60,1 };
		cContext->time_base = AVRational{ 1, 60 };

		result = avcodec_open2(cContext, codec, 0);
		CHECK(result);

		vStream->codec->width = cContext->width;
		vStream->codec->height = cContext->height;
		vStream->codec->pix_fmt = cContext->pix_fmt;
		vStream->codec->framerate = cContext->framerate;
		vStream->codec->time_base = cContext->time_base;

		result = avformat_write_header(fContext, 0);
		CHECK(result);

		AVPacket* pkt = av_packet_alloc();
		AVFrame* frame = av_frame_alloc();
		SwsContext* swsContext = sws_getContext(cContext->width, cContext->height, AV_PIX_FMT_RGBA,
			cContext->width, cContext->height, fmt, 0, 0, 0, 0);
		CHECKP(swsContext);
		frame->width = cContext->width;
		frame->height = cContext->height;
		frame->format = fmt;
		frame->color_range = AVColorRange::AVCOL_RANGE_MPEG;
		av_image_alloc(frame->data, frame->linesize, frame->width, frame->height, fmt, 1);
		for (size_t i = 0; i < totalFrameCount; i++)
		{
			result = 0;
			
			frame->pts = av_rescale_q(i, cContext->time_base, vStream->time_base);
			int rgba_stride[4] = { 4 * frame->width, 0, 0, 0 };
			int dst_stride[4] = { frame->height, 0, 0, 0 };
			int idk = av_image_get_buffer_size(AV_PIX_FMT_RGBA, frame->width, frame->height, 1);
			result = sws_scale(swsContext, &m_ExportTargetImage->m_Data, rgba_stride, 0, frame->height, frame->data, frame->linesize);
			CHECK(result);
			result = avcodec_send_frame(cContext, frame);
			CHECK(result);
			result = avcodec_receive_packet(cContext, pkt);
			if (result != -11)
			{
				result = av_interleaved_write_frame(fContext, pkt);
			}
			editor.Update();
			DrawEnvToExportSurface(*editor.m_Env);
			GetImageFromExportSurfaceToRAM();
			if (Renderer::Rdata->API == RendererType::OpenGL)
				m_ExportTargetImage->FlipHorizontally();
			updateUI(2, 2, (float)i / totalFrameCount);
		}
		av_packet_unref(pkt);
		av_freep(&frame->data[0]);
		av_frame_free(&frame);
		sws_freeContext(swsContext);

		result = av_write_trailer(fContext);
		CHECK(result);

		avio_close(fContext->pb);
		avcodec_free_context(&cContext);
		avformat_free_context(fContext);

		editor.Stop();
	}
}
