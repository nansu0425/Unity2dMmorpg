/*    GameServer/Network/Message.cpp    */

#include "GameServer/Pch.h"
#include "GameServer/Network/Message.h"
#include "GameServer/Network/Session.h"
#include "GameServer/Content/Player.h"
#include "GameServer/Content/Room.h"

ClientMessageHandlerManager    gMessageHandlerManager;

void ClientMessageHandlerManager::RegisterAllHandlers()
{
    RegisterHandler(MESSAGE_ID(ClientMessageId::Login), [this](SharedPtr<Session> session, ReceiveMessage message)
                    {
                        return HandleLogin(session, flatbuffers::GetRoot<MessageData::Client::Login>(message.GetData()));
                    });

    RegisterHandler(MESSAGE_ID(ClientMessageId::EnterGame), [this](SharedPtr<Session> session, ReceiveMessage message)
                    {
                        return HandleEnterGame(session, flatbuffers::GetRoot<MessageData::Client::EnterGame>(message.GetData()));
                    });

    RegisterHandler(MESSAGE_ID(ClientMessageId::Chat), [this](SharedPtr<Session> session, ReceiveMessage message)
                    {
                        return HandleChat(session, flatbuffers::GetRoot<MessageData::Client::Chat>(message.GetData()));
                    });
}

Bool ClientMessageHandlerManager::HandleLogin(SharedPtr<Session> session, const MessageData::Client::Login* data)
{
    SharedPtr<ClientSession> client = std::static_pointer_cast<ClientSession>(session);

    // TODO: DB에서 플레이어 정보를 가져온다

    // 인게임 id
    static Atomic<Int64> sPlayerId = 1;

    // 플레이어 생성
    SharedPtr<Player> player1 = std::make_shared<Player>(sPlayerId.fetch_add(1), TEXT_8("DB에서 긁어온 이름1"), MessageData::PlayerType_Knight, client);
    client->AddPlayer(player1);
    SharedPtr<Player> player2 = std::make_shared<Player>(sPlayerId.fetch_add(1), TEXT_8("DB에서 긁어온 이름2"), MessageData::PlayerType_Mage, client);
    client->AddPlayer(player2);

    // 전송 메시지 빌더 생성
    auto msgBuilder = std::make_shared<SendMessageBuilder>(MESSAGE_ID(ServerMessageId::Login));
    auto& dataBuilder = msgBuilder->GetDataBuilder();

    // 플레이어 데이터 빌드
    Vector<flatbuffers::Offset<MessageData::Player>> dataPlayers;
    auto namePlayer1 = dataBuilder.CreateString(player1->name);
    auto dataPlayer1 = MessageData::CreatePlayer(dataBuilder, player1->id, namePlayer1, player1->type);
    dataPlayers.push_back(dataPlayer1);
    auto namePlayer2 = dataBuilder.CreateString(player2->name);
    auto dataPlayer2 = MessageData::CreatePlayer(dataBuilder, player2->id, namePlayer2, player2->type);
    dataPlayers.push_back(dataPlayer2);
    auto vectorPlayers = dataBuilder.CreateVector(dataPlayers);

    // 로그인 처리
    client->SetLogined();
    gLogger->Info(TEXT_8("Logined: id={}, password={}"), data->id()->c_str(), data->password()->c_str());

    // 로그인 데이터 빌드 후 전송
    auto dataLogin = MessageData::Server::CreateLogin(dataBuilder, true, vectorPlayers);
    msgBuilder->FinishBuilding(dataLogin);
    client->Send(std::move(msgBuilder));

    return true;
}

Bool ClientMessageHandlerManager::HandleEnterGame(SharedPtr<Session> session, const MessageData::Client::EnterGame* data)
{
    SharedPtr<ClientSession> client = std::static_pointer_cast<ClientSession>(session);

    // 플레이어 인덱스 출력
    gLogger->Info(TEXT_8("EnterGame: player_idx={}"), data->player_idx());

    // TODO: 인덱스 유효성 검사

    // 플레이어 룸 입장
    SharedPtr<Player> player = client->GetPlayer(data->player_idx());
    gRoom->MakeJob(&Room::Enter, player);

    // 룸 입장 메시지 전송
    SharedPtr<SendMessageBuilder> message = std::make_shared<SendMessageBuilder>(MESSAGE_ID(ServerMessageId::EnterGame));
    auto& dataBuilder = message->GetDataBuilder();
    auto enterGame = MessageData::Server::CreateEnterGame(dataBuilder, true);
    message->FinishBuilding(enterGame);
    player->owner->Send(std::move(message));

    return true;
}

Bool ClientMessageHandlerManager::HandleChat(SharedPtr<Session> session, const MessageData::Client::Chat* data)
{
    SharedPtr<ClientSession> client = std::static_pointer_cast<ClientSession>(session);

    // 로그인 검사
    if (false == client->IsLogined())
    {
        gLogger->Warn(TEXT_8("Not logined"));
        return false;
    }

    // 채팅 메시지 로그
    gLogger->Info(TEXT_8("Chat: message={}"), data->message()->c_str());

    // 룸에 있는 모든 플레이어에게 채팅 메시지 전송
    SharedPtr<SendMessageBuilder> sendMessage = std::make_shared<SendMessageBuilder>(MESSAGE_ID(ServerMessageId::Chat));
    auto& dataBuilder = sendMessage->GetDataBuilder();
    auto messageData = dataBuilder.CreateString(data->message()->c_str());
    auto chatData = MessageData::Server::CreateChat(dataBuilder, client->GetPlayer(0)->id, messageData);
    sendMessage->FinishBuilding(chatData);
    gRoom->MakeJob(&Room::Broadcast, sendMessage);

    return true;
}
