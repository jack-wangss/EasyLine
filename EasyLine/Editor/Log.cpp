#include "Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace EasyLine {

std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

namespace {
    std::shared_ptr<spdlog::logger> CreateLogger(const std::string& name, const std::string& filename)
    {
        // Console sink with color
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

        // File sink
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

        // Create sinks array inline as temporary
        auto logger = std::make_shared<spdlog::logger>(name, 
            std::initializer_list<spdlog::sink_ptr>{console_sink, file_sink});
        
        logger->set_level(spdlog::level::trace);
        logger->flush_on(spdlog::level::trace);
        return logger;
    }
}

void Log::Init()
{
    try {
        s_CoreLogger = CreateLogger("EASYLINE", "EasyLine.log");
        spdlog::register_logger(s_CoreLogger);

        s_ClientLogger = CreateLogger("APP", "App.log");
        spdlog::register_logger(s_ClientLogger);

    } catch (const spdlog::spdlog_ex& ex) {
        // If logging initialization fails, fall back to stderr
        fprintf(stderr, "Log init failed: %s\n", ex.what());
    }
}

} // namespace EasyLine
