#pragma once

#include "editor/EditorCamera.h"
#include "ImGuizmo.h"

namespace Ainan {

	enum EnvironmentObjectType 
	{
		ModelType,
		ParticleSystemType,
		RadialLightType,
		SpotLightType,
		SpriteType,
		LitSpriteType, 
		CameraType
	};

	const float c_ObjectPositionDragControlSpeed = 0.1f;
	const float c_ObjectScaleDragControlSpeed = 0.01f;
	const float c_ObjectRotationDragControlSpeed = 1.0f;

	EnvironmentObjectType StringToEnvironmentObjectType(const std::string& type);
	std::string EnvironmentObjectTypeToString(EnvironmentObjectType type);

	enum ObjSpace
	{
		OBJ_SPACE_2D,
		OBJ_SPACE_3D
	};

	class EnvironmentObjectInterface
	{
	public:
		EnvironmentObjectInterface();

		//this function does no graphics works (no OpenGL calls)
		virtual void Update(const float deltaTime) {};
		virtual void Draw() {};
		virtual void DisplayGuiControls() {};
		virtual void OnTransform() {};
		virtual std::shared_ptr<std::mutex> GetMutex() { return ObjectMutex; };

		virtual ~EnvironmentObjectInterface() {};
		virtual int32_t GetAllowedGizmoOperation(ImGuizmo::OPERATION requestedOperation) { return requestedOperation; }

		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		ObjSpace Space = OBJ_SPACE_3D;
		std::string m_Name;
		bool RenameTextOpen = false;
		bool Selected = false;
		int Order = 0; //this should ONLY be used and edited by the Environment class and NOT be used by inherited classes.

		//set this to true if you want to delete the object because it can't be deleted at certain times
		bool ToBeDeleted = false;

		EnvironmentObjectType Type;

	protected:
		void DisplayTransformationControls();

	private:
		std::shared_ptr<std::mutex> ObjectMutex;
	};

	const char* ObjSpaceToStr(ObjSpace space);
	ObjSpace StrToObjSpace(const char* str);

	using pEnvironmentObject = std::unique_ptr<EnvironmentObjectInterface>;
}
