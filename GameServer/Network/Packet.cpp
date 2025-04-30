/*    GameServer/Network/Packet.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Network/Packet.h"
#include "ServerEngine/Network/Session.h"

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

SharedPtr<SendBuffer> ServerMessageGenerator::MakeSendBuffer(const flatbuffers::FlatBufferBuilder& fbb, ServerMessageId id)
{
    const Int32 dataSize = static_cast<Int32>(fbb.GetSize());
    const Int32 messageSize = dataSize + SIZE_32(PacketHeader);

    SharedPtr<SendBuffer> buffer = gSendBufferManager->Open(messageSize);
    // 헤더 설정
    PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer->GetBuffer());
    header->size = messageSize;
    header->id = static_cast<Int16>(id);
    // 데이터 복사
    ::memcpy(header + 1, fbb.GetBufferPointer(), dataSize);
    buffer->Close(messageSize);

    return buffer;
}
