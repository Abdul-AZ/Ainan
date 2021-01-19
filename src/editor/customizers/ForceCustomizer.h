#pragma once

#include "environment/ExposeToJson.h"
#include "editor/Window.h"

namespace Ainan {

	struct Force 
	{
		enum ForceType 
		{
			DirectionalForce,	//as in only having a direction and magnitude and position has no effect on it
			RelativeForce		//as in relative to some point in space
		};

		glm::vec2 GetEffect(const glm::vec2& particlePos)
		{
			if (Type == DirectionalForce)
				return DF_Value;
			else //it is a relative force
			{
				glm::vec2 dir = glm::normalize(RF_Target - particlePos);
				
				if (isnan(dir.x) || isnan(dir.y))
					return glm::vec2(0.0f);

				return RF_Strength * dir;
			}
		}

		ForceType Type = DirectionalForce;
		//DF stands for DirectionalForce and RF stands for RelativeForce
		glm::vec2 DF_Value = { 0.0f,0.0f };  //ONLY USED IN DirectionalForce MODE
		glm::vec2 RF_Target = { 0.45f,0.45f }; //ONLY USED IN RelativeForce MODE
		float RF_Strength = 10.0f;			 //ONLY USED IN RelativeForce MODE

		bool Enabled = false;

		//helper functions
		static constexpr const char* ForceTypeToString(ForceType type)
		{
			switch (type)
			{
			case DirectionalForce:
				return "Directional Force";
			case RelativeForce:
				return "Relative Force";
			default:
				AINAN_LOG_ERROR("Invalid force enum");
				return "";
			}
		}

		static ForceType StringToForceType(std::string type)
		{
			if (type == "Directional Force")
				return DirectionalForce;
			else if (type == "Relative Force")
				return RelativeForce;

			//if we get here, that means there is an error
			AINAN_LOG_ERROR("Invalid force type string");
			return DirectionalForce;
		}
	};

	class ForceCustomizer
	{
	public:
		ForceCustomizer();
		void DisplayGUI();

	private:
		std::map<std::string, Force> m_Forces;

		bool m_DisplayAddForceSettings = false;
		std::string m_AddForceInputString = "";
		std::string m_CurrentSelectedForceName = "";

		//exposing to render a gizmo on force target position if it is used
		friend class Editor;
		EXPOSE_CUSTOMIZER_TO_JSON
	};
}