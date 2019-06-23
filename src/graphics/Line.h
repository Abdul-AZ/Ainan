#pragma once

#include "ShaderProgram.h"
#include "Camera.h"
#include "Window.h"

namespace ALZ {

	class Line
	{
	public:
		Line();

		void Render(Camera& camera);
		float GetSlope();
		float GetYIntercept();
		glm::vec2 GetPointInLine(const float& t);
		void SetPoints(glm::vec2 startPoint, glm::vec2 endPoint);
		void SetPoints(glm::vec2 linePosition, float lineLength, float lineAngle); //angle in degrees NOT radians

	public:
		float Width = 2.0f;
		glm::vec4 Color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);


	private:
		void UpdateBufferWithPoints();

	private:
		glm::vec2 m_StartPoint;
		glm::vec2 m_EndPoint;

		friend class ParticleCustomizer;
	};
}