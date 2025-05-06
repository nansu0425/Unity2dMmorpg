/*    DummyClient/Network/Message.cpp    */

#include "DummyClient/Pch.h"
#include "DummyClient/Network/Message.h"
#include "ServerEngine/Network/Session.h"

ServerMessageHandlerManager    gMessageHandlerManager;

void ServerMessageHandlerManager::RegisterAllHandlers()
{
    RegisterHandler(MESSAGE_ID(ServerMessageId::Login), [this](SharedPtr<Session> session, ReceiveMessage message)
                    {
                        return HandleLogin(session, flatbuffers::GetRoot<MessageData::Server::Login>(message.GetData()));
                    });

    RegisterHandler(MESSAGE_ID(ServerMessageId::EnterGame), [this](SharedPtr<Session> session, ReceiveMessage message)
                    {
                        return HandleEnterGame(session, flatbuffers::GetRoot<MessageData::Server::EnterGame>(message.GetData()));
                    });

    RegisterHandler(MESSAGE_ID(ServerMessageId::Chat), [this](SharedPtr<Session> session, ReceiveMessage message)
                    {
                        return HandleChat(session, flatbuffers::GetRoot<MessageData::Server::Chat>(message.GetData()));
                    });
}

Bool ServerMessageHandlerManager::HandleLogin(SharedPtr<Session> session, const MessageData::Server::Login* data)
{
    gLogger->Info(TEXT_8("Login result: {}"), data->success() ? TEXT_8("Success") : TEXT_8("Fail"));

    // 플레이어 정보 출력
    for (UInt32 i = 0; i < data->players()->size(); ++i)
    {
        auto player = data->players()->Get(i);
        gLogger->Info(TEXT_8("Player: id={}, name={}, type={}"), player->id(), player->name()->c_str(), static_cast<Int32>(player->type()));
    }

    // 첫 번째 캐릭터로 게임 입장
    SharedPtr<SendMessageBuilder> message = std::make_shared<SendMessageBuilder>(MESSAGE_ID(ClientMessageId::EnterGame));
    auto& dataBuilder = message->GetDataBuilder();
    auto enterGame = MessageData::Client::CreateEnterGame(dataBuilder, 0);
    message->FinishBuilding(enterGame);
    session->Send(std::move(message));

    return true;
}

Bool ServerMessageHandlerManager::HandleEnterGame(SharedPtr<Session> session, const MessageData::Server::EnterGame* data)
{
    // 방 입장 성공 여부 출력
    gLogger->Info(TEXT_8("Enter game result: {}"), data->success() ? TEXT_8("Success") : TEXT_8("Fail"));

    return true;
}

Bool ServerMessageHandlerManager::HandleChat(SharedPtr<Session> session, const MessageData::Server::Chat* data)
{
    // 채팅 메시지와 플레이어 id 출력
    gLogger->Info(TEXT_8("Chat: id={}, message={}"), data->player_id(), data->message()->c_str());

    return true;
}
