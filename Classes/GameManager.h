//
//  PPGameManager.h
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/27/14.
//
//

#ifndef __PogoPainter__PPGameManager__
#define __PogoPainter__PPGameManager__

#include "GameState.h"
#include "BonusManager.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <condition_variable>
#include <mutex>

#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Net/TCPServerParams.h"
#include "Poco/Net/ServerSocket.h"

class GameManager {
public:
    GameManager(int t = 90): mTimer(t) {};

    void init() {}
    void update(float deltaTime) {};

    GameState& state() { return mState; }
    int timer() const { return mTimer; }

    static const float tickDelay;

private:
    GameState mState;
    const int mTimer;
};


class ServerConnection: public Poco::Net::TCPServerConnection
{
    friend class GameServer;
    GameServer * server;
public:
    ServerConnection(const Poco::Net::StreamSocket& s);

    void run();
};


class GameServer
{
    const int mMaxClients;
    const int mRemoteClients;
    std::unordered_map<std::string, std::pair<int, Player*>> mClinets;
    Poco::Net::TCPServer * server;
    std::unique_ptr<Poco::Net::TCPServerConnectionFactoryImpl<ServerConnection>> factory;

    std::vector<bool> mAlive;
    
    std::mutex mPingLock;
    std::mutex mWaitMutex;
    std::condition_variable mCanSendState;

    static GameServer * self;

    int mTicks;
    GameState mState;

    static const float tickDelay;
    const int mTimer;
public:
    static GameServer * getServer() { return GameServer::self; }

    enum Status { Waiting, Running, Stopped } status;
    
    GameServer(int gameTime, int clients);
    ~GameServer();

    bool startGame();
    void stopGame();

    void ping(const std::string & ip);

    // every connection calls this to wait before sending data
    void waitToSend();

    void addClient(const std::string & ip);

    void update(float deltaTime);

    GameState::game_state & getGameState();
    GameState::game_state::player_state & getPlayerState(const std::string & ip);

};

#endif /* defined(__PogoPainter__PPGameManager__) */
