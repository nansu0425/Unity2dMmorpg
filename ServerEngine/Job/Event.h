/*    ServerEngine/Job/Event.h    */

#pragma once

/*
 * JobPushEvent는 IOCP(I/O Completion Port)와 함께 사용되는 이벤트 구조체입니다.
 * OVERLAPPED 구조체를 상속하여 Windows IOCP 시스템에 등록 가능하며,
 * 비동기 작업 처리 결과를 전달하는 매개체로 사용됩니다.
 *
 * 주요 특징:
 * - IOCP를 통한 비동기 작업 처리 지원
 * - JobQueue에 대한 참조 소유권 관리
 * - 빠른 초기화를 위한 Init 메서드 제공
 */
struct JobPushEvent
    : public OVERLAPPED
{
    SharedPtr<JobQueue> owner;

    void Init()
    {
        Internal = 0;
        InternalHigh = 0;
        Offset = 0;
        OffsetHigh = 0;
        hEvent = NULL;
    }

    JobPushEvent()
    {
        Init();
    }
};

/*
 * JobPushEventPool은 JobPushEvent 객체의 효율적인 메모리 관리를 위한 풀 클래스입니다.
 * Windows의 락프리 싱글 링크드 리스트(SLIST)를 사용하여 멀티스레드 환경에서도
 * 최고의 성능을 발휘합니다.
 *
 * 주요 특징:
 * - 락프리 구현으로 높은 동시성 지원
 * - 메모리 정렬(alignment) 요구사항 준수
 * - 최적화된 이벤트 할당 및 반환 메커니즘
 * - 메모리 단편화 방지 및 재사용을 통한 성능 향상
 *
 * 동작 방식:
 * 1. 내부적으로 SLIST를 통해 미사용 이벤트 객체들을 관리
 * 2. Pop() 호출 시 가용한 이벤트 반환, 없으면 새로 할당
 * 3. Push() 호출 시 이벤트를 초기화 후 풀로 반환하여 재사용
 * 4. 메모리 정렬을 위한 특수 노드 구조체 사용
 */
class JobPushEventPool
{
public:
    JobPushEventPool();
    ~JobPushEventPool();

    // 이벤트를 풀에서 가져옴 (Pop)
    JobPushEvent* Pop();

    // 이벤트를 풀에 반환 (Push)
    void Push(JobPushEvent* event);

private:
    // 내부에서만 사용되는 노드 구조체
    struct alignas(MEMORY_ALLOCATION_ALIGNMENT) ListNode
    {
        SLIST_ENTRY ListEntry;      // 반드시 첫 번째 필드여야 함
        JobPushEvent Event;         // 실제 이벤트 데이터
    };

    alignas(MEMORY_ALLOCATION_ALIGNMENT) SLIST_HEADER mFreeList;

    // 노드를 이벤트 포인터로 변환
    JobPushEvent* NodeToEvent(ListNode* node) {
        return &(node->Event);
    }

    // 이벤트 포인터를 노드로 변환
    ListNode* EventToNode(JobPushEvent* event) {
        return CONTAINING_RECORD(event, ListNode, Event);
    }

    // 초기 풀 크기
    static constexpr int kInitPoolSize = 128;
};
