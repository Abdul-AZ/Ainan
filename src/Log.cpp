#include "Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::logger> AinanLogger;

void InitAinanLogger()
{
	AinanLogger = spdlog::stdout_color_mt("AinanLogger");
	AinanLogger->set_pattern("[%T] [%s:%#] %l: %^%v%$");
	//spdlog::sinks::stdout_color_sink_mt* color_sink = (spdlog::sinks::stdout_color_sink_mt*)&(*AinanLogger->sinks()[0]);
	//color_sink->set_color(spdlog::level::info, color_sink->CYAN);
}
