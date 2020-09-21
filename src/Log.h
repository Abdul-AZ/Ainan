#pragma once

#include <spdlog/spdlog.h>

void InitAinanLogger();
extern std::shared_ptr<spdlog::logger> AinanLogger;

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

//logging macros, prefer these to using spdlog directly
#ifndef NDEBUG
#define AINAN_LOG_INFO(x) {SPDLOG_LOGGER_INFO(AinanLogger.get(), x);}
#define AINAN_LOG_WARNING(x) {SPDLOG_LOGGER_WARN(AinanLogger.get(), x);}
#define AINAN_LOG_ERROR(x) {SPDLOG_LOGGER_ERROR(AinanLogger.get(), x);}
#define AINAN_LOG_FATAL(x) {SPDLOG_LOGGER_ERROR(AinanLogger.get(), x); assert(false);}
#else
#define AINAN_LOG_INFO(x)
#define AINAN_LOG_WARNING(x)
#define AINAN_LOG_ERROR(x)
#define AINAN_LOG_FATAL(x)
#endif