#pragma once

#include "editor/Camera.h"
#include "ImGuizmo.h"

namespace Ainan {

	enum EnvironmentObjectType 
	{
		ParticleSystemType,
		RadialLightType,
		SpotLightType,
		SpriteType,
		LitSpriteType
	};

	const float c_ObjectPositionDragControlSpeed = 0.1f;
	const float c_ObjectScaleDragControlSpeed = 0.01f;
	const float c_ObjectRotationDragControlSpeed = 1.0f;

	EnvironmentObjectType StringToEnvironmentObjectType(const std::string& type);
	std::string EnvironmentObjectTypeToString(EnvironmentObjectType type);

	class EnvironmentObjectInterface
	{
	public:
		EnvironmentObjectInterface();

		//this function does no graphics works (no OpenGL calls)
		virtual void Update(const float deltaTime) {};
		virtual void Draw() {};
		virtual void DisplayGUI() {};
		virtual std::shared_ptr<std::mutex> GetMutex() { return ObjectMutex; };

		virtual ~EnvironmentObjectInterface() {};
		virtual int32_t GetAllowedGizmoOperation(ImGuizmo::OPERATION requestedOperation) { return requestedOperation; }

		glm::mat4 Model = glm::mat4(1.0f);
		std::string m_Name;
		bool EditorOpen = false;
		bool RenameTextOpen = false;
		bool Selected = false;
		int Order = 0; //this should ONLY be used and edited by the Environment class and NOT be used by inherited classes.

		//set this to true if you want to delete the object because it can't be deleted at certain times
		bool ToBeDeleted = false;

		EnvironmentObjectType Type;

	private:
		std::shared_ptr<std::mutex> ObjectMutex;
	};

	enum ObjSpace
	{
		OBJ_SPACE_2D,
		OBJ_SPACE_3D
	};

	const char* ObjSpaceToStr(ObjSpace space);
	ObjSpace StrToObjSpace(const char* str);

	using pEnvironmentObject = std::unique_ptr<EnvironmentObjectInterface>;
}
