/*    GameServer/Content/Player.h    */

#pragma once

class GameSession;

struct Player
{
    Int64                       id = 0;
    String8                     name;
    MessageData::PlayerType     type = MessageData::PlayerType_None;
    SharedPtr<GameSession>      owner;

    Player() = default;
    Player(Int64 id, String8 name, MessageData::PlayerType type, SharedPtr<GameSession> owner)
        : id(id)
        , name(name)
        , type(type)
        , owner(std::move(owner))
    {}
};
