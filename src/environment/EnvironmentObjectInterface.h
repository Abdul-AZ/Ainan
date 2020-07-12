#pragma once

#include "editor/Camera.h"

namespace Ainan {

	enum EnvironmentObjectType {
		ParticleSystemType,
		RadialLightType,
		SpotLightType,
		SpriteType
	};

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
		virtual glm::vec2& GetPositionRef() { return glm::vec2(0.0f, 0.0f); };
		virtual std::shared_ptr<std::mutex> GetMutex() { return ObjectMutex; };

		virtual ~EnvironmentObjectInterface() {};

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

	using pEnvironmentObject = std::unique_ptr<EnvironmentObjectInterface>;
}
