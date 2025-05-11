/*    GameServer/Network/Session.h    */

#pragma once

#include "ServerEngine/Network/Session.h"
#include "GameServer/Network/Protocol/Packet.pb.h"

struct Player;
class Room;

//class ClientSession
//    : public Session
//{
//public:
//    void                SetLogined() { mIsLogined.store(true); }
//    Bool                IsLogined() const { return mIsLogined.load(); }
//    SharedPtr<Player>   GetPlayer(Int64 idx) const { return mPlayers[idx]; }
//    void                AddPlayer(SharedPtr<Player> player) { mPlayers.push_back(std::move(player)); }
//    void                SetCurrentPlayerIdx(Int64 idx) { mCurrentPlayerIdx = idx; }
//    Int64               GetCurrentPlayerIdx() const { return mCurrentPlayerIdx; }
//    void                SetPlayerRoom(SharedPtr<Room> room) { mPlayerRoom = std::move(room); }
//    SharedPtr<Room>     GetPlayerRoom() const { return mPlayerRoom.lock(); }
//
//protected:
//    virtual void        OnConnected() override;
//    virtual void        OnDisconnected(String8 cause) override;
//    virtual void        OnReceived(ReceiveMessage message) override;
//    virtual void        OnSent(Int64 numBytes) override;
//
//private:
//    Atomic<Bool>                mIsLogined = false;
//    Vector<SharedPtr<Player>>   mPlayers;
//    Int64                       mCurrentPlayerIdx = -1;
//    WeakPtr<Room>               mPlayerRoom;
//};

class ClientSession
    : public Session
{
protected:
    virtual void        OnConnected() override;
    virtual void        OnDisconnected(String8 cause) override;
    virtual Int64       OnReceived(const Byte* buffer, Int64 numBytes) override;
    virtual void        OnSent(Int64 numBytes) override;
};

class ClientSessionManager
{
public:
    ClientSessionManager()  = default;
    ~ClientSessionManager() = default;

    void                AddSession(SharedPtr<ClientSession> client);
    void                RemoveSession(SharedPtr<ClientSession> client);
    // void                Broadcast(SharedPtr<SendMessageBuilder> message);
    void                Broadcast(SharedPtr<SendBuffer> buffer);

private:
    RW_LOCK;
    HashSet<SharedPtr<ClientSession>>     mClients;
};

extern ClientSessionManager               gClientManager;
