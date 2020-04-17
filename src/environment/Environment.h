#pragma once

#include "misc/EnvironmentObjectInterface.h"
#include "misc/Background.h"
#include "misc/ParticleCustomizer.h"
#include "misc/ExportCamera.h"

namespace Ainan
{
	struct Environment
	{
		//project name
		std::string Name;

		//all the objects in the environement
		std::vector<std::unique_ptr<EnvironmentObjectInterface>> Objects;

		//post-processing data
		RenderingBlendMode BlendMode = RenderingBlendMode::Additive;
		bool BlurEnabled = false;
		float BlurRadius = 1.0f;


		Background m_Background; //TODO
		ParticleCustomizer Customizer;
		ExportCamera m_ExportCamera;
		std::string FolderPath;
	};
}