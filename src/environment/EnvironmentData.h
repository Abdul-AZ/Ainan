#pragma once

#include "misc/EnvironmentObjectInterface.h"
#include "misc/GeneralSettingsGUI.h"
#include "misc/Background.h"
#include "misc/ParticleCustomizer.h"
#include "misc/ExportCamera.h"

namespace Ainan
{
	struct EnvironmentData
	{
		std::string Name;
		std::vector<std::unique_ptr<EnvironmentObjectInterface>> Objects;
		GeneralSettingsGUI m_Settings; //TODO
		Background m_Background; //TODO
		ParticleCustomizer Customizer;
		ExportCamera m_ExportCamera;
		std::string FolderPath;
	};
}