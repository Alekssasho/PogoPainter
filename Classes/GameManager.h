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
#include <atomic>

#include "Socket.h"

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

class ClientConnection
{
public:
    ClientConnection(const std::string& ipaddres = "127.0.0.1", int time = 90);
    ~ClientConnection() { mSocket.Close(); };
    void sendPlayerState();
    GameState& state() { return mState; }
    int timer() const { return mTimer; }
    void registerWithServer();
    void deserializeAndSendEvents();
    
    std::atomic<bool> started;
    
    bool mReceived;
private:
    void registerPlayers();
    void gameStarted();
    
    ClientSocket mSocket;
    
    GameState mState;
    const int mTimer;
    
    std::string ipAddress;
    
    GameState::game_state::player_state mPlayer;
};

class GameServer;

class ServerConnection
{
private:
    GameServer * server;
    SocketStream sock;
    
public:
    ServerConnection(SocketStream s);

    void run();
};


class GameServer
{
    struct PlayerData {
        Color color;
        Vec2 pos;
        Direction dir;
    };

    const int mMaxClients;
    const int mRemoteClients;
    std::unordered_map<std::string, std::pair<int, Player*>> mClinets;
    ServerSocket server;

    const std::vector<std::pair<Color, PlayerData>> playerData;
    std::vector<bool> mAlive;
    
    std::mutex mPingLock;
    std::mutex mSendMutex;
    std::condition_variable mCanSendState;

    static GameServer * self;

    int mTicks;
    GameState mState;

    static const float tickDelay;
    const int mTimer;
    
    void addAiPlayer(Color color);
public:
    static GameServer * getServer() { return GameServer::self; }

    enum Status { Waiting, Running, Stopped } status = Status::Waiting;

    GameServer(const GameServer &) = delete;
    GameServer & operator=(const GameServer &) = delete;
    
    GameServer(int gameTime = 90, int clients = 4);
    ~GameServer();

    bool startGame();
    void stopGame();

    void ping(const std::string & ip);

    // every connection calls this to wait before sending data
    void waitToSend(std::function<bool()> until);
    int getThisTick() { return mState.ticks(); }

    void addClient(const std::string & ip);
    void removeClient(const std::string & ip);
    void replaceClient(const std::string & ip);

    void update(float deltaTime);

    GameState::game_state & getGameState();
    GameState::game_state::player_state & getPlayerState(const std::string & ip);

    std::chrono::milliseconds getTickDelay() { return std::chrono::milliseconds(static_cast<int>(tickDelay * 1000)); }
    std::chrono::milliseconds getReceiveTime() { return std::chrono::milliseconds(static_cast<int>(static_cast<float>(getTickDelay().count()) * 0.9)); }
    std::chrono::milliseconds getSendTime() { return getTickDelay() - getReceiveTime(); }
};

#endif /* defined(__PogoPainter__PPGameManager__) */
