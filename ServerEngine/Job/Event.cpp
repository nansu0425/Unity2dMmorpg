/*    ServerEngine/Job/Event.cpp    */

#include "ServerEngine/Pch.h"
#include "ServerEngine/Job/Event.h"

/**
 * JobPushEventPool 생성자
 * 락프리 스택을 초기화하고 이벤트 풀을 미리 할당합니다.
 */
JobPushEventPool::JobPushEventPool()
{
    // SLIST 헤더 초기화
    ::InitializeSListHead(&mFreeList);

    // 일정량의 이벤트를 미리 할당
    for (int i = 0; i < kInitPoolSize; i++)
    {
        ListNode* node = new ListNode();
        ::InterlockedPushEntrySList(&mFreeList, &(node->ListEntry));
    }
}

/**
 * JobPushEventPool 소멸자
 * 풀에 남아있는 모든 이벤트를 해제합니다.
 */
JobPushEventPool::~JobPushEventPool()
{
    // 풀에 남아있는 모든 이벤트 해제
    while (PSLIST_ENTRY entry = ::InterlockedPopEntrySList(&mFreeList))
    {
        ListNode* node = CONTAINING_RECORD(entry, ListNode, ListEntry);
        delete node;
    }
}

/**
 * 이벤트를 풀에서 가져옵니다.
 *
 * @return 사용 가능한 JobPushEvent 개체
 */
JobPushEvent* JobPushEventPool::Pop()
{
    PSLIST_ENTRY entry = ::InterlockedPopEntrySList(&mFreeList);

    if (entry == nullptr)
    {
        // 새 노드 생성
        ListNode* node = new ListNode();
        return NodeToEvent(node);
    }

    ListNode* node = CONTAINING_RECORD(entry, ListNode, ListEntry);
    return NodeToEvent(node);
}

/**
 * 이벤트를 풀에 반환합니다.
 *
 * @param event 반환할 JobPushEvent 개체
 */
void JobPushEventPool::Push(JobPushEvent* event)
{
    ASSERT_CRASH_DEBUG(event && (event->owner == nullptr), "INVALID_EVENT");

    // 이벤트 초기화
    event->Init();

    // 노드 얻기 및 스택에 반환
    ListNode* node = EventToNode(event);
    ::InterlockedPushEntrySList(&mFreeList, &(node->ListEntry));
}
