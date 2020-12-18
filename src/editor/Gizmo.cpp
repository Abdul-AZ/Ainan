#include "Gizmo.h"

#include <GLFW/glfw3.h>

namespace Ainan {

#define GIZMO_LINE_LENGTH_TILL_TIP 0.25f
#define GIZMO_LINE_LENGTH_TILL_WINGS 0.21f
#define GIZMO_LINE_WIDTH 0.03f
#define GIZMO_LINE_WINGS_HEIGHT 0.0425f
#define GIZMO_LINE_START_X 0.0f
#define GIZMO_LINE_START_Y 0.0f

	//                                            v3
	//                                            |\
	//                                            | \
	//                                            |  \
	//           v1                             v2|   \
	//            |-------------------------------|    \
	//            |                                     \
	//            |                                      | v4
	//            |                                     /
	//            |-------------------------------|    /
	//           v0                             v6|   /
	//                                            |  /
	//                                            | /
	//                                            |/
	//                                            v5
	//

	//in normalized device coordinates -> ranges from -1 to 1
	static const float arrowVertices[] =
	{
		GIZMO_LINE_START_X, GIZMO_LINE_START_Y,  //v0
		0.75f, 0.0f, 0.0f, 1.0f,
		GIZMO_LINE_START_X,  GIZMO_LINE_START_Y + GIZMO_LINE_WIDTH,  //v1
		0.75f, 0.0f, 0.0f, 1.0f,
		GIZMO_LINE_START_X + GIZMO_LINE_LENGTH_TILL_WINGS,  GIZMO_LINE_START_Y + GIZMO_LINE_WIDTH,  //v2
		0.75f, 0.0f, 0.0f, 1.0f,
		GIZMO_LINE_START_X + GIZMO_LINE_LENGTH_TILL_WINGS, GIZMO_LINE_START_Y + GIZMO_LINE_WIDTH + GIZMO_LINE_WINGS_HEIGHT,  //v3
		0.75f, 0.0f, 0.0f, 1.0f,
		GIZMO_LINE_START_X + GIZMO_LINE_LENGTH_TILL_TIP,  GIZMO_LINE_START_Y + GIZMO_LINE_WIDTH / 2.0f,  //v4
		0.75f, 0.0f, 0.0f, 1.0f,
		GIZMO_LINE_START_X + GIZMO_LINE_LENGTH_TILL_WINGS, GIZMO_LINE_START_Y - GIZMO_LINE_WINGS_HEIGHT,  //v5
		0.75f, 0.0f, 0.0f, 1.0f,
		GIZMO_LINE_START_X + GIZMO_LINE_LENGTH_TILL_WINGS, GIZMO_LINE_START_Y,  //v6
		0.75f, 0.0f, 0.0f, 1.0f,

		GIZMO_LINE_START_Y, GIZMO_LINE_START_X,  //v0
		0.0f, 0.75f, 0.0f, 1.0f,
		GIZMO_LINE_START_Y + GIZMO_LINE_WIDTH, GIZMO_LINE_START_X,  //v1
		0.0f, 0.75f, 0.0f, 1.0f,
		GIZMO_LINE_START_Y + GIZMO_LINE_WIDTH, GIZMO_LINE_START_X + GIZMO_LINE_LENGTH_TILL_WINGS,  //v2
		0.0f, 0.75f, 0.0f, 1.0f,
		GIZMO_LINE_START_Y + GIZMO_LINE_WIDTH + GIZMO_LINE_WINGS_HEIGHT, GIZMO_LINE_START_X + GIZMO_LINE_LENGTH_TILL_WINGS,  //v3
		0.0f, 0.75f, 0.0f, 1.0f,
		GIZMO_LINE_START_Y + GIZMO_LINE_WIDTH / 2.0f, GIZMO_LINE_START_X + GIZMO_LINE_LENGTH_TILL_TIP,  //v4
		0.0f, 0.75f, 0.0f, 1.0f,
		GIZMO_LINE_START_Y - GIZMO_LINE_WINGS_HEIGHT,GIZMO_LINE_START_X + GIZMO_LINE_LENGTH_TILL_WINGS,  //v5
		0.0f, 0.75f, 0.0f, 1.0f,
		GIZMO_LINE_START_Y, GIZMO_LINE_START_X + GIZMO_LINE_LENGTH_TILL_WINGS,   //v6
		0.0f, 0.75f, 0.0f, 1.0f
	};

	static const uint32_t c_OpenGLArrowIndecies[] =
	{
		0,1,2,
		2,3,4,
		4,5,6,
		6,4,2,
		0,2,6,

		7 ,8 ,9 ,
		9 ,10,11,
		11,12,13,
		13,11,9 ,
		7 ,9 ,13
	};

	static const uint32_t c_DirectXArrowIndecies[] =
	{
		0,1,2,
		2,3,4,
		4,5,6,
		2,4,6,
		0,2,6,

		7 ,9 ,8 ,
		9 ,11,10,
		11,13,12,
		9 ,13,11,
		7 ,13,9
	};

