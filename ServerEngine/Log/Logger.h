/*    ServerEngine/Log/Logger.h    */

#pragma once

class LoggerUtils
{
public:
    static SharedPtr<Logger>    CreateLogger();
    static void                 ShutdownLogger();

private:
    enum Constants : Int64
    {
        kQueueSize      = 8192,
        kThreadCount    = 1,
    };
};
