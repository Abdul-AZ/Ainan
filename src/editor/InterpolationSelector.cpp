#include "InterpolationSelector.h"

namespace Ainan {

	void DisplayInterpolationTypeSelector(InterpolationType& type, InterpolationSelectorFlags flags, void* id)
	{
		ImGui::PushID(id);
		if (ImGui::BeginCombo("##Interpolation", InterpolationTypeToString(type)))
		{
			{
				bool is_Active = type == Fixed;
				if (ImGui::Selectable(InterpolationTypeToString(Fixed), &is_Active)) {

					ImGui::SetItemDefaultFocus();
					type = Fixed;
				}
			}

			if (!(bool)((int)flags & (int)InterpolationSelectorFlags::NoLinearMode))
			{
				bool is_Active = type == Linear;
				if (ImGui::Selectable(InterpolationTypeToString(Linear), &is_Active)) {

					ImGui::SetItemDefaultFocus();
					type = Linear;
				}
			}

			if (!(bool)((int)flags & (int)InterpolationSelectorFlags::NoCubicMode))
			{
				bool is_Active = type == Cubic;
				if (ImGui::Selectable(InterpolationTypeToString(Cubic), &is_Active)) {

					ImGui::SetItemDefaultFocus();
					type = Cubic;
				}
			}

			if (!(bool)((int)flags & (int)InterpolationSelectorFlags::NoSmoothstepMode))
			{
				bool is_Active = type == Smoothstep;
				if (ImGui::Selectable(InterpolationTypeToString(Smoothstep), &is_Active)) {

					ImGui::SetItemDefaultFocus();
					type = Smoothstep;
				}
			}

			if (!(bool)((int)flags & (int)InterpolationSelectorFlags::NoCustomMode))
			{
				bool is_Active = type == Custom;
				if (ImGui::Selectable(InterpolationTypeToString(Custom), &is_Active)) {

					ImGui::SetItemDefaultFocus();
					type = Custom;
				}
			}

			ImGui::EndCombo();
		}
		ImGui::PopID();
	}
}