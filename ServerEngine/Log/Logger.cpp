/*    ServerEngine/Log/Logger.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Log/Logger.h"

#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <io.h>
#include <fcntl.h>

/**
 * Logger 생성자
 *
 * 지정된 이름으로 새 로거 인스턴스를 초기화합니다.
 *
 * @param name 로거의 식별 이름
 */
Logger::Logger(String8View name)
{
    Init(name);
}

/**
 * Logger 소멸자
 *
 * 로거 리소스를 정리하고 spdlog 라이브러리를 종료합니다.
 */
Logger::~Logger()
{
    Shutdown();
}

/**
 * 로거 초기화
 *
 * 비동기 로깅을 위한 스레드 풀 생성, 로거 인스턴스 설정 및
 * 콘솔 출력 설정을 초기화합니다.
 *
 * 주요 설정:
 * - 비동기 로깅 스레드 풀 초기화
 * - 컬러 콘솔 출력 싱크 생성
 * - 로그 포맷 설정 ([시간] [레벨] 메시지)
 * - 디버그/릴리스 모드에 따른 로그 레벨 설정
 * - 콘솔을 UTF-8 모드로 설정
 *
 * @param name 로거의 식별 이름
 */
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

    // warn 이상일 때 즉시 버퍼를 비우고 기록
    mLogger->flush_on(spdlog::level::warn);

    // 콘솔을 UTF-8 모드로 설정
    ::SetConsoleOutputCP(CP_UTF8);
    ::SetConsoleCP(CP_UTF8);

    // CRT stdout을 UTF-8 narrow-text 모드로 전환
    Int32 result = ::_setmode(::_fileno(stdout), _O_U8TEXT);
    ASSERT_CRASH(result != -1, "Failed to set stdout mode to UTF-8.");
}

/**
 * 로거 종료
 *
 * spdlog 라이브러리의 모든 리소스를 정리합니다.
 * 프로그램 종료 전 로그 버퍼가 완전히 플러시되도록 보장합니다.
 */
void Logger::Shutdown()
{
    spdlog::shutdown();
}
