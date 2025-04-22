/*    GameServer/Main.cpp    */

#include "GameServer/Pch.h"
#include "ServerEngine/Concurrency/Thread.h"
#include "ServerEngine/Network/Listener.h"
#include "ServerEngine/Io/IoManager.h"

int main()
{
    mi_version();

    Listener* listener = new Listener();
    listener->StartAccept(NetAddress(TEXT_16("127.0.0.1"), 7777));

    for (Int64 i = 0; i < 5; ++i)
    {
        gThreadManager->Launch([=]()
                               {
                                   while (true)
                                   {
                                       gIoManager.DispatchIoEvent();
                                   }
                               });
    }



    gThreadManager->Join();
    delete listener;

    return 0;
}
