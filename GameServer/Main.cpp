/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"

constexpr Int64 gThreadCount = 4;
UInt64 gExecTime[gThreadCount] = {};
constexpr Int64 gMemoryCount = 10'000;
void* gMemory[gMemoryCount] = {};
Int64 gMemorySize[gMemoryCount] = {};
constexpr Int64 gTestCount = 1000;

class Resource_Block
{
public:
    static void* operator new(size_t size)
    {
        return BlockMemoryAllocator::Alloc(size);
    }

    static void operator delete(void* memory) noexcept
    {
        BlockMemoryAllocator::Free(memory, sizeof(Resource_Block));
    }

private:
    UInt64   mA = 0xAAAA'AAAA;
    UInt64   mB = 0xBBBB'BBBB;
    UInt64   mC = 0xCCCC'CCCC;
    UInt64   mD = 0xDDDD'DDDD;
};

class Resource_Malloc
{
private:
    UInt64   mA = 0xAAAA'AAAA;
    UInt64   mB = 0xBBBB'BBBB;
    UInt64   mC = 0xCCCC'CCCC;
    UInt64   mD = 0xDDDD'DDDD;
};

void BlockMemoryPoolTest(Int64 idx)
{
    // 실행 시간 측정 시작
    auto start = std::chrono::high_resolution_clock::now();
    for (Int32 i = 0; i < gTestCount; ++i)
    {
        // std::vector<std::unique_ptr<Resource_Block>, BlockMemoryAllocator::ContainerAdapter<std::unique_ptr<Resource_Block>>> resources;

        for (Int64 i = gMemoryCount / gThreadCount * idx; i < gMemoryCount / gThreadCount * (idx + 1); ++i)
        {
            // resources.push_back(std::make_unique<Resource_Block>());
            // 1~4096 범위의 랜덤 사이즈 메모리 할당
            gMemorySize[i] = rand() % 4096 + 1;
            gMemory[i] = BlockMemoryAllocator::Alloc(gMemorySize[i]);
        }

        for (Int64 i = gMemoryCount / gThreadCount * idx; i < gMemoryCount / gThreadCount * (idx + 1); ++i)
        {
            BlockMemoryAllocator::Free(gMemory[i], gMemorySize[i]);
            gMemory[i] = nullptr;
        }
    }
    // 실행 시간 측정 종료
    auto end = std::chrono::high_resolution_clock::now();
    gExecTime[idx] = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

void MallocTest(Int64 idx)
{
    // 실행 시간 측정 시작
    auto start = std::chrono::high_resolution_clock::now();
    for (Int64 i = 0; i < gTestCount; ++i)
    {
        // std::vector<std::unique_ptr<Resource_Malloc>> resources;

        for (Int64 i = gMemoryCount / gThreadCount * idx; i < gMemoryCount / gThreadCount * (idx + 1); ++i)
        {
            // resources.push_back(std::make_unique<Resource_Malloc>());
            // 1~4096 범위의 랜덤 사이즈 메모리 할당
            gMemorySize[i] = rand() % 4096 + 1;
            gMemory[i] = ::malloc(gMemorySize[i]);
        }

        for (Int64 i = gMemoryCount / gThreadCount * idx; i < gMemoryCount / gThreadCount * (idx + 1); ++i)
        {
            ::free(gMemory[i]);
            gMemory[i] = nullptr;
        }
    }
    // 실행 시간 측정 종료
    auto end = std::chrono::high_resolution_clock::now();
    gExecTime[idx] = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

int main()
{
    
    for (Int32 i = 0; i < gThreadCount; ++i)
    {
        gThreadManager->Launch([i]()
                               {
                                   BlockMemoryPoolTest(i);
                               });
    }
    gThreadManager->Join();
    // 실행 시간 평균 계산 후 출력
    UInt64 totalTime = 0;
    for (Int32 i = 0; i < gThreadCount; ++i)
    {
        totalTime += gExecTime[i];
        gExecTime[i] = 0;
    }
    UInt64 avgTime = totalTime / gThreadCount;
    std::cout << "BlockMemoryPoolTest average time: " << avgTime << " ms" << std::endl;

    for (Int32 i = 0; i < gThreadCount; ++i)
    {
        gThreadManager->Launch([i]()
                               {
                                   MallocTest(i);
                               });
    }
    gThreadManager->Join();
    // 실행 시간 평균 계산 후 출력
    totalTime = 0;
    for (Int32 i = 0; i < gThreadCount; ++i)
    {
        totalTime += gExecTime[i];
        gExecTime[i] = 0;
    }
    avgTime = totalTime / gThreadCount;
    std::cout << "MallocTest average time: " << avgTime << " ms" << std::endl;

    // UniquePtr<Resource_Block> resource = std::make_unique<Resource_Block>();

    return 0;
}
