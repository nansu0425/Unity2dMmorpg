/*    ServerEngine/Log/Logger.h    */

#pragma once

#include <spdlog/spdlog.h>

class Logger
{
public:
    Logger(String8View name);
    ~Logger();

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
