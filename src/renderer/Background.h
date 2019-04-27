#pragma once

#include "environment/Camera.h"
#include "ShaderProgram.h"
#include "RadialLight.h"

//This should be the same as the one in the Background shader (inside the fragment shader)
#define MAX_NUM_RADIAL_LIGHTS 10

namespace ALZ {

	class Background {
	public:
		Background();

		void SubmitLight(const RadialLight& light);

		void Render(Camera& camera);

	public:
		glm::vec3 BaseColor;
		float BaseLight = 0.2f;

	private:

		unsigned int m_RadialLightSubmissionCount = 0;
		glm::vec2 m_RadialLightPositionBuffer[MAX_NUM_RADIAL_LIGHTS];
		glm::vec3 m_RadialLightColorBuffer[MAX_NUM_RADIAL_LIGHTS];
		float m_RadialLightConstantBuffer[MAX_NUM_RADIAL_LIGHTS];
		float m_RadialLightLinearBuffer[MAX_NUM_RADIAL_LIGHTS];
		float m_RadialLightQuadraticBuffer[MAX_NUM_RADIAL_LIGHTS];
	};
}