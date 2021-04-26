#pragma once

#include "renderer/ShaderProgram.h"
#include "renderer/Renderer.h"
#include "EditorCamera.h"
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
		void Draw(const EditorCamera& camera);
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
			if (strcmp(str, "X-Y") == 0)
				return XY;
			else if (strcmp(str, "X-Z") == 0)
				return XZ;
			else if (strcmp(str, "Y-Z") == 0)
				return YZ;

			AINAN_LOG_ERROR("Inavlid Grid Orientation");
			return XY;
		}
	};
}