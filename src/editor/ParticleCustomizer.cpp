#include "ParticleCustomizer.h"

#include "environment/EnvironmentObjectInterface.h"

namespace Ainan {

	ParticleCustomizer::ParticleCustomizer() :
		mt(std::random_device{}())
	{
		VertexLayout layout(1);
		layout[0] = VertexLayoutElement("POSITION", 0, ShaderVariableType::Vec3);
		m_LineVertexBuffer = Renderer::CreateVertexBuffer(nullptr, sizeof(glm::vec3) * 2, layout, Renderer::ShaderLibrary()["LineShader"], true);

		glm::vec2 vertices[c_CircleVertexCount];
		uint32_t indecies[c_CircleVertexCount * 2 - 2];

		float degreesBetweenVertices = 360.0f / c_CircleVertexCount;

		for (int32_t i = 0; i < c_CircleVertexCount; i++)
		{
			float angle = i * degreesBetweenVertices;
			vertices[i].x = (float)cos(angle * (glm::pi<float>() / 180.0));
			vertices[i].y = (float)sin(angle * (glm::pi<float>() / 180.0));

			if (i == c_CircleVertexCount - 1)
				continue;

			indecies[i * 2] = i;
			indecies[i * 2 + 1] = i + 1;
		}
		vertices[c_CircleVertexCount - 1] = vertices[0];

		layout[0] = VertexLayoutElement("POSITION", 0, ShaderVariableType::Vec2);
		m_CircleVertexBuffer = Renderer::CreateVertexBuffer(vertices, sizeof(glm::vec2) * c_CircleVertexCount, layout, Renderer::ShaderLibrary()["CircleOutlineShader"]);

		m_CircleIndexBuffer = Renderer::CreateIndexBuffer(indecies, c_CircleVertexCount * 2 - 2);

		m_CircleTransformUniformBuffer = Renderer::CreateUniformBuffer("ObjectTransform", 1, { VertexLayoutElement("u_Model", 0, ShaderVariableType::Mat4) });

		layout[0] = VertexLayoutElement("COLOR", 0, ShaderVariableType::Vec4);
		m_SpawnAreaColorUniformBuffer = Renderer::CreateUniformBuffer("ObjectColor", 1, layout);
		m_SpawnAreaColorUniformBuffer.UpdateData((void*)&c_ParticleSpawnAreaColor, sizeof(glm::vec4));
	}

	ParticleCustomizer::~ParticleCustomizer()
	{
		Renderer::DestroyVertexBuffer(m_LineVertexBuffer);
		Renderer::DestroyVertexBuffer(m_CircleVertexBuffer);
		Renderer::DestroyIndexBuffer(m_CircleIndexBuffer);
		Renderer::DestroyUniformBuffer(m_SpawnAreaColorUniformBuffer);
		Renderer::DestroyUniformBuffer(m_CircleTransformUniformBuffer);
	}

	std::string GetModeAsText(const SpawnMode& mode)
	{
		switch (mode)
		{
		case SpawnMode::SpawnOnPoint:
			return "Spawn On Point";

		case SpawnMode::SpawnOnLine:
			return "Spawn On Line";

		case SpawnMode::SpawnOnCircle:
			return "Spawn On Circle";

		case SpawnMode::SpawnInsideCircle:
			return "Spawn Inside Circle";

		default:
			return "";
		}
	}

	SpawnMode GetTextAsMode(const std::string& mode)
	{
		if (mode == "Spawn On Point")
			return SpawnMode::SpawnOnPoint;
		else if (mode == "Spawn On Line")
			return SpawnMode::SpawnOnLine;
		else if (mode == "Spawn On Circle")
			return SpawnMode::SpawnOnCircle;
		else if (mode == "Spawn Inside Circle")
			return SpawnMode::SpawnInsideCircle;
		else {
			AINAN_LOG_ERROR("Invalid spawn mode string");
			return SpawnMode::SpawnOnPoint;
		}
	}

	void ParticleCustomizer::DisplayGUI(const std::string& windowName)
	{
	}

