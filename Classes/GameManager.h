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
    
    bool checkForSignal();
    
    std::atomic<bool> started;
private:
    void registerPlayers();
    void gameStarted();
    
    ClientSocket mSocket;
    
    GameState mState;
    const int mTimer;
    
    std::string ipAddress;
    
    GameState::game_state::player_state mPlayer;
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
    
    std::vector<std::pair<SocketStream, std::string>> mClientSockets;

    static GameServer * self;

    int mTicks;
    GameState mState;

    const int mTimer;
    
    void addAiPlayer(Color color);
public:
    static const float tickDelay;
    static GameServer * getServer() { return GameServer::self; }

    enum Status { Waiting, Running, Stopped } status = Status::Waiting;

    GameServer(const GameServer &) = delete;
    GameServer & operator=(const GameServer &) = delete;
    
    GameServer(int gameTime = 90, int clients = 4);
    ~GameServer() {}

    void startGame();
    void stopGame();

    void addClient(const std::string & ip);
    void removeClient(const std::string & ip);
    void replaceClient(const std::string & ip);

    void update();

    GameState::game_state & getGameState();
    GameState::game_state::player_state & getPlayerState(const std::string & ip);
};

#endif /* defined(__PogoPainter__PPGameManager__) */
