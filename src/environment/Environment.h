#pragma once

#include "EnvironmentObjectInterface.h"
#include "renderer/Renderer.h"

namespace Ainan
{
	//this is a data only structure, functionality is in the Editor class
	//all data stored in this class is saved and loaded, it is basically the project filetype in Ainan
	struct Environment
	{
		//returns the index in of the object in Objects
		//returns -1 if the object is not found
		int32_t FindObjectByID(const UUID& id);

		//metadata
		std::string Name;

		//all the objects in the environement
		std::vector<std::unique_ptr<EnvironmentObjectInterface>> Objects;

		//post-processing data
		RenderingBlendMode BlendMode = RenderingBlendMode::Additive;
		bool BlurEnabled = false;
		float BlurRadius = 1.0f;

		static Environment Default()
		{
			Environment env;
			env.Name = "New Environment";
			return env;
		}
	};
}