	ParticleDescription ParticleCustomizer::GetParticleDescription()
	{
		ParticleDescription particleDesc = {};

		switch (Mode)
		{
		case SpawnMode::SpawnOnPoint: 
		{
			particleDesc.Position = m_SpawnPosition;
			break;
		}

		case SpawnMode::SpawnOnLine: 
		{
			std::uniform_real_distribution<float> dest(-1.0f, 1.0f);

			float t = dest(mt);
			float x = m_SpawnPosition.x + t * m_LineLength * cos(glm::radians(m_LineAngle));
			float y = m_SpawnPosition.x + t * m_LineLength * sin(glm::radians(m_LineAngle));

			particleDesc.Position = glm::vec3(x, y, 0.0f);
			break;
		}

		case SpawnMode::SpawnOnCircle: 
		{
			//random angle between 0 and 2pi (360 degrees)
			std::uniform_real_distribution<float> dest(0.0f, 2.0f * 3.14159f);
			float angle = dest(mt);

			float x = m_SpawnPosition.x + m_CircleRadius * cos(angle);
			float y = m_SpawnPosition.y + m_CircleRadius * sin(angle);

			particleDesc.Position = glm::vec3(x, y, 0.0f);
			break;
		}

		case SpawnMode::SpawnInsideCircle: 
		{
			std::uniform_real_distribution<float> dest(0.0f, 1.0f);
			float r = m_CircleRadius * sqrt(dest(mt));
			float theta = dest(mt) * 2 * glm::pi<float>(); //in radians
			particleDesc.Position = glm::vec3(m_SpawnPosition.x + r * cos(theta), m_SpawnPosition.y + r * sin(theta), 0.0f);
			break;
		}
		}

		particleDesc.Velocity = m_VelocityCustomizer.GetVelocity();
		particleDesc.LifeTime = m_LifetimeCustomizer.GetLifetime();

		//particleDesc.StartScale = m_ScaleCustomizer.GetScaleInterpolator().startPoint;
		if (m_ScaleCustomizer.m_RandomScale)
		{
			std::uniform_real_distribution<float> dest(m_ScaleCustomizer.m_MinScale, m_ScaleCustomizer.m_MaxScale);
			particleDesc.StartScale = dest(mt);
		}
		else
			particleDesc.StartScale = m_ScaleCustomizer.m_DefinedScale;

		particleDesc.EndScale = m_ScaleCustomizer.m_EndScale;

		return particleDesc;
	}

	void ParticleCustomizer::DrawWorldSpaceUI()
	{
		if (Mode == SpawnMode::SpawnOnLine)
		{
			glm::vec3 offset = m_LineLength * glm::vec3(cos(glm::radians(m_LineAngle)), sin(glm::radians(m_LineAngle)), 0.0f);

			std::array<glm::vec3, 2> vertices;
			vertices[0] = (glm::vec3(m_SpawnPosition.x, m_SpawnPosition.y, 0.0f) + offset);
			vertices[1] = (glm::vec3(m_SpawnPosition.x, m_SpawnPosition.y, 0.0f) - offset);

			m_LineVertexBuffer.UpdateData(0, sizeof(glm::vec3) * 2, vertices.data());

			auto& shader = Renderer::ShaderLibrary()["LineShader"];
			shader.BindUniformBuffer(m_SpawnAreaColorUniformBuffer, 1, RenderingStage::FragmentShader);

			Renderer::Draw(m_LineVertexBuffer, shader, Primitive::Lines, 2);
		}
		else if (Mode == SpawnMode::SpawnOnCircle || Mode == SpawnMode::SpawnInsideCircle)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(m_SpawnPosition.x, m_SpawnPosition.y, 0.0f));
			model = glm::scale(model, glm::vec3(m_CircleRadius, m_CircleRadius, m_CircleRadius));

			auto& shader = Renderer::ShaderLibrary()["CircleOutlineShader"];

			shader.BindUniformBuffer(m_CircleTransformUniformBuffer, 1, RenderingStage::VertexShader);
			m_CircleTransformUniformBuffer.UpdateData(&model, sizeof(glm::mat4));

			shader.BindUniformBuffer(m_SpawnAreaColorUniformBuffer, 2, RenderingStage::FragmentShader);

			Renderer::Draw(m_CircleVertexBuffer, shader, Primitive::Lines, m_CircleIndexBuffer);
		}

	}
}