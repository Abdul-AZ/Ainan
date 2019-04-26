#pragma once

#include "Camera.h"

namespace ALZ {

	enum InspectorObjectType {
		ParticleSystemType,
		RadiaLightType
	};

	class InspectorInterface
	{
	public:
		virtual void Update(const float& deltaTime, Camera& camera) {};
		virtual void Render(Camera& camera) {};
		virtual void DisplayGUI(Camera& camera) {};

		virtual ~InspectorInterface() {};

		std::string m_Name;
		bool m_EditorOpen = false;
		bool m_RenameTextOpen = false;
		bool m_Selected = false;
		int m_ID;

		InspectorObjectType Type;
	};
}
