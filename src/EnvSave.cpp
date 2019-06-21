#include <pch.h>

#include "Environment.h"
#include "json/json.hpp"

using json = nlohmann::json;

#define VEC4_TO_JSON_ARRAY(vec) { vec.x, vec.y, vec.z, vec.w }
#define VEC2_TO_JSON_ARRAY(vec) { vec.x, vec.y }

namespace ALZ {

	struct ps_data {
		std::string d;
		std::string d1;
	};

	//forward declarations
	static void toJson(json& j, const ParticleSystem& ps, const int& objectOrder);
	static void toJson(json& j, const RadialLight& ps, const int& objectOrder);
	static void toJson(json& j, const GeneralSettingsGUI& settings);

	bool SaveEnvironment(const Environment& env, std::string path)
	{
		json data;

		//serialize inspector objects count int
		data["objectCount"] = env.InspectorObjects.size();
		
		//serialize inspector objects
		for (size_t i = 0; i < env.InspectorObjects.size(); i++)
		{
			//this means we are serializing a Particle System
			if (env.InspectorObjects[i]->Type == InspectorObjectType::ParticleSystemType)
			{
				toJson(data, *(ParticleSystem*)env.InspectorObjects[i].get(), i);
			}
			//this means we are serializing a Radial Light
			else if (env.InspectorObjects[i]->Type == InspectorObjectType::RadiaLightType)
			{
				toJson(data, *(RadialLight*)env.InspectorObjects[i].get(), i);
			}
		}
		toJson(data, env.m_Settings);

		std::cout << data.dump(4);

		return true;
	}

	void toJson(json& j, const ParticleSystem& ps, const int& objectOrder)
	{
		std::string id = "obj" + std::to_string(objectOrder) + "_";

		j[id + "Type"] = "Particle System";
		j[id + "Name"] = ps.m_Name;
		j[id + "Mode"] = GetModeAsText(ps.Customizer.Mode);
		j[id + "SpawnPosition"] = { ps.Customizer.m_SpawnPosition.x, ps.Customizer.m_SpawnPosition.y };
		j[id + "LinePosition"] = { ps.Customizer.m_LinePosition.x, ps.Customizer.m_LinePosition.y };
		j[id + "ParticlesPerSecond"] = ps.Customizer.m_ParticlesPerSecond;

		//Scale data
		j[id + "MinScale"] = ps.Customizer.m_ScaleCustomizer.m_MinScale;
		j[id + "MaxScale"] = ps.Customizer.m_ScaleCustomizer.m_MaxScale;
		j[id + "DefinedScale"] = ps.Customizer.m_ScaleCustomizer.m_DefinedScale;
		j[id + "EndScale"] = ps.Customizer.m_ScaleCustomizer.m_EndScale;
		j[id + "ScaleInterpolationType"] = InterpolationTypeToString(ps.Customizer.m_ScaleCustomizer.m_Interpolator.Type);

		//Color data
		j[id + "DefinedColor"] = VEC4_TO_JSON_ARRAY(ps.Customizer.m_ColorCustomizer.m_DefinedColor);
		j[id + "EndColor"] = VEC4_TO_JSON_ARRAY(ps.Customizer.m_ColorCustomizer.m_EndColor);
		j[id + "ColorInterpolationType"] = InterpolationTypeToString(ps.Customizer.m_ColorCustomizer.m_Interpolator.Type);

		//Lifetime data
		j[id + "DefinedLifetime"] = ps.Customizer.m_LifetimeCustomizer.m_DefinedLifetime;
		j[id + "MinLifetime"] = ps.Customizer.m_LifetimeCustomizer.m_MinLifetime;
		j[id + "MaxLifetime"] = ps.Customizer.m_LifetimeCustomizer.m_MaxLifetime;

		//Velocity data
		j[id + "DefinedVelocity"] = VEC2_TO_JSON_ARRAY(ps.Customizer.m_VelocityCustomizer.m_DefinedVelocity);
		j[id + "MinVelocity"] = VEC2_TO_JSON_ARRAY(ps.Customizer.m_VelocityCustomizer.m_MinVelocity);
		j[id + "MaxVelocity"] = VEC2_TO_JSON_ARRAY(ps.Customizer.m_VelocityCustomizer.m_MaxVelocity);

		//Noise data
		j[id + "NoiseEnabled"] = ps.Customizer.m_NoiseCustomizer.m_NoiseEnabled;
		j[id + "NoiseStrength"] = ps.Customizer.m_NoiseCustomizer.m_NoiseStrength;

		//Texture data
		j[id + "UseDefaultTexture"] = ps.Customizer.m_TextureCustomizer.UseDefaultTexture;
		j[id + "TexturePath"] = ps.Customizer.m_TextureCustomizer.m_FileBrowser.m_CurrentselectedFilePath;
	}

	void toJson(json& j, const RadialLight& ps, const int& objectOrder)
	{
	}

	void toJson(json& j, const GeneralSettingsGUI& settings)
	{
	}
}

#undef VEC4_TO_JSON_ARRAY
#undef VEC2_TO_JSON_ARRAY
