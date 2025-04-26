/*    ServerEngine/Log/Logger.h    */

#pragma once

// spdlog가 UTF-16을 지원하도록 설정
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT

#include <spdlog/spdlog.h>

class Logger
{
public:
    Logger(String8View name);
    ~Logger();

    /* 인자가 없는 로그 메시지 */

    template <typename T>
    void Trace(const T& msg)    { mLogger->trace(msg); }
    template <typename T>
    void Debug(const T& msg)    { mLogger->debug(msg); }
    template <typename T>
    void Info(const T& msg)     { mLogger->info(msg); }
    template <typename T>
    void Warn(const T& msg)     { mLogger->warn(msg); }
    template <typename T>
    void Error(const T& msg)    { mLogger->error(msg); }
    template <typename T>
    void Critical(const T& msg) { mLogger->critical(msg); }

    /* TEXT_8 로그 메시지 */

    template <typename... Args>
    void Trace(fmt::format_string<Args...> fmt, Args&&... args)     { mLogger->trace(fmt, std::forward<Args>(args)...); }
    template <typename... Args>
    void Debug(fmt::format_string<Args...> fmt, Args&&... args)     { mLogger->debug(fmt, std::forward<Args>(args)...); }
    template <typename... Args>
    void Info(fmt::format_string<Args...> fmt, Args&&... args)      { mLogger->info(fmt, std::forward<Args>(args)...); }
    template <typename... Args>
    void Warn(fmt::format_string<Args...> fmt, Args&&... args)      { mLogger->warn(fmt, std::forward<Args>(args)...); }
    template <typename... Args>
    void Error(fmt::format_string<Args...> fmt, Args&&... args)     { mLogger->error(fmt, std::forward<Args>(args)...); }
    template <typename... Args>
    void Critical(fmt::format_string<Args...> fmt, Args&&... args)  { mLogger->critical(fmt, std::forward<Args>(args)...); }

    /* TEXT_16 로그 메시지 */

    template <typename... Args>
    void Trace(fmt::wformat_string<Args...> fmt, Args&&... args)    { mLogger->trace(fmt, std::forward<Args>(args)...); }
    template <typename... Args>
    void Debug(fmt::wformat_string<Args...> fmt, Args&&... args)    { mLogger->debug(fmt, std::forward<Args>(args)...); }
    template <typename... Args>
    void Info(fmt::wformat_string<Args...> fmt, Args&&... args)     { mLogger->info(fmt, std::forward<Args>(args)...); }
    template <typename... Args>
    void Warn(fmt::wformat_string<Args...> fmt, Args&&... args)     { mLogger->warn(fmt, std::forward<Args>(args)...); }
    template <typename... Args>
    void Error(fmt::wformat_string<Args...> fmt, Args&&... args)    { mLogger->error(fmt, std::forward<Args>(args)...); }
    template <typename... Args>
    void Critical(fmt::wformat_string<Args...> fmt, Args&&... args) { mLogger->critical(fmt, std::forward<Args>(args)...); }

private:
    void Init(String8View name);
    void Shutdown();

private:
    enum Constants : Int64
    {
        kQueueSize      = 8192,
        kThreadCount    = 1,
    };

private:
    SharedPtr<spdlog::logger>   mLogger;
};
