#include <pch.h>

#include "InspectorInterface.h"

namespace ALZ {

	InspectorObjectType StringToInspectorObjectType(const std::string& type)
	{
		if (type == "Particle System")
			return ParticleSystemType;
		else if (type == "Radial Light")
			return RadialLightType;
		else if (type == "Spot Light")
			return SpotLightType;

		//we should never reach here
		assert(false);
		return ParticleSystemType;
	}

	std::string InspectorObjectTypeToString(InspectorObjectType type)
	{
		switch (type)
		{
		case ParticleSystemType:
			return "Particle System";

		case RadialLightType:
			return "Radial Light";

		case SpotLightType:
			return "Spot Light";
		}

		//we should never reach here
		assert(false);
		return "";
	}
}