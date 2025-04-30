/*    DummyClient/Network/Packet.cpp    */

#include "DummyClient/Pch.h"
#include "DummyClient/Network/Packet.h"
#include "ServerEngine/Network/Session.h"
#include "Common/MessageData/Generated/Server_generated.h"

MessageHandlerManager    gMessageHandlerManager;

MessageHandlerManager::MessageHandlerManager()
{
    Init();
}

void MessageHandlerManager::Init()
{
    for (Int16 i = 0; i < std::numeric_limits<Int16>::max(); ++i)
    {
        mHandlers[i] = [this](SharedPtr<PacketSession> session, Byte* message, Int64 size)
            {
                return HandleInvalid(std::move(session), message, size);
            };
    }

    mHandlers[static_cast<Int16>(ServerMessageId::Test)] = [this](SharedPtr<PacketSession> session, Byte* message, Int64 size)
        {
            return HandleTest(session, flatbuffers::GetRoot<MessageData::Server::Test>(message + SIZE_64(PacketHeader)));
        };
}

Bool MessageHandlerManager::HandleMessage(SharedPtr<PacketSession> session, Byte* message, Int64 size)
{
    PacketHeader* header = reinterpret_cast<PacketHeader*>(message);
    // 메시지 id에 해당하는 핸들러 호출
    return mHandlers[header->id](std::move(session), message, size);
}

Bool MessageHandlerManager::HandleInvalid(SharedPtr<PacketSession> session, Byte* message, Int64 size)
{
    gLogger->Error(TEXT_16("Invalid message: id={}, size={}"), reinterpret_cast<PacketHeader*>(message)->id, size);
    return true;
}

Bool MessageHandlerManager::HandleTest(SharedPtr<PacketSession> session, const MessageData::Server::Test* data)
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
