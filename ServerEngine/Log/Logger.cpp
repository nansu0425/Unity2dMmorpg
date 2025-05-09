/*    ServerEngine/Log/Logger.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Log/Logger.h"

#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <io.h>
#include <fcntl.h>

Logger::Logger(String8View name)
{
    Init(name);
}

Logger::~Logger()
{
    Shutdown();
}

void Logger::Init(String8View name)
{
    // 스레드 풀 초기화
    spdlog::init_thread_pool(kQueueSize, kThreadCount);

    // 콘솔 싱크 생성
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    // 로거 생성
    mLogger = std::make_shared<spdlog::async_logger>(String8(name), std::move(consoleSink), spdlog::thread_pool(), spdlog::async_overflow_policy::block);

    // 로거 포맷 설정
    mLogger->set_pattern("[%H:%M:%S] [%^%l%$] %v");

    // 로거 레벨 설정
#ifdef _DEBUG
    mLogger->set_level(spdlog::level::debug);
#else
    mLogger->set_level(spdlog::level::info);
#endif // _DEBUG

    // 경고 이상일 때 즉시 버퍼를 비우고 기록
    mLogger->flush_on(spdlog::level::warn);

    // 콘솔을 UTF-8 모드로 설정
    ::SetConsoleOutputCP(CP_UTF8);
    ::SetConsoleCP(CP_UTF8);
    // CRT stdout을 UTF-8 narrow-text 모드로 전환
    ::_setmode(::_fileno(stdout), _O_U8TEXT);
}

void Logger::Shutdown()
{
    spdlog::shutdown();
}
