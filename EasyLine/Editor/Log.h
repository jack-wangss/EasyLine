#pragma once

#include <memory>
#include <spdlog/spdlog.h>

namespace EasyLine {

class Log {
public:
    static void Init();

    inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
    inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

private:
    static std::shared_ptr<spdlog::logger> s_CoreLogger;
    static std::shared_ptr<spdlog::logger> s_ClientLogger;
};

} // namespace EasyLine

// Core logger macros
#define EL_CORE_TRACE(...)    ::EasyLine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define EL_CORE_INFO(...)     ::EasyLine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define EL_CORE_WARN(...)     ::EasyLine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define EL_CORE_ERROR(...)    ::EasyLine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define EL_CORE_FATAL(...)    ::EasyLine::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client logger macros
#define EL_TRACE(...)         ::EasyLine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define EL_INFO(...)          ::EasyLine::Log::GetClientLogger()->info(__VA_ARGS__)
#define EL_WARN(...)          ::EasyLine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define EL_ERROR(...)         ::EasyLine::Log::GetClientLogger()->error(__VA_ARGS__)
#define EL_FATAL(...)         ::EasyLine::Log::GetClientLogger()->critical(__VA_ARGS__)
