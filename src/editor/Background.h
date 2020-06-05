#pragma once

#include "Camera.h"
#include "renderer/ShaderProgram.h"
#include "renderer/Renderer.h"
#include "environment/RadialLight.h"
#include "environment/SpotLight.h"
#include "environment/Environment.h"

//This should be the same as the one in the Background shader (inside the fragment shader)
#define MAX_NUM_RADIAL_LIGHTS 10
#define MAX_NUM_SPOT_LIGHTS 10

namespace Ainan {

	class Background {
	public:
		Background();
		~Background();

		void SubmitLight(const RadialLight& light);
		void SubmitLight(const SpotLight& light);

		void DisplayGUI(Environment& env);
		void Draw(Environment& env);

	public:
		bool SettingsWindowOpen = true;

	private:
		std::shared_ptr<VertexBuffer> VBO;
		std::shared_ptr<UniformBuffer> TransformUniformBuffer;
		std::shared_ptr<UniformBuffer> LightingUniformBuffer;
		uint32_t LightDataPackingBufferSize;
		uint8_t* LightDataPackingBuffer;

		//RadialLight data
		unsigned int m_RadialLightSubmissionCount = 0;
		glm::vec2 m_RadialLightPositionBuffer[MAX_NUM_RADIAL_LIGHTS] = {};
		glm::vec3 m_RadialLightColorBuffer[MAX_NUM_RADIAL_LIGHTS] = {};
		float m_RadialLightIntensityBuffer[MAX_NUM_RADIAL_LIGHTS] = {};

		//SpotLightData
		unsigned int m_SpotLightSubmissionCount = 0;
		glm::vec2 m_SpotLightPositionBuffer[MAX_NUM_SPOT_LIGHTS] = {};
		glm::vec3 m_SpotLightColorBuffer[MAX_NUM_SPOT_LIGHTS] = {};
		float m_SpotLightAngleBuffer[MAX_NUM_SPOT_LIGHTS] = {};
		float m_SpotLightInnerCutoffBuffer[MAX_NUM_SPOT_LIGHTS] = {};
		float m_SpotLightOuterCutoffBuffer[MAX_NUM_SPOT_LIGHTS] = {};
		float m_SpotLightIntensityBuffer[MAX_NUM_SPOT_LIGHTS] = {};
	};
}