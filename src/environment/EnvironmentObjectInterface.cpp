#include "EnvironmentObjectInterface.h"

namespace Ainan {

	EnvironmentObjectInterface::EnvironmentObjectInterface() :
		ObjectMutex(std::make_shared<std::mutex>())
	{
	}

	void EnvironmentObjectInterface::DisplayTransformationControls()
	{
		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(ModelMatrix, scale, rotation, translation, skew, perspective);

		if (Space == OBJ_SPACE_3D)
		{
			ImGui::Text("Position: ");
			ImGui::NextColumn();
			ImGui::DragFloat3("##Position: ", &ModelMatrix[3][0], c_ObjectPositionDragControlSpeed);

			ImGui::NextColumn();
			ImGui::Text("Rotation: ");
			ImGui::NextColumn();
			glm::vec3 rotEular = glm::eulerAngles(rotation) * 180.0f / glm::pi<float>();
			if (ImGui::DragFloat3("##Rotation: ", &rotEular.x, c_ObjectRotationDragControlSpeed))
			{
				//reconstruct model with new rotation
				ModelMatrix = glm::mat4(1.0f);
				ModelMatrix = glm::translate(ModelMatrix, translation);
				ModelMatrix = ModelMatrix *  glm::mat4_cast(glm::quat(rotEular * glm::pi<float>() / 180.0f));
				ModelMatrix = glm::scale(ModelMatrix, scale);
			}

			ImGui::NextColumn();
			ImGui::Text("Scale: ");
			ImGui::NextColumn();
			if (ImGui::DragFloat3("##Scale: ", &scale.x, c_ObjectScaleDragControlSpeed))
			{
				//reconstruct model with new scale
				ModelMatrix = glm::mat4(1.0f);
				ModelMatrix = glm::translate(ModelMatrix, translation);
				ModelMatrix *= glm::mat4_cast(rotation);
				ModelMatrix = glm::scale(ModelMatrix, scale);
			}
		}
		else if (Space == OBJ_SPACE_2D)
		{
			ImGui::Text("Position: ");
			ImGui::NextColumn();
			ImGui::DragFloat2("##Position: ", &ModelMatrix[3][0], c_ObjectPositionDragControlSpeed);

			ImGui::NextColumn();
			ImGui::Text("Rotation: ");
			ImGui::NextColumn();
			float rotEular = glm::eulerAngles(rotation).z * 180.0f / glm::pi<float>();
			if (ImGui::DragFloat("##Rotation: ", &rotEular, c_ObjectRotationDragControlSpeed))
			{
				//reconstruct model with new rotation
				ModelMatrix = glm::mat4(1.0f);
				ModelMatrix = glm::translate(ModelMatrix, translation);
				ModelMatrix = glm::rotate(ModelMatrix, rotEular * glm::pi<float>() / 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));
				ModelMatrix = glm::scale(ModelMatrix, scale);
			}

			ImGui::NextColumn();
			ImGui::Text("Scale: ");
			ImGui::NextColumn();
			float scaleAverage = (scale.x + scale.y + scale.z) / 3.0f;
			if (ImGui::DragFloat("##Scale: ", &scaleAverage, c_ObjectScaleDragControlSpeed))
			{
				ModelMatrix = glm::scale(ModelMatrix, (1.0f / scale));
				ModelMatrix = glm::scale(ModelMatrix, glm::vec3(scaleAverage));
			}
		}
		else
			AINAN_LOG_FATAL("Invalid object space");
	}

	EnvironmentObjectType StringToEnvironmentObjectType(const std::string& type)
	{
		if (type == "Particle System")
			return ParticleSystemType;
		else if (type == "Sprite")
			return SpriteType;
		else if (type == "Lit Sprite")
			return LitSpriteType;
		else if (type == "Model")
			return ModelType;
		else if (type == "Radial Light")
			return RadialLightType;
		else if (type == "Spot Light")
			return SpotLightType;
		
		//we should never reach here
		AINAN_LOG_ERROR("Invalid object type enum");
		return ParticleSystemType;
	}

	std::string EnvironmentObjectTypeToString(EnvironmentObjectType type)
	{
		switch (type)
		{
		case ParticleSystemType:
			return "Particle System";

		case SpriteType:
			return "Sprite";

		case LitSpriteType:
			return "Lit Sprite";

		case ModelType:
			return "Model";

		case RadialLightType:
			return "Radial Light";

		case SpotLightType:
			return "Spot Light";
		}

		//we should never reach here
		AINAN_LOG_ERROR("Invalid object type string");
		return "";
	}

	const char* ObjSpaceToStr(ObjSpace space)
	{
		switch (space)
		{
		case OBJ_SPACE_2D:
			return "2D";
			break;

		case OBJ_SPACE_3D:
			return "3D";
			break;
		}

		AINAN_LOG_ERROR("Invalid Object Space Requested");
		return "";
	}

	ObjSpace StrToObjSpace(const char* str)
	{
		if (strcmp(str, "2D") == 0)
			return OBJ_SPACE_2D;
		else if (strcmp(str, "3D") == 0)
			return OBJ_SPACE_3D;

		AINAN_LOG_ERROR("Invalid Object Space Requested");
		return OBJ_SPACE_2D;
	}
}