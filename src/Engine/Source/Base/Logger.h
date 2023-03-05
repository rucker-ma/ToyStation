#pragma once
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <vector>

constexpr const char *FilenameWithoutPath(const char *path) {
    const char *result = path;
    while (*path != '\0')
        if (*path++ == '\\') {
            result = path;
        }

    return result;
}

#define LOG_WRITE(log_level, ...)                                             \
    Logger::GetInstance()->write(                                             \
        log_level, "[" + std::string(FilenameWithoutPath(__FILE__)) + ":" + \
                       std::to_string(__LINE__) + "] " + __VA_ARGS__)

#define LogDebug(...) LOG_WRITE(spdlog::level::level_enum::debug, __VA_ARGS__)
#define LogInfo(...) LOG_WRITE(spdlog::level::level_enum::info, __VA_ARGS__)
#define LogWarn(...) LOG_WRITE(spdlog::level::level_enum::warn, __VA_ARGS__)
#define LogError(...) LOG_WRITE(spdlog::level::level_enum::err, __VA_ARGS__)
#define LogFatal(...) \
    LOG_WRITE(spdlog::level::level_enum::critical, __VA_ARGS__)

namespace toystation {
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
        spdlog::set_level(spdlog::level::debug);
        logger_->flush_on(spdlog::level::debug);
        std::chrono::seconds flush_seconds = std::chrono::seconds(1);
        spdlog::flush_every(flush_seconds);
    }
    ~Logger() { spdlog::drop_all(); }
    std::shared_ptr<spdlog::logger> logger_;
    std::vector<spdlog::sink_ptr> sink_list;
};

}  // namespace toystation
