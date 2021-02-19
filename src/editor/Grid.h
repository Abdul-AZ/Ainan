#pragma once

#include "renderer/ShaderProgram.h"
#include "renderer/Renderer.h"
#include "Camera.h"
#include "Window.h"

namespace Ainan {

	class Grid
	{
	public:
		enum GridPlane
		{
			XY,
			XZ,
			YZ
		};
		Grid(float unitLength = 100.0f, int32_t numLinesPerAxis = 51);
		~Grid();
		void Draw(const Camera& camera);
		GridPlane Orientation = XY;

	private:
		float m_UnitLength = 0.0f;
		VertexBuffer m_VertexBuffer;
		IndexBuffer m_IndexBuffer;
		UniformBuffer m_TransformUniformBuffer;

	public:
		static const char* GridPlaneToStr(GridPlane plane)
		{
			switch (plane)
			{
			case XY:
				return "X-Y";
				break;
			case XZ:
				return "X-Z";
				break;
			case YZ:
				return "Y-Z";
				break;
			}

			AINAN_LOG_ERROR("Inavlid Grid Orientation");
			return "";
		}

		static GridPlane StrToGridPlane(const char* str)
		{
			if (str == "X-Y")
				return XY;
			else if (str == "X-Z")
				return XZ;
			else if (str == "Y-Z")
				return YZ;

			AINAN_LOG_ERROR("Inavlid Grid Orientation");
			return XY;
		}
	};
}