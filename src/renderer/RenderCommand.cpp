#include "RenderCommand.h"

namespace Ainan {

	const char* RenderingBlendModeToStr(RenderingBlendMode mode)
	{
		switch (mode)
		{
		case RenderingBlendMode::Additive:
			return "Additive";
			break;
		case RenderingBlendMode::Screen:
			return "Screen";
			break;
		case RenderingBlendMode::Overlay:
			return "Overlay";
			break;
		case RenderingBlendMode::NotSpecified:
		default:
			AINAN_LOG_FATAL("Unkown blend mode passed");
			return "";
			break;
		}
	}

	RenderingBlendMode StrToRenderingBlendMode(std::string str)
	{
		if (str == "Additive")
			return RenderingBlendMode::Additive;
		else if (str == "Screen")
			return RenderingBlendMode::Screen;
		else if (str == "Overlay")
			return RenderingBlendMode::Overlay;

		AINAN_LOG_FATAL("Unkown blend mode passed");
		return RenderingBlendMode::NotSpecified;
	}
}