	Gizmo::Gizmo()
	{
		VertexLayout layout(2);
		layout[0] = VertexLayoutElement("POSITION", 0, ShaderVariableType::Vec2);
		layout[1] = VertexLayoutElement("NORMAL", 0, ShaderVariableType::Vec4);
		m_VertexBuffer = Renderer::CreateVertexBufferNew((void*)arrowVertices, sizeof(arrowVertices), layout, Renderer::ShaderLibraryNew()["GizmoShader"]);

		if(Renderer::Rdata->CurrentActiveAPI->GetContext()->GetType() == RendererType::OpenGL)
			m_IndexBuffer = Renderer::CreateIndexBufferNew((uint32_t*)c_OpenGLArrowIndecies, sizeof(c_OpenGLArrowIndecies) / sizeof(uint32_t));
		else
			m_IndexBuffer = Renderer::CreateIndexBufferNew((uint32_t*)c_DirectXArrowIndecies, sizeof(c_DirectXArrowIndecies) / sizeof(uint32_t));

		m_UniformBuffer = Renderer::CreateUniformBufferNew("ObjectTransform", 1,
			{ 
				VertexLayoutElement("u_Position", 0, ShaderVariableType::Vec2),
				VertexLayoutElement("u_AspectRatio", 1, ShaderVariableType::Float),
				VertexLayoutElement("u_OpacityR", 2, ShaderVariableType::Float),
				VertexLayoutElement("u_OpacityG", 3, ShaderVariableType::Float) 
			}
		);
	}

	void Gizmo::Draw(glm::vec2* objectPosition,
		const ViewportWindow& window,
		const Camera& camera)
	{
		//later used for drawing
		TransformationData data;
		data.Position = camera.WorldSpaceToViewportNDC(*objectPosition);
		data.AspectRatio = window.WindowContentRegionSize.y / window.WindowContentRegionSize.x;

		//get mouse position
		double xpos, ypos;
		glfwGetCursorPos(Window::Ptr, &xpos, &ypos);
		//the GetFrameHeightWithSpacing on the next line is just an approximation for the top border if the window
		//if someone knows a better way please tell me, glfwGetFrameSize doesnt work and I'm tired of banging my head with the WindowsAPI
		//with all of its outdated functions and ones that give incorrect results
		glm::vec2 windowPos = window.WindowPosition - glm::vec2{Window::Position.x, Window::Position.y - ImGui::GetFrameHeightWithSpacing() };

		//transform mouse coordinates to viewport NDC
		float xposNDC = ((xpos - windowPos.x) / window.WindowSize.x) * 2.0f - 1.0f;
		float yposNDC = (((ypos - windowPos.y + ImGui::GetFrameHeightWithSpacing() - ImGui::GetFrameHeight()) / window.WindowSize.y) );

		//change range to not include outside frame of the window
		yposNDC *= (1.0f + ImGui::GetFrameHeightWithSpacing() / window.WindowSize.y);
		yposNDC = -(yposNDC * 2.0f - 1.0f);

		//if mouse is hovering over the viewport, handle events
		if (xposNDC > -1.0f && xposNDC < 1.0f &&
			yposNDC > -1.0f && yposNDC < 1.0f)
			HandleInteration(xposNDC, yposNDC, objectPosition, camera, data);

		//Draw the triangles
		auto& shader = Renderer::ShaderLibraryNew()["GizmoShader"];

		shader.BindUniformBuffer(m_UniformBuffer, 1, RenderingStage::VertexShader);
		m_UniformBuffer.UpdateData(&data, sizeof(TransformationData));

		Renderer::Draw(m_VertexBuffer, shader, Primitive::Triangles, m_IndexBuffer);
	}

	//we are treating arrow hitboxes as rectangles for now
	void Gizmo::HandleInteration(float xposNDC, float yposNDC, glm::vec2* objectPosition, const Camera& camera, TransformationData& data)
	{
		static int32_t pressCount = 0;
		if (glfwGetMouseButton(Window::Ptr, GLFW_MOUSE_BUTTON_LEFT))
			pressCount++;
		else
		{
			pressCount = 0;
			m_CurrentlyPressedArrow = None;
		}

		//check inside horizontal arrow
		{
			//get rectangle size
			glm::vec2 rectangleSize = glm::vec2(0.0f);
			rectangleSize.x = GIZMO_LINE_LENGTH_TILL_TIP * data.AspectRatio;
			rectangleSize.y = GIZMO_LINE_WIDTH;

			if (xposNDC > data.Position.x && xposNDC < data.Position.x + rectangleSize.x &&
				yposNDC > data.Position.y && yposNDC < data.Position.y + rectangleSize.y)
			{
				if (pressCount == 1)
				{
					m_CurrentlyPressedArrow = Horizontal;
					m_DistanceBetweenMouseAndArrowNDC = xposNDC - data.Position.x;
				}
				data.OpacityR = 1.25f;
			}
			if (m_CurrentlyPressedArrow == Horizontal)
				data.OpacityR = 2.0f;
		}

		//check inside vertical arrow
		{
			//get rectangle size
			glm::vec2 rectangleSize = glm::vec2(0.0f);
			rectangleSize.x = GIZMO_LINE_WIDTH * data.AspectRatio;
			rectangleSize.y = GIZMO_LINE_LENGTH_TILL_TIP;

			if (xposNDC > data.Position.x && xposNDC < data.Position.x + rectangleSize.x &&
				yposNDC > data.Position.y && yposNDC < data.Position.y + rectangleSize.y)
			{
				if (pressCount == 1)
				{
					m_CurrentlyPressedArrow = Vertical;
					m_DistanceBetweenMouseAndArrowNDC = yposNDC - data.Position.y;
				}
				data.OpacityG = 1.25f;
			}
			if (m_CurrentlyPressedArrow == Vertical)
				data.OpacityG = 2.0f;
		}

		switch (m_CurrentlyPressedArrow)
		{
		case Horizontal:
		{
			glm::vec2 newPos = camera.ViewportNDCToWorldSpace(glm::vec2(xposNDC - m_DistanceBetweenMouseAndArrowNDC, yposNDC));
			objectPosition->x = newPos.x;
		}
		break;
		case Vertical:
		{
			glm::vec2 newPos = camera.ViewportNDCToWorldSpace(glm::vec2(xposNDC, yposNDC - m_DistanceBetweenMouseAndArrowNDC));
			objectPosition->y = newPos.y;
		}
		break;
		}
	}
}
