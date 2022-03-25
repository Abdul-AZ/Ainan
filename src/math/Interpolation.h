#pragma once

//NOTE: t is a floating point number from 0 to 1 on all functions.
//		the closer it is to 0, the closer the result is from the 'start' argument.
//		the closer it is to 1, the closer the result is from the 'end' argument.

namespace Ainan {

	enum class InterpolationType
	{
		Fixed,
		Linear,
		Cubic,
		Smoothstep,
		Custom
	};

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

	namespace Interpolation {

		template<typename type>
		type Interporpolate(InterpolationType interpolationType, const type& start, const type& end, float t)
		{
			switch (interpolationType)
			{
			case InterpolationType::Fixed:
				return start;
				break;

			case InterpolationType::Linear:
				return Linear(start, end, t);
				break;

			case InterpolationType::Cubic:
				return Cubic(start, end, t);
				break;

			case InterpolationType::Smoothstep:
				return Smoothstep(start, end, t);
				break;

			case InterpolationType::Custom:
			default:

				assert(false);
				return start;
				break;
			}
		}

		//Basic linear interpolation
		template<typename type>
		type Linear(const type& start, const type& end, float t)
		{
			if (t >= 1.0f)
				return end;
			else if (t <= 0.0f)
				return start;

			type diffrence = end - start;
			return start + diffrence * t;
		}

		//Cubic interpolation
		template<typename type>
		type Cubic(const type& start, const type& end, float t)
		{
			if (t >= 1.0f)
				return end;
			else if (t <= 0.0f)
				return start;

			type diffrence = end - start;
			return start + diffrence * (t * t * t);
		}

		//Smoothstep interpolation
		template<typename type>
		type Smoothstep(const type& start, const type& end, float t)
		{
			if (t >= 1.0f)
				return end;
			else if (t <= 0.0f)
				return start;

			type diffrence = end - start;
			return start + diffrence * (t * t * (3 - 2 * t));
		}

	}
}
