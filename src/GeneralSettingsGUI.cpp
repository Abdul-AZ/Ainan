#include "GeneralSettingsGUI.h"

void GeneralSettingsGUI::DisplayGUI()
{
	ImGui::Begin("Settings");

	if(ImGui::Checkbox("Blend", &m_GLBlendEnabled)) {
		if (m_GLBlendEnabled)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
	}

	ImGui::Checkbox("Blur", &m_BlurEnabled);

	if (m_GLBlendEnabled) {

		if (ImGui::TreeNode("Blend Settings:"))
		{
			if (ImGui::BeginCombo("sFactor",GetBlendFuncText(m_Sfactor).c_str())) {

				DisplayAllBlendOptions(m_Sfactor);
				glBlendFunc(m_Sfactor, m_Dfactor);

				ImGui::EndCombo();
			}

			if (ImGui::BeginCombo("dFactor", GetBlendFuncText(m_Dfactor).c_str())) {

				DisplayAllBlendOptions(m_Dfactor);
				glBlendFunc(m_Sfactor, m_Dfactor);

				ImGui::EndCombo();
			}


			ImGui::TreePop();
		}
	}


	if (ImGui::TreeNode("Background:"))
	{
		if (ImGui::ColorPicker4("Background Color", &m_BackgroundColor.r))
			glClearColor(m_BackgroundColor.r, m_BackgroundColor.g, m_BackgroundColor.b, m_BackgroundColor.a);

		ImGui::TreePop();
	}

	ImGui::End();
}

void GeneralSettingsGUI::DisplayAllBlendOptions(GLenum& factor)
{
	DisplayBlendOption(GL_SRC_ALPHA, factor);
	DisplayBlendOption(GL_ONE_MINUS_SRC_ALPHA, factor);
	DisplayBlendOption(GL_DST_ALPHA, factor);
	DisplayBlendOption(GL_ONE_MINUS_DST_ALPHA, factor);
	DisplayBlendOption(GL_SRC_COLOR, factor);
	DisplayBlendOption(GL_ONE_MINUS_SRC_COLOR, factor);
	DisplayBlendOption(GL_DST_COLOR, factor);
	DisplayBlendOption(GL_ONE_MINUS_DST_COLOR, factor);
	DisplayBlendOption(GL_CONSTANT_COLOR, factor);
	DisplayBlendOption(GL_ONE_MINUS_CONSTANT_COLOR, factor);
	DisplayBlendOption(GL_CONSTANT_ALPHA, factor);
	DisplayBlendOption(GL_ONE_MINUS_CONSTANT_ALPHA, factor);
	DisplayBlendOption(GL_SRC_ALPHA_SATURATE, factor);
	DisplayBlendOption(GL_SRC1_ALPHA, factor);
	DisplayBlendOption(GL_ONE_MINUS_SRC1_ALPHA, factor);
	DisplayBlendOption(GL_SRC1_COLOR, factor);
	DisplayBlendOption(GL_ONE_MINUS_SRC1_COLOR, factor);
	DisplayBlendOption(GL_ZERO, factor);
	DisplayBlendOption(GL_ONE, factor);
}

void GeneralSettingsGUI::DisplayBlendOption(const GLenum& option, GLenum& factor)
{
	bool is_Active = m_Sfactor == option;
	if (ImGui::Selectable(GetBlendFuncText(option).c_str(), &is_Active)) {

		ImGui::SetItemDefaultFocus();
		factor = option;
	}
}

std::string GeneralSettingsGUI::GetBlendFuncText(const GLenum & func)
{
	switch (func)
	{
	case GL_ZERO:
		return "Zero";

	case GL_ONE:
		return "One";

	case GL_SRC_COLOR:
		return "Source Color";

	case GL_ONE_MINUS_SRC_COLOR:
		return "1 - Source Color";

	case GL_DST_COLOR:
		return "Destination Color";

	case GL_ONE_MINUS_DST_COLOR:
		return "1 - Destination Color";

	case GL_SRC_ALPHA:
		return "Source Alpha";

	case GL_ONE_MINUS_SRC_ALPHA:
		return "1 - Source Alpha";

	case GL_DST_ALPHA:
		return "Destination Alpha";

	case GL_ONE_MINUS_DST_ALPHA:
		return "1 - Destination Alpha";

	case GL_CONSTANT_COLOR:
		return "Constant Color";

	case GL_ONE_MINUS_CONSTANT_COLOR:
		return "1 - Constant Color";

	case GL_CONSTANT_ALPHA:
		return "Constant Alpha";

	case GL_ONE_MINUS_CONSTANT_ALPHA:
		return "1 - Constant Alpha";

	case GL_SRC_ALPHA_SATURATE:
		return "Source Alpha Saturate";

	case GL_SRC1_COLOR:
		return "Source(1) Color";

	case GL_ONE_MINUS_SRC1_COLOR:
		return "1 - Source(1) Color";

	case GL_SRC1_ALPHA:
		return "Source(1) Alpha";

	case GL_ONE_MINUS_SRC1_ALPHA:
		return "1 - Source(1) Alpha";

	default:
		return std::string();
		break;
	}
}
