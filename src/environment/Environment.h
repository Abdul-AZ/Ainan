#pragma once

#include "EnvironmentObjectInterface.h"
#include "renderer/Renderer.h"

namespace Ainan
{
	//this is a data only structure, functionality is in the Editor class
	//all data stored in this class is saved and loaded, it is basically the project filetype in Ainan
	struct Environment
	{
		//metadata
		std::string Name;

		//all the objects in the environement
		std::vector<std::unique_ptr<EnvironmentObjectInterface>> Objects;

		//post-processing data
		RenderingBlendMode BlendMode = RenderingBlendMode::Additive;
		bool BlurEnabled = false;
		float BlurRadius = 1.0f;

		//background data
		glm::vec3 BackgroundColor = glm::vec3(0.1f, 0.1f, 0.1f);
		float BackgroundBaseLight = 0.1f;
		float BackgroundConstant = 1.0f;
		float BackgroundLinear = 0.15f;
		float BackgroundQuadratic = 0.02f;
	};
}