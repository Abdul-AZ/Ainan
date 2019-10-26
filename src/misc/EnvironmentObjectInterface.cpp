#include <pch.h>

#include "EnvironmentObjectInterface.h"

namespace ALZ {

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

		//we should never reach here
		assert(false);
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
		}

		//we should never reach here
		assert(false);
		return "";
	}
}