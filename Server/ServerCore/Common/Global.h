/*    ServerCore/Common/Global.h    */

#pragma once

extern class Logger*                gLogger;
extern class ThreadManager*         gThreadManager;
extern class DeadlockDetector*      gDeadlockDetector;
extern class SendChunkPool*         gSendChunkPool;
extern class JobQueueManager*       gJobQueueManager;
extern class JobTimer*              gJobTimer;

/*
 * GlobalContext 클래스는 서버 엔진의 핵심 글로벌 객체들을 관리합니다.
 * 서버 시작 시 필요한 모든 글로벌 컴포넌트를 초기화하고,
 * 서버 종료 시 해당 리소스들을 안전하게 정리합니다.
 *
 * 관리하는 주요 컴포넌트:
 * - Logger: 로그 기록
 * - ThreadManager: 스레드 관리
 * - DeadlockDetector: 데드락 감지
 * - SendChunkPool: 네트워크 전송 버퍼 풀
 * - 소켓 초기화 및 정리
 * - JobQueueManager: 작업 큐 관리
 * - JobTimer: 작업 타이머
 *
 * 이 클래스의 전역 인스턴스(gGlobalContext)를 통해
 * 리소스 획득과 해제를 RAII 패턴으로 자동화합니다.
 */
class GlobalContext
{
public:
    GlobalContext();
    ~GlobalContext();
};

extern GlobalContext    gGlobalContext;
