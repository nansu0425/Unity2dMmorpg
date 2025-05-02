/*    GameServer/Network/Message.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Network/Message.h"
#include "GameServer/Network/Session.h"

ClientMessageHandlerManager    gMessageHandlerManager;

ClientMessageHandlerManager::ClientMessageHandlerManager()
{
    RegisterHandler(static_cast<MessageId>(ClientMessageId::Login), [this](SharedPtr<Session> session, ReceiveMessage message)
                    {
                        return HandleLogin(session, flatbuffers::GetRoot<MessageData::Client::Login>(message.GetData()));
                    });
}

Bool ClientMessageHandlerManager::HandleLogin(SharedPtr<Session> session, const MessageData::Client::Login* data)
{
    SharedPtr<SendMessageBuilder> message = std::make_shared<SendMessageBuilder>(static_cast<MessageId>(ServerMessageId::Login));
    flatbuffers::FlatBufferBuilder& dataBuilder = message->GetDataBuilder();

    // 로그인 처리
    std::static_pointer_cast<GameSession>(session)->SetLogined();
    gLogger->Info(TEXT_8("Login Message: id={}, password={}"), data->id()->c_str(), data->password()->c_str());

    auto id = dataBuilder.CreateString(data->id()->c_str());
    auto login = MessageData::Server::CreateLogin(dataBuilder, MessageData::LoginStatus_Success, id);

    message->FinishBuilding(login);
    session->Send(std::move(message));

    return true;
}
