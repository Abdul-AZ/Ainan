#pragma once

#include "math/Interpolation.h"

namespace ALZ {
	
	//convertes InterpolationType enum to string just to make the code cleaner
	constexpr const char* InterpolationTypeToString(InterpolationType e) noexcept
	{
#define X(name) case(name): return #name;
		switch (e)
		{
			X(Fixed)
			X(Linear)
			X(Cubic)
			X(Smoothstep)
		}
#undef X
		return "";
	}

	template<typename T>
	class InterpolationSelector
	{
	public:

		InterpolationSelector() :
			Type(InterpolationType::Fixed),
			startPoint(0),
			endPoint(0)
		{}

		InterpolationSelector(InterpolationType mode, T start, T end) :
			Type(mode),
			startPoint(start),
			endPoint(end)
		{}

		T Interpolate(const float& t) {
			T diffrence = endPoint - startPoint;

			switch (Type)
			{
			case Fixed:
				return startPoint;

			case Linear:
				return Interpolation::Linear(startPoint, endPoint, t);

			case Cubic:
				return Interpolation::Cubic(startPoint, endPoint, t);

			case Smoothstep:
				return Interpolation::Smoothstep(startPoint, endPoint, t);
			default:
				return startPoint;
			}
		}

		//shows a drop box that has the interpolation options
		void DisplayGUI(const std::string& title)
		{
			if (ImGui::BeginCombo(title.c_str(), InterpolationTypeToString(Type))) {

				{
					bool is_Active = Type == Fixed;
					if (ImGui::Selectable(InterpolationTypeToString(Fixed), &is_Active)) {

						ImGui::SetItemDefaultFocus();
						Type = Fixed;
					}
				}

				{
					bool is_Active = Type == Linear;
					if (ImGui::Selectable(InterpolationTypeToString(Linear), &is_Active)) {

						ImGui::SetItemDefaultFocus();
						Type = Linear;
					}
				}

				{
					bool is_Active = Type == Cubic;
					if (ImGui::Selectable(InterpolationTypeToString(Cubic), &is_Active)) {

						ImGui::SetItemDefaultFocus();
						Type = Cubic;
					}
				}

				{
					bool is_Active = Type == Smoothstep;
					if (ImGui::Selectable(InterpolationTypeToString(Smoothstep), &is_Active)) {

						ImGui::SetItemDefaultFocus();
						Type = Smoothstep;
					}
				}

				ImGui::EndCombo();
			}
		}


		InterpolationType Type;

		T startPoint;
		T endPoint;
	};
}