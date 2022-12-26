#pragma once
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <vector>

// #define LOG_WRITE(level, ...)    \
//     Logger::GetInstance().write( \
//         level, "[" + std::string(__FUNCTION__) + "]" + __VA_ARGS__)
// #define LOG_DEBUG(...) LOG_WRITE(spdlog::level::level_enum::debug,
// __VA_ARGS__) #define LOG_INFO(...) LOG_WRITE(spdlog::level::level_enum::info,
// __VA_ARGS__) #define LOG_WARN(...) LOG_WRITE(spdlog::level::level_enum::warn,
// __VA_ARGS__) #define LOG_ERROR(...) LOG_WRITE(spdlog::level::level_enum::err,
// __VA_ARGS__)
// #define LOG_FATAL(...) \
//     LOG_WRITE(spdlog::level::level_enum::critical, __VA_ARGS__)

namespace TSEngine {

class Logger {
public:
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    Logger(Logger &&) = delete;
    Logger &operator=(Logger &&) = delete;
    template <typename... ARGS>
    void write(spdlog::level::level_enum level, ARGS &&...args) {
        logger_->log(level, std::forward<ARGS>(args)...);
    }
    static Logger *GetInstance() {
        static Logger logger;
        return &logger;
    }

private:
    Logger() {
        std::vector<spdlog::sink_ptr> sink_list;
#ifdef _DEBUG
        auto console_sink =
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::debug);
        sink_list.push_back(console_sink);
#endif
        auto basic_sink =
            std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/log.txt");
        basic_sink->set_level(spdlog::level::debug);
        sink_list.push_back(basic_sink);
        logger_ = std::make_shared<spdlog::logger>("log", std::begin(sink_list),
                                                   std::end(sink_list));
        spdlog::register_logger(logger_);
        logger_->flush_on(spdlog::level::err);
        std::chrono::seconds flush_seconds = std::chrono::seconds(1);
        spdlog::flush_every(flush_seconds);
    }
    ~Logger() { spdlog::drop_all(); }
    std::shared_ptr<spdlog::logger> logger_;
};

template <typename... ARGS>
constexpr static void LogDebug(ARGS &&...args) {
    Logger::GetInstance()->write(spdlog::level::level_enum::debug,
                                 std::forward<ARGS>(args)...);
}
template <typename... ARGS>
constexpr static void LogInfo(ARGS &&...args) {
    Logger::GetInstance()->write(spdlog::level::level_enum::info,
                                 std::forward<ARGS>(args)...);
}
template <typename... ARGS>
constexpr static void LogWarn(ARGS &&...args) {
    Logger::GetInstance()->write(spdlog::level::level_enum::warn,
                                 std::forward<ARGS>(args)...);
}
template <typename... ARGS>
constexpr static void LogError(ARGS &&...args) {
    Logger::GetInstance()->write(spdlog::level::level_enum::err,
                                 std::forward<ARGS>(args)...);
}
template <typename... ARGS>
constexpr static void LogFatal(ARGS &&...args) {
    Logger::GetInstance()->write(spdlog::level::level_enum::critical,
                                 std::forward<ARGS>(args)...);
}

}  // namespace TSEngine
