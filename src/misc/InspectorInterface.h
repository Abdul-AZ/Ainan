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
		//this function does no graphics works (no OpenGL calls)
		virtual void Update(const float& deltaTime) {};
		//this function updates uniform buffers (does OpenGL calls)
		virtual void UpdateUniforms(Camera& camera) {};
		virtual void Draw() {};
		virtual void DisplayGUI(Camera& camera) {};

		virtual ~InspectorInterface() {};

		std::string m_Name;
		bool EditorOpen = false;
		bool RenameTextOpen = false;
		bool Selected = false;
		int ID;

		//set this to true if you want to delete the object because it can't be deleted at certain times
		bool ToBeDeleted = false;

		InspectorObjectType Type;
	};
}
