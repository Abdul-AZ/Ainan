#include "ExportCamera.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
}

namespace Ainan {

	//TODO use environment directory instead of default
	ExportCamera::ExportCamera() :
		m_ImageLocationBrowser(STARTING_BROWSER_DIRECTORY, "Save Image"), 
		RealCamera(CameraMode::CentreIsMidPoint)
	{
		memset(m_OutlineVertices.data(), 0, m_OutlineVertices.size() * sizeof(glm::vec2));

		ImageSavePath = STARTING_BROWSER_DIRECTORY;

		VertexLayout layout(1);
		layout[0] = VertexLayoutElement("POSITION", 0, ShaderVariableType::Vec2);
		m_OutlineVertexBuffer = Renderer::CreateVertexBuffer(nullptr, sizeof(glm::vec2) * 8, layout, Renderer::ShaderLibrary()["LineShader"], true);

		layout[0] = VertexLayoutElement("u_Color",0, ShaderVariableType::Vec4);
		m_OutlineUniformBuffer = Renderer::CreateUniformBuffer("ObjectColor", 1, layout, (void*)&c_OutlineColor);

		SetSize();
	}

	void ExportCamera::DrawOutline()
	{
		if (!m_DrawExportCamera)
			return;

		std::array<glm::vec2, 8> vertices =
		{
			m_OutlineVertices[0], m_OutlineVertices[1], //bottom left to top left
			m_OutlineVertices[1], m_OutlineVertices[2], //top left to top right
			m_OutlineVertices[2], m_OutlineVertices[3], //top right to bottom right
			m_OutlineVertices[3], m_OutlineVertices[0]  //bottom right to bottom left
		};

		m_OutlineVertexBuffer->UpdateData(0, sizeof(glm::vec2) * 8, vertices.data());

		auto& shader = Renderer::ShaderLibrary()["LineShader"];
		shader->BindUniformBuffer(m_OutlineUniformBuffer, 1, RenderingStage::FragmentShader);

		Renderer::Draw(m_OutlineVertexBuffer, shader, Primitive::Lines,  8);
	}

	void ExportCamera::SetSize()
	{
		glm::vec2 size = glm::vec2(RealCamera.ZoomFactor * m_AspectRatio, RealCamera.ZoomFactor) / c_GlobalScaleFactor;
		m_OutlineVertices[0] = m_ExportCameraPosition - (size / 2.0f); //bottom left
		m_OutlineVertices[1] = m_ExportCameraPosition + glm::vec2(-size.x, size.y) / 2.0f; //top left
		m_OutlineVertices[2] = m_ExportCameraPosition + (size / 2.0f);     //top right
		m_OutlineVertices[3] = m_ExportCameraPosition + glm::vec2(size.x, -size.y) / 2.0f; //bottom right

		for (glm::vec2& vertex : m_OutlineVertices)
			vertex *= c_GlobalScaleFactor;

		RealCamera.Update(0.0f, Renderer::GetCurrentViewport());
		glm::vec2 reversedPos = glm::vec2(-m_ExportCameraPosition.x, -m_ExportCameraPosition.y);

		RealCamera.SetPosition(reversedPos * c_GlobalScaleFactor);
	}

#define CHECK(x) if((x) < 0) __debugbreak();
#define CHECKP(x) if((x) == 0) __debugbreak();

