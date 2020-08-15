#include <pch.h>

#include "Background.h"

#include <numeric>

namespace Ainan {

	Background::Background()
	{
		auto vertices = Renderer::GetQuadVertices();

		VertexLayout layout(1);
		layout[0] = { "aPos", ShaderVariableType::Vec2 };
		VBO = Renderer::CreateVertexBuffer(vertices.data(), vertices.size() * sizeof(glm::vec2), layout, Renderer::ShaderLibrary()["BackgroundShader"]);

		TransformUniformBuffer = Renderer::CreateUniformBuffer("ObjectTransform", 1, { {"u_Model", ShaderVariableType::Mat4} }, nullptr);

		VertexLayout uniformBufferLayout = 
		{
			{"g_BaseColor", ShaderVariableType::Vec3},
			{"g_BaseLight", ShaderVariableType::Float},
			{"g_Constant", ShaderVariableType::Float},
			{"g_Linear", ShaderVariableType::Float},
			{"g_Quadratic", ShaderVariableType::Float},
			{"RadialLightPosition", ShaderVariableType::Vec2Array, MAX_NUM_RADIAL_LIGHTS},
			{"RadialLightColor", ShaderVariableType::Vec3Array, MAX_NUM_RADIAL_LIGHTS},
			{"RadialLightIntensity", ShaderVariableType::FloatArray, MAX_NUM_RADIAL_LIGHTS},
			{"SpotLightPosition", ShaderVariableType::Vec2Array, MAX_NUM_SPOT_LIGHTS},
			{"SpotLightColor", ShaderVariableType::Vec3Array, MAX_NUM_SPOT_LIGHTS},
			{"SpotLightAngle", ShaderVariableType::FloatArray, MAX_NUM_SPOT_LIGHTS},
			{"SpotLightInnerCutoff", ShaderVariableType::FloatArray, MAX_NUM_SPOT_LIGHTS},
			{"SpotLightOuterCutoff", ShaderVariableType::FloatArray, MAX_NUM_SPOT_LIGHTS},
			{"SpotLightIntensity", ShaderVariableType::FloatArray, MAX_NUM_SPOT_LIGHTS}
		};

		LightingUniformBuffer = Renderer::CreateUniformBuffer("LightingData", 2, uniformBufferLayout, nullptr);
		LightDataPackingBufferSize = std::accumulate(uniformBufferLayout.begin(), uniformBufferLayout.end(), 0,
			[](const uint32_t& a, const VertexLayoutPart& b)
			{
				return a + b.GetSize();
			});
		LightDataPackingBuffer = new uint8_t[LightDataPackingBufferSize]();
	}

	Background::~Background()
	{
		delete[] LightDataPackingBuffer;
	}

	void Background::SubmitLight(const RadialLight& light)
	{
		m_RadialLightPositionBuffer[m_RadialLightSubmissionCount] = light.Position * c_GlobalScaleFactor;
		m_RadialLightColorBuffer[m_RadialLightSubmissionCount] = light.Color;
		m_RadialLightIntensityBuffer[m_RadialLightSubmissionCount] = light.Intensity;

		m_RadialLightSubmissionCount++;
	}

	void Background::SubmitLight(const SpotLight& light)
	{
		m_SpotLightPositionBuffer[m_SpotLightSubmissionCount] = light.Position * c_GlobalScaleFactor;
		m_SpotLightColorBuffer[m_SpotLightSubmissionCount] = light.Color;
		m_SpotLightAngleBuffer[m_SpotLightSubmissionCount] = light.Angle * PI / 180.0f;            //convert to radians
		m_SpotLightInnerCutoffBuffer[m_SpotLightSubmissionCount] = light.InnerCutoff * PI / 180.0f;//convert to radians
		m_SpotLightOuterCutoffBuffer[m_SpotLightSubmissionCount] = light.OuterCutoff * PI / 180.0f;//convert to radians
		m_SpotLightIntensityBuffer[m_SpotLightSubmissionCount] = light.Intensity;

		m_SpotLightSubmissionCount++;
	}

	void Background::DisplayGUI(Environment& env)
	{
		if (!SettingsWindowOpen)
			return;

		ImGui::Begin("Background", &SettingsWindowOpen);
		ImGui::Text("Background\nColor");
		ImGui::SameLine();
		ImGui::ColorEdit3("##Base Background Color", &env.BackgroundColor.r);

		ImGui::Text("Base Light");
		ImGui::SameLine();
		float xPos = ImGui::GetCursorPosX();
		ImGui::SliderFloat("##Base Light", &env.BackgroundBaseLight, 0.0f, 1.0f);

		ImGui::Text("Constant");
		ImGui::SameLine();
		ImGui::SetCursorPosX(xPos);
		ImGui::DragFloat("##Constant", &env.BackgroundConstant, 0.01f);

		ImGui::Text("Linear");
		ImGui::SameLine();
		ImGui::SetCursorPosX(xPos);
		ImGui::DragFloat("##Linear", &env.BackgroundLinear, 0.0001f);

		ImGui::Text("Quadratic");
		ImGui::SameLine();
		ImGui::SetCursorPosX(xPos);
		ImGui::DragFloat("##Quadratic", &env.BackgroundQuadratic, 0.00001f);

		ImGui::End();
	}

