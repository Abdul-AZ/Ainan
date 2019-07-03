#pragma once

#include "renderer/Renderer.h"
#include "Line.h"
#include "Camera.h"

namespace ALZ {

	class ExportCamera {
	public:
		ExportCamera();
		void DrawOutline();
		void SetSize(const glm::vec2& bottomLeftPosition,const glm::vec2& size);

		Camera RealCamera;
		RenderSurface m_RenderSurface;
	private:
		glm::vec2 m_Edges[4];
		Line m_OutlineLines[4];
	};
}