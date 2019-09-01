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

	std::string CheckEnvironmentFile(const std::string& path) {

		json data;
		try
		{
			data = json::parse(FileManager::ReadEntireTextFile(path));
			return "";
		}
		catch (const std::exception& e)
		{
			return e.what();
		}
	}

	static void ParticleSystemFromJson(Environment* env, json& data, std::string id);
	static void RadialLightFromJson(Environment* env,json& data, std::string id);
	static void SettingsFromJson(Environment* env, json& data);
	static void BackgroundFromJson(Background& background, json& data);

	Environment* LoadEnvironment(const std::string& path)
	{
		json data = json::parse(FileManager::ReadEntireTextFile(path));

		Environment* env = new Environment();
		env->InspectorObjects.clear();

		SettingsFromJson(env, data);
		BackgroundFromJson(env->m_Background, data);

		int objectCount = data["objectCount"].get<int>();

		for (size_t i = 0; i < objectCount; i++)
		{
			std::string id = "obj" + std::to_string(i) + "_";
			std::string typeStr = data[id + "Type"].get<std::string>();

			EnvironmentObjectType type = StringToInspectorObjectType(typeStr);

			switch (type)
			{
			case ParticleSystemType:
				ParticleSystemFromJson(env, data, id);
				break;

			case RadialLightType:
				RadialLightFromJson(env, data, id);
				break;

			case SpotLightType:
				SpotLightFromJson(env, data, id);
				break;

			case SpriteType:
				SpriteFromJson(env, data, id);
				break;

			default:
				assert(false);
				break;
			}
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

	void BackgroundFromJson(Background& background, json& data)
	{
		background.BaseColor = JSON_ARRAY_TO_VEC3(data["BackgroundColor"].get<std::vector<float>>());
		background.BaseLight = data["BackgroundBaseLight"].get<float>();
		background.Constant = data["BackgroundConstant"].get<float>();
		background.Linear = data["BackgroundLinear"].get<float>();
		background.Quadratic = data["BackgroundQuadratic"].get<float>();
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
		ps->Customizer.m_LifetimeCustomizer.m_RandomLifetime = data[id + "IsLifetimeRandom"].get<bool>();
		ps->Customizer.m_LifetimeCustomizer.m_DefinedLifetime = data[id + "DefinedLifetime"].get<float>();
		ps->Customizer.m_LifetimeCustomizer.m_MinLifetime = data[id + "MinLifetime"].get<float>();
		ps->Customizer.m_LifetimeCustomizer.m_MaxLifetime = data[id + "MaxLifetime"].get<float>();

		//Velocity data
		ps->Customizer.m_VelocityCustomizer.m_RandomVelocity = data[id + "IsStartingVelocityRandom"].get<bool>();
		ps->Customizer.m_VelocityCustomizer.m_DefinedVelocity = JSON_ARRAY_TO_VEC2(data[id + "DefinedVelocity"].get<std::vector<float>>());
		ps->Customizer.m_VelocityCustomizer.m_MinVelocity = JSON_ARRAY_TO_VEC2(data[id + "MinVelocity"].get<std::vector<float>>());
		ps->Customizer.m_VelocityCustomizer.m_MaxVelocity = JSON_ARRAY_TO_VEC2(data[id + "MaxVelocity"].get<std::vector<float>>());
		ps->Customizer.m_VelocityCustomizer.CurrentVelocityLimitType =  StringToLimitType(data[id + "VelocityLimitType"].get<std::string>());
		ps->Customizer.m_VelocityCustomizer.m_MinNormalVelocityLimit = data[id + "MinNormalVelocityLimit"].get<float>();
		ps->Customizer.m_VelocityCustomizer.m_MaxNormalVelocityLimit = data[id + "MaxNormalVelocityLimit"].get<float>();
		ps->Customizer.m_VelocityCustomizer.m_MinPerAxisVelocityLimit = JSON_ARRAY_TO_VEC2(data[id + "MinPerAxisVelocityLimit"].get<std::vector<float>>());
		ps->Customizer.m_VelocityCustomizer.m_MaxPerAxisVelocityLimit = JSON_ARRAY_TO_VEC2(data[id + "MaxPerAxisVelocityLimit"].get<std::vector<float>>());

		//Noise data
		ps->Customizer.m_NoiseCustomizer.m_NoiseEnabled = data[id + "NoiseEnabled"].get<bool>();
		ps->Customizer.m_NoiseCustomizer.m_NoiseStrength = data[id + "NoiseStrength"].get<float>();

		//Texture data
		ps->Customizer.m_TextureCustomizer.UseDefaultTexture = data[id + "UseDefaultTexture"].get<bool>();
		ps->Customizer.m_TextureCustomizer.m_FileBrowser.m_CurrentselectedFilePath = data[id + "TexturePath"].get<std::string>();
		if (!ps->Customizer.m_TextureCustomizer.UseDefaultTexture)
		{
			ps->Customizer.m_TextureCustomizer.ParticleTexture = Renderer::CreateTexture();
			ps->Customizer.m_TextureCustomizer.ParticleTexture->SetImage(Image::LoadFromFile(ps->Customizer.m_TextureCustomizer.m_FileBrowser.m_CurrentselectedFilePath));
			//ps->Customizer.m_TextureCustomizer.ParticleTexture.Init(ps->Customizer.m_TextureCustomizer.m_FileBrowser.m_CurrentselectedFilePath, 4);
		}

		//Force data
		size_t forceCount = data[id + "Force Count"].get<size_t>();

		//delete the default force
		ps->Customizer.m_ForceCustomizer.m_Forces.erase("Gravity");

		for (size_t i = 0; i < forceCount; i++)
		{
			//make a new force with the data from the json file
			Force force;
			ps->Customizer.m_ForceCustomizer.m_Forces[data[id + "Force" + std::to_string(i).c_str() + "Key"].get<std::string>()] = force;
			Force& currentForce = ps->Customizer.m_ForceCustomizer.m_Forces[data[id + "Force" + std::to_string(i).c_str() + "Key"].get<std::string>()];

			currentForce.Enabled = data[id + "Force" + std::to_string(i).c_str() + "Enabled"].get<bool>();
			currentForce.Type = Force::StringToForceType(data[id + "Force" + std::to_string(i).c_str() + "Type"].get<std::string>());
			currentForce.DF_Value = JSON_ARRAY_TO_VEC2(data[id + "Force" + std::to_string(i).c_str() + "DF_Value"].get<std::vector<float>>());
			currentForce.RF_Target = JSON_ARRAY_TO_VEC2(data[id + "Force" + std::to_string(i).c_str() + "RF_Target"].get<std::vector<float>>());
			currentForce.RF_Strength = data[id + "Force" + std::to_string(i).c_str() + "RF_Strength"].get<float>();
		}
		
		//add particle system to environment
		pEnvironmentObject startingPSi((EnvironmentObjectInterface*)(ps.release()));
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
		light->Intensity = data[id + "Intensity"].get<float>();

		//add radial light to environment
		pEnvironmentObject startingPSi((EnvironmentObjectInterface*)(light.release()));
		env->InspectorObjects.push_back(std::move(startingPSi));
	}

	void SpotLightFromJson(Environment* env, json& data, std::string id)
	{
		//create radial light
		std::unique_ptr<SpotLight> light = std::make_unique<SpotLight>();

		//populate with data
		light->m_Name = data[id + "Name"].get <std::string>();
		light->Position = JSON_ARRAY_TO_VEC2(data[id + "Position"].get<std::vector<float>>());
		light->Color = JSON_ARRAY_TO_VEC3(data[id + "Color"].get<std::vector<float>>());
		light->OuterCutoff = data[id + "OuterCutoff"].get<float>();
		light->InnerCutoff = data[id + "InnerCutoff"].get<float>();
		light->Intensity = data[id + "Intensity"].get<float>();

		//add radial light to environment
		pEnvironmentObject obj((EnvironmentObjectInterface*)(light.release()));
		env->InspectorObjects.push_back(std::move(obj));
	}

	void SpriteFromJson(Environment* env, json& data, std::string id)
	{
		//create sprite
		std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();

		//populate with data
		sprite->m_Name = data[id + "Name"].get<std::string>();
		sprite->Position = JSON_ARRAY_TO_VEC2(data[id + "Position"].get<std::vector<float>>());
		sprite->Scale = JSON_ARRAY_TO_VEC2(data[id + "Scale"].get<std::vector<float>>());
		sprite->Rotation = data[id + "Rotation"].get<float>();
		sprite->Tint = JSON_ARRAY_TO_VEC4(data[id + "Tint"].get<std::vector<float>>());
		sprite->LoadTextureFromFile(data[id + "TextureImagePath"].get<std::string>());

		pEnvironmentObject obj((EnvironmentObjectInterface*)(sprite.release()));
		env->InspectorObjects.push_back(std::move(obj));
	}
}

#undef JSON_ARRAY_TO_VEC4
#undef JSON_ARRAY_TO_VEC3
#undef JSON_ARRAY_TO_VEC2
