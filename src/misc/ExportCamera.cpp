#include <pch.h>

#include "ExportCamera.h"

namespace ALZ {

	ExportCamera::ExportCamera()
	{
		memset(m_Edges, 0, sizeof(m_Edges));
		memset(m_OutlineLines, 0, sizeof(m_OutlineLines));
		for (size_t i = 0; i < 4; i++)
			m_OutlineLines[i].Color = { 1.0f,1.0f,1.0f,1.0f };
	}

	void ExportCamera::DrawOutline()
	{
		for (size_t i = 0; i < 4; i++)
			m_OutlineLines[i].Draw();
	}

	void ExportCamera::SetSize(const glm::vec2& bottomLeftPosition, const glm::vec2& size)
	{
		m_Edges[0] = bottomLeftPosition;                                                          //bottom left
		m_Edges[1] = glm::vec2(bottomLeftPosition.x, bottomLeftPosition.y + size.y);              //top left
		m_Edges[2] = glm::vec2(bottomLeftPosition.x + size.x, bottomLeftPosition.y + size.y);     //top right
		m_Edges[3] = glm::vec2(bottomLeftPosition.x + size.x, bottomLeftPosition.y);              //bottom right

		m_OutlineLines[0].SetPoints(m_Edges[0], m_Edges[1]); //bottom left to top left
		m_OutlineLines[1].SetPoints(m_Edges[1], m_Edges[2]); //top left to top right
		m_OutlineLines[2].SetPoints(m_Edges[2], m_Edges[3]); //top right to bottom right
		m_OutlineLines[3].SetPoints(m_Edges[3], m_Edges[0]); //bottom right to bottom left

		RealCamera.Update(0.0f);
		glm::vec2 reversedPos = glm::vec2(-bottomLeftPosition.x, -bottomLeftPosition.y);
		RealCamera.SetPosition(reversedPos * GlobalScaleFactor);
	}
}