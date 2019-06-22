#include <pch.h>

#include "Environment.h"
#include "json/json.hpp"

using json = nlohmann::json;

#define JSON_ARRAY_TO_VEC4(arr) glm::vec4(arr[0], arr[1], arr[2], arr[3])
#define JSON_ARRAY_TO_VEC3(arr) glm::vec3(arr[0], arr[1], arr[2])
#define JSON_ARRAY_TO_VEC2(arr) glm::vec2(arr[0], arr[1])

namespace ALZ {

	static InterpolationType StringToInterpolationType(const std::string& type)
	{
		if (type == "Fixed")
			return InterpolationType::Fixed;
		else if (type == "Linear")
			return InterpolationType::Linear;
		else if (type == "Cubic")
			return InterpolationType::Cubic;
		else if (type == "Smoothstep")
			return InterpolationType::Smoothstep;
		else
		{
			assert(false);
			return InterpolationType::Fixed;
		}
	}



	static void ParticleSystemFromJson(Environment* env, json& data, std::string id);
	static void RadialLightFromJson(Environment* env,json& data, std::string id);
	static void SettingsFromJson(Environment* env, json& data);

	Environment* LoadEnvironment(const std::string& path)
	{
		json data = json::parse(FileManager::ReadEntireTextFile(path));
		Environment* env = new Environment();
		env->InspectorObjects.clear();

		SettingsFromJson(env, data);

		int objectCount = data["objectCount"].get<int>();

		for (size_t i = 0; i < objectCount; i++)
		{
			std::string id = "obj" + std::to_string(i) + "_";
			std::string type = data[id + "Type"].get<std::string>();
			if (type == "Particle System")
				ParticleSystemFromJson(env, data, id);
			else if (type == "Radial Light")
				RadialLightFromJson(env, data, id);
			else
				assert(false);
		}

		return env;
	}

	void SettingsFromJson(Environment* env, json& data)
	{
		env->m_Settings.BlurEnabled = data["BlurEnabled"].get<bool>();
		env->m_Settings.BlurScale = data["BlurScale"].get<float>();
		env->m_Settings.BlurStrength = data["BlurStrength"].get<float>();
		env->m_Settings.BlurGaussianSigma = data["BlurGaussianSigma"].get<float>();

		env->m_Settings.ShowGrid = data["ShowGrid"].get<bool>();
	}

	void ParticleSystemFromJson(Environment* env, json& data, std::string id)
	{
		//create particle system
		std::unique_ptr<ParticleSystem> ps = std::make_unique<ParticleSystem>();

		//populate with data

		ps->m_Name = data[id + "Name"].get<std::string>();
		ps->Customizer.Mode = GetTextAsMode(data[id + "Mode"].get<std::string>());
		ps->Customizer.m_ParticlesPerSecond = data[id + "ParticlesPerSecond"].get<float>();
		ps->Customizer.m_SpawnPosition = JSON_ARRAY_TO_VEC2(data[id + "SpawnPosition"].get<std::vector<float>>());
		ps->Customizer.m_LinePosition = JSON_ARRAY_TO_VEC2(data[id + "LinePosition"].get<std::vector<float>>());
		ps->Customizer.m_LineLength = data[id + "LineLength"].get<float>();
		ps->Customizer.m_LineAngle = data[id + "LineAngle"].get<float>();
		ps->Customizer.m_CircleOutline.Position = JSON_ARRAY_TO_VEC2(data[id + "CirclePosition"].get<std::vector<float>>());
		ps->Customizer.m_CircleOutline.Radius = data[id + "CircleRadius"].get<float>();

		//Scale data
		ps->Customizer.m_ScaleCustomizer.m_MinScale = data[id + "MinScale"].get<float>();
		ps->Customizer.m_ScaleCustomizer.m_MaxScale = data[id + "MaxScale"].get<float>();
		ps->Customizer.m_ScaleCustomizer.m_DefinedScale = data[id + "DefinedScale"].get<float>();
		ps->Customizer.m_ScaleCustomizer.m_EndScale = data[id + "EndScale"].get<float>();
		ps->Customizer.m_ScaleCustomizer.m_Interpolator.Type = StringToInterpolationType(data[id + "ScaleInterpolationType"].get<std::string>());
		

		//Color data
		ps->Customizer.m_ColorCustomizer.m_DefinedColor = JSON_ARRAY_TO_VEC4(data[id + "DefinedColor"].get<std::vector<float>>());
		ps->Customizer.m_ColorCustomizer.m_EndColor = JSON_ARRAY_TO_VEC4(data[id + "EndColor"].get<std::vector<float>>());
		ps->Customizer.m_ColorCustomizer.m_Interpolator.Type = StringToInterpolationType(data[id + "ColorInterpolationType"].get<std::string>());
		

		//Lifetime data
		ps->Customizer.m_LifetimeCustomizer.m_DefinedLifetime = data[id + "DefinedLifetime"].get<float>();
		ps->Customizer.m_LifetimeCustomizer.m_MinLifetime = data[id + "MinLifetime"].get<float>();
		ps->Customizer.m_LifetimeCustomizer.m_MaxLifetime = data[id + "MaxLifetime"].get<float>();

		//Velocity data
		ps->Customizer.m_VelocityCustomizer.m_DefinedVelocity = JSON_ARRAY_TO_VEC2(data[id + "DefinedVelocity"].get<std::vector<float>>());
		ps->Customizer.m_VelocityCustomizer.m_MinVelocity = JSON_ARRAY_TO_VEC2(data[id + "MinVelocity"].get<std::vector<float>>());
		ps->Customizer.m_VelocityCustomizer.m_MaxVelocity = JSON_ARRAY_TO_VEC2(data[id + "MaxVelocity"].get<std::vector<float>>());

		//Noise data
		ps->Customizer.m_NoiseCustomizer.m_NoiseEnabled = data[id + "NoiseEnabled"].get<bool>();
		ps->Customizer.m_NoiseCustomizer.m_NoiseStrength = data[id + "NoiseStrength"].get<float>();

		//Texture data
		ps->Customizer.m_TextureCustomizer.UseDefaultTexture = data[id + "UseDefaultTexture"].get<bool>();
		ps->Customizer.m_TextureCustomizer.m_FileBrowser.m_CurrentselectedFilePath = data[id + "TexturePath"].get<std::string>();


		//add particle system to environment
		Inspector_obj_ptr startingPSi((InspectorInterface*)(ps.release()));
		env->InspectorObjects.push_back(std::move(startingPSi));
	}

	void RadialLightFromJson(Environment* env, json& data, std::string id)
	{
		//create radial light
		std::unique_ptr<RadialLight> light = std::make_unique<RadialLight>();

		//populate with data
		light->m_Name = data[id + "Name"].get <std::string>();
		light->Position = JSON_ARRAY_TO_VEC2(data[id + "Position"].get<std::vector<float>>());
		light->Color = JSON_ARRAY_TO_VEC3(data[id + "Color"].get<std::vector<float>>());
		light->Constant = data[id + "Constant"].get<float>();
		light->Linear = data[id + "Linear"].get<float>();
		light->Quadratic = data[id + "Quadratic"].get<float>();

		//add radial light to environment
		Inspector_obj_ptr startingPSi((InspectorInterface*)(light.release()));
		env->InspectorObjects.push_back(std::move(startingPSi));
	}
}

#undef JSON_ARRAY_TO_VEC4
#undef JSON_ARRAY_TO_VEC3
#undef JSON_ARRAY_TO_VEC2
