/*    DummyClient/Network/Message.cpp    */

#include "DummyClient/Pch.h"
#include "DummyClient/Network/Message.h"

ServerMessageHandlerManager    gMessageHandlerManager;

ServerMessageHandlerManager::ServerMessageHandlerManager()
{
    RegisterHandler(ServerMessageId_Test, [this](SharedPtr<Session> session, MessageHeader* header)
                    {
                        return HandleTest(session, flatbuffers::GetRoot<MessageData::Server::Test>(header + 1));
                    });
}

Bool ServerMessageHandlerManager::HandleTest(SharedPtr<Session> session, const MessageData::Server::Test* data)
{
    gLogger->Debug(TEXT_16("Test Message: id={}, hp={}, attack={}"), data->id(), data->hp(), data->attack());
    
    for (UInt32 i = 0; i < data->buffs()->size(); ++i)
    {
        gLogger->Debug(TEXT_16("Buff {}: id={}, remain_time={}")
                       , i
                       , data->buffs()->Get(i)->id()
                       , data->buffs()->Get(i)->remain_time());
        for (UInt32 j = 0; j < data->buffs()->Get(i)->victims()->size(); ++j)
        {
            gLogger->Debug(TEXT_16("Victim {}: {}"), j, data->buffs()->Get(i)->victims()->Get(j));
        }
    }

    return true;
}