	void ExportCamera::ExportVideoLoop(Environment& env)
	{
		RenderNextFrame(env);

		AVFormatContext* fContext = nullptr;
		AVCodecContext* cContext = nullptr;
		AVStream* vStream = nullptr;
		AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
		cContext = avcodec_alloc_context3(codec);
		int32_t result = 0;
		result = avformat_alloc_output_context2(&fContext, nullptr, nullptr, "sample.mp4");
		CHECK(result);
		fContext->duration = 60 * 3;

		vStream = avformat_new_stream(fContext, codec);
		vStream->codecpar->format = AV_PIX_FMT_YUV420P;
		vStream->codecpar->width = std::round(m_ExportTargetImage->m_Width / 2.0f) * 2;
		vStream->codecpar->height = std::round(m_ExportTargetImage->m_Height/ 2.0f) * 2;
		vStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
		vStream->codecpar->codec_id = AV_CODEC_ID_H264;
		CHECKP(vStream);

		result = avio_open(&fContext->pb, fContext->filename, AVIO_FLAG_WRITE);
		CHECK(result);

		cContext->width = vStream->codecpar->width;
		cContext->height = vStream->codecpar->height;
		cContext->pix_fmt = AV_PIX_FMT_YUV420P;
		cContext->framerate = AVRational{ 25,1 };
		cContext->time_base = AVRational{ 1,25 };

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

		AVFrame* frame = 0;
		for (size_t i = 0; i < 60 * 3 - 1; i++)
		{
			result = 0;
			RenderNextFrame(env);

			if(i == 0)
				frame = av_frame_alloc();
			frame->width = cContext->width;
			frame->height = cContext->height;
			frame->format = AV_PIX_FMT_YUV420P;
			frame->pts = av_rescale_q(i, cContext->time_base, vStream->time_base);
			int32_t size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, frame->width, frame->height, 1);
			av_image_alloc(frame->data, frame->linesize, frame->width, frame->height, AV_PIX_FMT_YUV420P, 1);
			CHECK(size);
			CHECK(result);
			char buf[AV_ERROR_MAX_STRING_SIZE];
			av_make_error_string(buf, AV_ERROR_MAX_STRING_SIZE, -11);
			result = avcodec_send_frame(cContext, frame);
			CHECK(result);
			result = avcodec_receive_packet(cContext, pkt);
			if (result != -11)
			{
				result = av_interleaved_write_frame(fContext, pkt);
			}
		}
		av_frame_unref(frame);

		result = av_write_trailer(fContext);
		CHECK(result);
	}

	void ExportCamera::RenderNextFrame(Environment& env)
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
			if (obj->Type == RadialLightType)
			{
				RadialLight* light = static_cast<RadialLight*>(obj.get());
				Renderer::AddRadialLight(light->Position, light->Color, light->Intensity);
			}
			else if (obj->Type == SpotLightType)
			{
				SpotLight* light = static_cast<SpotLight*>(obj.get());
				Renderer::AddSpotLight(light->Position, light->Color, light->Angle, light->InnerCutoff, light->OuterCutoff, light->Intensity);
			}
		}

		for (pEnvironmentObject& obj : env.Objects)
			obj->Draw();

		Renderer::EndScene();

		delete m_ExportTargetImage;
		m_ExportTargetImage = new Image(m_RenderSurface.SurfaceFrameBuffer->ReadPixels());
	}

#undef CHECK(x)
#undef CHECKP(x)

	void ExportCamera::DisplayGUI()
	{
		ImGui::PushID(this);
		if (SettingsWindowOpen) 
		{
			ImGui::Begin("ExportMode Settings", &SettingsWindowOpen);

			ImGui::Text("Export Mode");
			ImGui::SameLine();
			if (ImGui::BeginCombo("##Export Mode", GetModeString(m_Mode).c_str()))
			{
				bool is_video = m_Mode == Video ? true : false;
				if (ImGui::Selectable(GetModeString(Video).c_str(), &is_video))
				{
					ImGui::SetItemDefaultFocus();
					m_Mode = Video;
				}

				bool is_picture = m_Mode == Picture ? true : false;
				if (ImGui::Selectable(GetModeString(Picture).c_str(), &is_picture))
				{
					ImGui::SetItemDefaultFocus();
					m_Mode = Picture;
				}

				ImGui::EndCombo();
			}


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
		if (m_Mode == Video)
		{
			ExportVideoLoop(env);
			return;
		}

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
			if (obj->Type == RadialLightType)
			{
				RadialLight* light = static_cast<RadialLight*>(obj.get());
				Renderer::AddRadialLight(light->Position, light->Color, light->Intensity);
			}
			else if (obj->Type == SpotLightType)
			{
				SpotLight* light = static_cast<SpotLight*>(obj.get());
				Renderer::AddSpotLight(light->Position, light->Color, light->Angle, light->InnerCutoff, light->OuterCutoff, light->Intensity);
			}
		}

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