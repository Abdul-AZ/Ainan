#pragma once

#include "math/Interpolation.h"

namespace Ainan {
	
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
			X(Custom)
		}
#undef X
		return "";
	}

	//you should or (||) these together if you want multiple flags
	enum class InterpolationSelectorFlags : int
	{
		None              = 0b0000,
		NoLinearMode      = 0b0001,
		NoCubicMode       = 0b0010,
		NoSmoothstepMode  = 0b0100,
		NoCustomMode      = 0b1000
	};

	template<typename T>
	class InterpolationSelector
	{
	public:

		InterpolationSelector(InterpolationSelectorFlags interpolationFlags = InterpolationSelectorFlags::None) :
			Type(InterpolationType::Fixed),
			startPoint(0),
			endPoint(0),
			flags(interpolationFlags)
		{}

		InterpolationSelector(InterpolationType mode, T start, T end, InterpolationSelectorFlags interpolationFlags = InterpolationSelectorFlags::None) :
			Type(mode),
			startPoint(start),
			endPoint(end),
			flags(interpolationFlags)
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
			ImGui::Text(title.c_str());
			ImGui::SameLine();
			if (ImGui::BeginCombo(("##" + title).c_str(), InterpolationTypeToString(Type))) {

				{
					bool is_Active = Type == Fixed;
					if (ImGui::Selectable(InterpolationTypeToString(Fixed), &is_Active)) {

						ImGui::SetItemDefaultFocus();
						Type = Fixed;
					}
				}

				if(!(bool)((int)flags & (int)InterpolationSelectorFlags::NoLinearMode))
				{
					bool is_Active = Type == Linear;
					if (ImGui::Selectable(InterpolationTypeToString(Linear), &is_Active)) {

						ImGui::SetItemDefaultFocus();
						Type = Linear;
					}
				}

				if (!(bool)((int)flags & (int)InterpolationSelectorFlags::NoCubicMode))
				{
					bool is_Active = Type == Cubic;
					if (ImGui::Selectable(InterpolationTypeToString(Cubic), &is_Active)) {

						ImGui::SetItemDefaultFocus();
						Type = Cubic;
					}
				}

				if (!(bool)((int)flags & (int)InterpolationSelectorFlags::NoSmoothstepMode))
				{
					bool is_Active = Type == Smoothstep;
					if (ImGui::Selectable(InterpolationTypeToString(Smoothstep), &is_Active)) {

						ImGui::SetItemDefaultFocus();
						Type = Smoothstep;
					}
				}

				if (!(bool)((int)flags & (int)InterpolationSelectorFlags::NoCustomMode))
				{
					bool is_Active = Type == Custom;
					if (ImGui::Selectable(InterpolationTypeToString(Custom), &is_Active)) {

						ImGui::SetItemDefaultFocus();
						Type = Custom;
					}
				}

				ImGui::EndCombo();
			}
		}


		InterpolationType Type;
		InterpolationSelectorFlags flags;
		T startPoint;
		T endPoint;
	};
}