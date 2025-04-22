/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Network/Listener.h"
#include "ServerEngine/Io/Dispatcher.h"

Bool gIsQuit = false;

int main()
{
    mi_version();

    SharedPtr<Listener> listener = std::make_shared<Listener>();
    listener->StartAccept(NetAddress(TEXT_16("127.0.0.1"), 7777));

    // 입출력 이벤트 전달 스레드 생성
    for (Int64 i = 0; i < std::thread::hardware_concurrency() / 2; ++i)
    {
        gThreadManager->Launch([=]()
                               {
                                   Int64 result = SUCCESS;

                                   while ((gIsQuit == false) &&
                                          (result == SUCCESS))
                                   {
                                       result = gIoEventDispatcher.Dispatch();
                                   }
                               });
    }

    // q 입력 대기
    std::cout << "Press 'q' to quit..." << std::endl;
    Char8 input;
    while (true)
    {
        std::cin >> input;
        if (input == 'q')
        {
            gIsQuit = true;
            break;
        }
    }

    return 0;
}
