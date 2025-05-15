/*    ServerEngine/Concurrency/Queue.h    */

#pragma once

/*
 * 내부적으로 Lock을 사용하여 스레드 안전성을 보장하는 Queue
 */
template<typename T>
class LockQueue
{
public:
    void Push(const T& item)
    {
        WRITE_GUARD;
        mScheduledItems.push(item);
    }

    void Push(T&& item)
    {
        WRITE_GUARD;
        mScheduledItems.push(std::move(item));
    }

    // item을 가져왔다면 true, 비어있다면 false
    Bool Pop(T& item)
    {
        WRITE_GUARD;
        if (false == mScheduledItems.empty())
        {
            item = mScheduledItems.front();
            mScheduledItems.pop();
            return true;
        }
        return false;
    }

    // items를 가져왔다면 true, 비어있다면 false
    Bool PopAll(Vector<T>& items)
    {
        WRITE_GUARD;
        if (false == mScheduledItems.empty())
        {
            items.reserve(mScheduledItems.size());
            while (false == mScheduledItems.empty())
            {
                items.push_back(mScheduledItems.front());
                mScheduledItems.pop();
            }
            return true;
        }
        return false;
    }

    void Clear()
    {
        WRITE_GUARD;
        mScheduledItems = Queue<T>();
    }

private:
    RW_LOCK;
    Queue<T>    mScheduledItems;
};
