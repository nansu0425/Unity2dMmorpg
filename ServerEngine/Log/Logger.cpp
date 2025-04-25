/*    ServerEngine/Log/Logger.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Log/Logger.h"
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>

SharedPtr<Logger> LoggerUtils::CreateLogger()
{
    // 스레드 풀 초기화
    spdlog::init_thread_pool(kQueueSize, kThreadCount);

    // 콘솔 싱크 생성
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    // 로거 생성
    auto logger = std::make_shared<spdlog::async_logger>("Logger", std::move(consoleSink), spdlog::thread_pool(), spdlog::async_overflow_policy::block);

    // 로거 포맷 설정
    logger->set_pattern("[%H:%M:%S] [%^%l%$] %v");

    // 로거 레벨 설정
#ifdef _DEBUG
    logger->set_level(spdlog::level::debug);
#else
    logger->set_level(spdlog::level::info);
#endif // _DEBUG

    // 경고 이상일 때 즉시 버퍼를 비우고 기록
    logger->flush_on(spdlog::level::warn);

    return logger;
}

void LoggerUtils::ShutdownLogger()
{
    spdlog::shutdown();
}
