#pragma once

//NOTE: t goes from 0 to 1 on all algorithms.
//		if t is 0 start is returned and if t is 1 end is returned.

namespace Ainan {

	enum InterpolationType
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

		//Basic linear interpolation
		template<typename type>
		type Linear(const type& start, const type& end, const float& t)
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
		type Cubic(const type& start, const type& end, const float& t)
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
		type Smoothstep(const type& start, const type& end, const float& t)
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