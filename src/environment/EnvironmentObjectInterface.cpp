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