/*    GameServer/Content/Player.h    */

#pragma once

#include "Common/MessageData/Common_generated.h"

class ClientSession;

struct Player
{
    Int64                       id = 0;
    String8                     name;
    MessageData::PlayerType     type = MessageData::PlayerType_None;
    SharedPtr<ClientSession>    owner;

    Player() = default;
    Player(Int64 id, String8 name, MessageData::PlayerType type, SharedPtr<ClientSession> owner)
        : id(id)
        , name(name)
        , type(type)
        , owner(std::move(owner))
    {}
};
