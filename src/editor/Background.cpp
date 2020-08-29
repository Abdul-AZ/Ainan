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
			{"g_Quadratic", ShaderVariableType::Float}
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
		}
		
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(5000.0f));

		shader->BindUniformBuffer(Renderer::Rdata->SceneUniformbuffer, 0, RenderingStage::FragmentShader);

		shader->BindUniformBuffer(TransformUniformBuffer, 1, RenderingStage::VertexShader);
		TransformUniformBuffer->UpdateData((void*)&model);

		shader->BindUniformBuffer(LightingUniformBuffer, 2, RenderingStage::FragmentShader);
		LightingUniformBuffer->UpdateData(LightDataPackingBuffer);

		Renderer::Draw(VBO, shader, Primitive::Triangles, 6);
	}
}