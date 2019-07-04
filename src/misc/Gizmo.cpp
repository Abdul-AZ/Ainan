#include <pch.h>

#include "Gizmo.h"

#include <GLFW/glfw3.h>

namespace ALZ {

#define GIZMO_LINE_LENGTH_TILL_TIP 1.0f
#define GIZMO_LINE_LENGTH_TILL_WINGS 0.9f
#define GIZMO_LINE_WIDTH 0.075f
#define GIZMO_LINE_WINGS_HEIGHT 0.1f
#define GIZMO_LINE_START_X 0
#define GIZMO_LINE_START_Y 0

	//                                            v3
	//                                            |\
	//                                            | \
	//                                            |  \
	//           v1                            v2 |   \
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
	static const glm::vec2 arrowVertices[] =
	{
		glm::vec2(GIZMO_LINE_START_X, GIZMO_LINE_START_Y),  //v0
		glm::vec2(GIZMO_LINE_START_X,  GIZMO_LINE_START_Y + GIZMO_LINE_WIDTH),  //v1
		glm::vec2(GIZMO_LINE_START_X + GIZMO_LINE_LENGTH_TILL_WINGS,  GIZMO_LINE_START_Y + GIZMO_LINE_WIDTH),  //v2
		glm::vec2(GIZMO_LINE_START_X + GIZMO_LINE_LENGTH_TILL_WINGS, GIZMO_LINE_START_Y + GIZMO_LINE_WIDTH + GIZMO_LINE_WINGS_HEIGHT),  //v3
		glm::vec2(GIZMO_LINE_START_X + GIZMO_LINE_LENGTH_TILL_TIP,  GIZMO_LINE_START_Y + GIZMO_LINE_WIDTH / 2),  //v4
		glm::vec2(GIZMO_LINE_START_X + GIZMO_LINE_LENGTH_TILL_WINGS, GIZMO_LINE_START_Y - GIZMO_LINE_WINGS_HEIGHT),  //v5
		glm::vec2(GIZMO_LINE_START_X + GIZMO_LINE_LENGTH_TILL_WINGS, GIZMO_LINE_START_Y),  //v6

	};

	static const unsigned int arrowIndecies[] =
	{
		0,1,2,
		2,3,4,
		4,5,6,
		6,4,2,
		0,2,6
	};

	static IndexBuffer* EBO;
	static VertexBuffer* VBO;
	static VertexArray* VAO;
	static ShaderProgram* GizmoShader;

	Gizmo::Gizmo()
	{
		VAO = Renderer::CreateVertexArray().release();
		VAO->Bind();

		VBO = Renderer::CreateVertexBuffer((void*)arrowVertices, sizeof(arrowVertices)).release();
		VBO->Bind();
		VBO->SetLayout({ ShaderVariableType::Vec2 });

		EBO = Renderer::CreateIndexBuffer((unsigned int*)arrowIndecies, sizeof(arrowIndecies) / sizeof(unsigned int)).release();
		EBO->Bind();

		VAO->Unbind();

		GizmoShader = Renderer::CreateShaderProgram("shaders/Gizmo.vert", "shaders/Gizmo.frag").release();
	}

	void Gizmo::Draw(glm::vec2& objectPosition, const glm::vec2& mousePositionNDC)
	{
		glm::vec2 objectPositionWS = objectPosition * GlobalScaleFactor;

		glm::mat4 invView = glm::inverse(Renderer::m_Camera->ViewMatrix);
		glm::mat4 invProj = glm::inverse(Renderer::m_Camera->ProjectionMatrix);

		glm::vec4 result = invView * invProj * glm::vec4(mousePositionNDC.x, mousePositionNDC.y, 0.0f, 1.0f);

		//check collision inside arrow rectangle(the part before the wings)

		bool mouseButtonDown = glfwGetMouseButton(&Window::GetWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

		GizmoArrow mouseOver = None;

		if (result.x > objectPositionWS.x && result.x < objectPositionWS.x + GIZMO_LINE_LENGTH_TILL_WINGS * GlobalScaleFactor / 10.0f &&
			result.y > objectPositionWS.y && result.y < objectPositionWS.y + GIZMO_LINE_WIDTH * GlobalScaleFactor / 10.0f )
		{
			mouseOver = Horizontal;
			if(mouseButtonDown && m_ArrowPressed != Vertical)
				m_ArrowPressed = Horizontal;
		}

		if (result.x > objectPositionWS.x && result.x < objectPositionWS.x + GIZMO_LINE_WIDTH * GlobalScaleFactor / 10.0f &&
			result.y > objectPositionWS.y && result.y < objectPositionWS.y + GIZMO_LINE_LENGTH_TILL_WINGS * GlobalScaleFactor / 10.0f)
		{
			mouseOver = Vertical;
			if(mouseButtonDown && m_ArrowPressed != Horizontal)
				m_ArrowPressed = Vertical;
		}

		if (m_ArrowPressed == Horizontal)
			objectPosition.x = result.x / GlobalScaleFactor;
		else if(m_ArrowPressed == Vertical)
			objectPosition.y = result.y / GlobalScaleFactor;

		if (!mouseButtonDown)
			m_ArrowPressed = None;

		//Draw the triangles
		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(objectPositionWS.x, objectPositionWS.y, 0.0f));
		model = glm::scale(model, glm::vec3(100.0f, 100.0f, 100.0f));

		glm::vec4 color;

		//assign a brighter color if the mouse is over the arrow
		if (m_ArrowPressed == Horizontal || mouseOver == Horizontal)
			color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		else
			color = glm::vec4(0.75f, 0.0f, 0.0f, 1.0f);

		GizmoShader->SetUniformVec4("color", color);
		GizmoShader->SetUniformMat4("model", model);
		Renderer::Draw(*VAO, *GizmoShader, Primitive::Triangles, *EBO);

		model = glm::rotate(model, -PI / 2, glm::vec3(0.0f, 0.0f, -1.0f));

		//assign a brighter color if the mouse is over the arrow
		if (m_ArrowPressed == Vertical || mouseOver == Vertical)
			color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
		else
			color = glm::vec4(0.0f, 0.75f, 0.0f, 1.0f);

		GizmoShader->SetUniformVec4("color", color);
		GizmoShader->SetUniformMat4("model", model);
		glm::vec2 mousePosWS = Renderer::m_Camera->Position + mousePositionNDC * GlobalScaleFactor;
		glm::vec2 objectPosWS = objectPositionWS * GlobalScaleFactor;

		Renderer::Draw(*VAO, *GizmoShader, Primitive::Triangles, *EBO);
	}

}
