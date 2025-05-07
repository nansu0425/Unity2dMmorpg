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
        mItems.push(item);
    }

    void Push(T&& item)
    {
        WRITE_GUARD;
        mItems.push(std::move(item));
    }

    // item을 가져왔다면 true, 비어있다면 false
    Bool Pop(T& item)
    {
        WRITE_GUARD;
        if (false == mItems.empty())
        {
            item = mItems.front();
            mItems.pop();
            return true;
        }
        return false;
    }

    // items를 가져왔다면 true, 비어있다면 false
    Bool PopAll(Vector<T>& items)
    {
        WRITE_GUARD;
        if (false == mItems.empty())
        {
            items.reserve(mItems.size());
            while (false == mItems.empty())
            {
                items.push_back(mItems.front());
                mItems.pop();
            }
            return true;
        }
        return false;
    }

    void Clear()
    {
        WRITE_GUARD;
        mItems.clear();
    }

private:
    RW_LOCK;
    Queue<T>    mItems;
};
