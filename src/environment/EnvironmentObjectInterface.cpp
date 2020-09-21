#include "EnvironmentObjectInterface.h"

namespace Ainan {

	EnvironmentObjectInterface::EnvironmentObjectInterface() :
		ObjectMutex(std::make_shared<std::mutex>())
	{
	}

	EnvironmentObjectType StringToEnvironmentObjectType(const std::string& type)
	{
		if (type == "Particle System")
			return ParticleSystemType;
		else if (type == "Radial Light")
			return RadialLightType;
		else if (type == "Spot Light")
			return SpotLightType;
		else if (type == "Sprite")
			return SpriteType;
		else if (type == "Lit Sprite")
			return LitSpriteType;

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

		case RadialLightType:
			return "Radial Light";

		case SpotLightType:
			return "Spot Light";

		case SpriteType:
			return "Sprite";

		case LitSpriteType:
			return "Lit Sprite";
		}

		//we should never reach here
		AINAN_LOG_ERROR("Invalid object type string");
		return "";
	}
}