	void Background::Draw(Environment& env)
	{
		auto& shader = Renderer::ShaderLibrary()["BackgroundShader"];

		//not used light spots
		for (int i = m_RadialLightSubmissionCount; i < MAX_NUM_RADIAL_LIGHTS; i++)
			m_RadialLightIntensityBuffer[i] = 0.0f;

		for (int i = m_SpotLightSubmissionCount; i < MAX_NUM_SPOT_LIGHTS; i++)
			m_SpotLightIntensityBuffer[i] = 0.0f;

		//zero buffer
		memset(LightDataPackingBuffer, 0, LightDataPackingBufferSize);
		//copy all data in the order of the layout
		{
			uint8_t* copyPtr = LightDataPackingBuffer;
			//copy material data
			memcpy(copyPtr, &env.BackgroundColor, sizeof(glm::vec3));
			copyPtr += sizeof(glm::vec3);
			memcpy(copyPtr, &env.BackgroundBaseLight, sizeof(float));
			copyPtr += sizeof(float);
			memcpy(copyPtr, &env.BackgroundConstant, sizeof(float));
			copyPtr += sizeof(float);
			memcpy(copyPtr, &env.BackgroundLinear, sizeof(float));
			copyPtr += sizeof(float);
			memcpy(copyPtr, &env.BackgroundQuadratic, sizeof(float));
			copyPtr += sizeof(float);

			//copy radial light data
			memcpy(copyPtr, m_RadialLightPositionBuffer, sizeof(glm::vec2) * m_RadialLightSubmissionCount);
			copyPtr += sizeof(glm::vec2) * MAX_NUM_RADIAL_LIGHTS;
			memcpy(copyPtr, m_RadialLightColorBuffer, sizeof(glm::vec3) * m_RadialLightSubmissionCount);
			copyPtr += sizeof(glm::vec3) * MAX_NUM_RADIAL_LIGHTS;
			memcpy(copyPtr, m_RadialLightIntensityBuffer, sizeof(float) * m_RadialLightSubmissionCount);
			copyPtr += sizeof(float) * MAX_NUM_RADIAL_LIGHTS;

			//copy spot light data
			memcpy(copyPtr, m_SpotLightPositionBuffer, sizeof(glm::vec2) * m_SpotLightSubmissionCount);
			copyPtr += sizeof(glm::vec2) * MAX_NUM_SPOT_LIGHTS;
			memcpy(copyPtr, m_SpotLightColorBuffer, sizeof(glm::vec3) * m_SpotLightSubmissionCount);
			copyPtr += sizeof(glm::vec3) * MAX_NUM_SPOT_LIGHTS;
			memcpy(copyPtr, m_SpotLightAngleBuffer, sizeof(float) * m_SpotLightSubmissionCount);
			copyPtr += sizeof(float) * MAX_NUM_SPOT_LIGHTS;
			memcpy(copyPtr, m_SpotLightInnerCutoffBuffer, sizeof(float) * m_SpotLightSubmissionCount);
			copyPtr += sizeof(float) * MAX_NUM_SPOT_LIGHTS;
			memcpy(copyPtr, m_SpotLightOuterCutoffBuffer, sizeof(float) * m_SpotLightSubmissionCount);
			copyPtr += sizeof(float) * MAX_NUM_SPOT_LIGHTS;
			memcpy(copyPtr, m_SpotLightIntensityBuffer, sizeof(float) * m_SpotLightSubmissionCount);
			copyPtr += sizeof(float) * MAX_NUM_SPOT_LIGHTS;
		}
		
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(5000.0f));

		shader->BindUniformBuffer(TransformUniformBuffer, 1, RenderingStage::VertexShader);
		TransformUniformBuffer->UpdateData((void*)&model);

		shader->BindUniformBuffer(LightingUniformBuffer, 2, RenderingStage::FragmentShader);
		LightingUniformBuffer->UpdateData(LightDataPackingBuffer);

		Renderer::Draw(*VBO, *shader, Primitive::Triangles, 6);

		m_RadialLightSubmissionCount = 0;
		m_SpotLightSubmissionCount = 0;
	}
}