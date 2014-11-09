#ifndef __PP_GAME_MANAGER_H__
#define __PP_GAME_MANAGER_H__

#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Net/TCPServerParams.h"
#include "Poco/Net/ServerSocket.h"


using namespace Poco::Net;

#include "cocos2d.h"
#include "PPPlayer.h"
#include <string>
#include <vector>
USING_NS_CC;

struct PPPlayerData {
    PPPlayer::Color player_color;
    PPColor board_color;
    const char * texture;
    Vec2 pos;
};

const std::vector<PPPlayerData> PlayerData = {
    {PPPlayer::Color::R, PPColor::Red, "Player/player_red.png", Vec2(0, 0)},
    {PPPlayer::Color::G, PPColor::Green, "Player/player_green.png", Vec2(0, 7)},
    {PPPlayer::Color::B, PPColor::Blue, "Player/player_blue.png", Vec2(7, 7)},
    {PPPlayer::Color::Y, PPColor::Yellow, "Player/player_yellow.png", Vec2(7, 0)}
};

class PPGameConnection : public TCPServerConnection {
public:
    PPGameConnection(const StreamSocket& s) : TCPServerConnection(s) { }

    void run();
};


class PPGameManager
{
    PPHumanPlayer * human;
    std::vector<PPStupidAiPlayer*> ai;
    std::map<std::string, PPPlayer*> clients;
    
    PPBoard * board;
    PogoPainter * scene;

    enum GameState { Running, Settingup } state;
    PPGameManager() : state(Settingup), board(NULL), scene(NULL), human(NULL) {}
public:
    enum Role { Client, Server } role;

    void update(int tick, float dt);


    static PPGameManager & getInstance() {
        static PPGameManager instance;
        return instance;
    }

    void setBoard(PPBoard * board) { this->board = board; }
    void setScene(PogoPainter * scene) { this->scene = scene; }
    void setRole(PPGameManager::Role r) {
        role = r;
    }

    void initHuman();

    bool addClient(const std::string & ip, PPPlayer * player);
    bool disconnectClient(const std::string & ip);
    bool hasClient(const std::string & ip);
    
    std::vector<PPPlayer*> getPlayers();
    
    
    void fillWithAi(int max);
        
    void start() { state = Running; }

    void setupServer() {
        auto factory = new TCPServerConnectionFactoryImpl<PPGameConnection>();
        SocketAddress sa(IPAddress(), 8126);
        ServerSocket sock(sa);
        auto srv = new TCPServer(factory, sock);
        srv->start();
    }
};



#endif