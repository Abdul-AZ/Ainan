#include "InterpolationSelector.h"

namespace Ainan {

	void DisplayInterpolationTypeSelector(InterpolationType& type, InterpolationSelectorFlags flags, void* id)
	{
		ImGui::PushID(id);
		if (ImGui::BeginCombo("##Interpolation", InterpolationTypeToString(type)))
		{
			{
				bool is_Active = type == InterpolationType::Fixed;
				if (ImGui::Selectable(InterpolationTypeToString(InterpolationType::Fixed), &is_Active)) {

					ImGui::SetItemDefaultFocus();
					type = InterpolationType::Fixed;
				}
			}

			if (!(bool)((int)flags & (int)InterpolationSelectorFlags::NoLinearMode))
			{
				bool is_Active = type == InterpolationType::Linear;
				if (ImGui::Selectable(InterpolationTypeToString(InterpolationType::Linear), &is_Active)) {

					ImGui::SetItemDefaultFocus();
					type = InterpolationType::Linear;
				}
			}

			if (!(bool)((int)flags & (int)InterpolationSelectorFlags::NoCubicMode))
			{
				bool is_Active = type == InterpolationType::Cubic;
				if (ImGui::Selectable(InterpolationTypeToString(InterpolationType::Cubic), &is_Active)) {

					ImGui::SetItemDefaultFocus();
					type = InterpolationType::Cubic;
				}
			}

			if (!(bool)((int)flags & (int)InterpolationSelectorFlags::NoSmoothstepMode))
			{
				bool is_Active = type == InterpolationType::Smoothstep;
				if (ImGui::Selectable(InterpolationTypeToString(InterpolationType::Smoothstep), &is_Active)) {

					ImGui::SetItemDefaultFocus();
					type = InterpolationType::Smoothstep;
				}
			}

			if (!(bool)((int)flags & (int)InterpolationSelectorFlags::NoCustomMode))
			{
				bool is_Active = type == InterpolationType::Custom;
				if (ImGui::Selectable(InterpolationTypeToString(InterpolationType::Custom), &is_Active)) {

					ImGui::SetItemDefaultFocus();
					type = InterpolationType::Custom;
				}
			}

			ImGui::EndCombo();
		}
		ImGui::PopID();
	}
}
