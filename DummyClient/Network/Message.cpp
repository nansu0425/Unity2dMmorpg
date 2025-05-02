/*    DummyClient/Network/Message.cpp    */

#include "DummyClient/Pch.h"
#include "DummyClient/Network/Message.h"

ServerMessageHandlerManager    gMessageHandlerManager;

ServerMessageHandlerManager::ServerMessageHandlerManager()
{
    RegisterHandler(static_cast<MessageId>(ServerMessageId::Test), [this](SharedPtr<Session> session, ReceiveMessage message)
                    {
                        return HandleTest(session, flatbuffers::GetRoot<MessageData::Server::Test>(message.GetData()));
                    });

    RegisterHandler(static_cast<MessageId>(ServerMessageId::Login), [this](SharedPtr<Session> session, ReceiveMessage message)
                    {
                        return HandleLogin(session, flatbuffers::GetRoot<MessageData::Server::Login>(message.GetData()));
                    });
}

Bool ServerMessageHandlerManager::HandleTest(SharedPtr<Session> session, const MessageData::Server::Test* data)
{
    gLogger->Debug(TEXT_8("Test Message: id={}, hp={}, attack={}"), data->id(), data->hp(), data->attack());
    
    for (UInt32 i = 0; i < data->buffs()->size(); ++i)
    {
        gLogger->Debug(TEXT_8("Buff {}: id={}, remain_time={}")
                       , i
                       , data->buffs()->Get(i)->id()
                       , data->buffs()->Get(i)->remain_time());
        for (UInt32 j = 0; j < data->buffs()->Get(i)->victims()->size(); ++j)
        {
            gLogger->Debug(TEXT_8("Victim {}: {}"), j, data->buffs()->Get(i)->victims()->Get(j));
        }
    }

    return true;
}

Bool ServerMessageHandlerManager::HandleLogin(SharedPtr<Session> session, const MessageData::Server::Login* data)
{
    Bool result = false;

    if (data->status() == MessageData::LoginStatus_Success)
    {
        gLogger->Info(TEXT_8("Login Success: id={}"), data->id()->c_str());
        result = true;
    }
    else
    {
        gLogger->Error(TEXT_8("Login Failed: id={}"), data->id()->c_str());
        result = false;
    }

    return result;
}
