 //
//  PPGameManager.cpp
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/27/14.
//
//

#include "GameManager.h"
#include "SimpleAudioEngine.h"
#include "SpecificPlayers.h"

#include <cocos2d.h>

#include <future>
#include <chrono>
#include <utility>

#include "Macros.h"

using namespace cocos2d;
using namespace std;

const float GameServer::tickDelay = 0.5f;

//Globals
static const int gPortNumber = 8126;

static const char GAME_START_SIGNAL = 0;
static const char SEND_DIR_SIGNAL = 1;

GameServer * GameServer::self = nullptr;

GameServer::GameServer(int gameTime, int clients)
: mMaxClients(4), mRemoteClients(clients), mTimer(gameTime),
playerData({
    make_pair(Color::Red, PlayerData{ Color::Red, Vec2(0, 0), Direction::Up }),
    make_pair(Color::Green, PlayerData{ Color::Green, Vec2(0, 7), Direction::Right }),
    make_pair(Color::Blue, PlayerData{ Color::Blue, Vec2(7, 7), Direction::Down }),
    make_pair(Color::Yellow, PlayerData{ Color::Yellow, Vec2(7, 0), Direction::Left })
})
{
    int c;
    auto player_data = playerData.begin();

    for (c = 0; c < clients; ++c, ++player_data) {
        auto player = new ServerPlayer;
        player->color = player_data->second.color;
        player->pos = player_data->second.pos;
        player->currentDirection = player_data->second.dir;
        mState.players().push_back(PlayerPtr(player));
    }

    for (; c < 4; ++c, ++player_data) {
        addAiPlayer(player_data->second.color);
    }
    
    mState.serialize();

    GameServer::self = this;
    server.Listen(gPortNumber);
    
    mClientSockets.reserve(clients);
    for(int i = 0; i < clients; ++i) {
        SocketStream sock(server.Accept());
        string ip = sock.GetPeerName();
        mClientSockets.emplace_back(move(sock), move(ip));
    }
    
    for (auto& sock : mClientSockets){
        this->addClient(sock.second);
        
        auto myState = &this->getPlayerState(sock.second);
        if(sock.first.SendBytes(myState, sizeof(*myState)) != sizeof(*myState)) {
            this->removeClient(sock.second);
            sock.first.Close();
        }
    }
}

void GameServer::startGame()
{
    while (mClinets.size() != mRemoteClients)
        ;
    
    status = Status::Running;
    
    auto gameState = &this->getGameState();
    const size_t gameStateSize = sizeof(*gameState);
    
    //tell client game have started
    for(auto& sock : mClientSockets) {
        sock.first.SendBytes(&GAME_START_SIGNAL, sizeof(char));
        sock.first.SendBytes(gameState, gameStateSize);
    }
    
    while (this->status == GameServer::Status::Running) {
        
        auto time = chrono::system_clock::now();
        
        this->update();
        
        for(auto& sock : mClientSockets) {
            sock.first.SendBytes(&SEND_DIR_SIGNAL, sizeof(char));
            
            auto pPlayerDir = &this->getPlayerState(sock.second).dir;
            
            while (sock.first.Available() != sizeof(*pPlayerDir))
                ;
            sock.first.ReceiveBytes(pPlayerDir, sizeof(*pPlayerDir));
            
            sock.first.SendBytes(gameState, gameStateSize);
        }
        
        std::this_thread::sleep_for(chrono::milliseconds(500) - (chrono::system_clock::now() - time));
    }
    
}

void GameServer::addAiPlayer(Color color)
{
    const auto & pData = playerData[color].second;
    auto pAi = new StupidAiPlayer;
    pAi->color = color;
    pAi->currentDirection = pData.dir;
    pAi->pos = pData.pos;
    mState.players().push_back(PlayerPtr(pAi));
}

void GameServer::addClient(const std::string & ip)
{
    static int numConn = 0;
    if (mClinets.find(ip) == mClinets.end()) {
        mClinets[ip] = make_pair(numConn++, nullptr);
    }
}

void GameServer::removeClient(const std::string & ip)
{
    if (status == Status::Running) {
        replaceClient(ip);
    } else {
        auto cl = mClinets.find(ip);
        if (cl != mClinets.end()) {
            mClinets.erase(cl);
        }
    }
}

void GameServer::replaceClient(const std::string & ip)
{
    auto cl = mClinets.find(ip);
    if (cl != mClinets.end()) {
        auto removedColor = mState.state.player[cl->second.first].color;
        mClinets.erase(cl);
        addAiPlayer(removedColor);
    }
}

GameState::game_state::player_state & GameServer::getPlayerState(const std::string & ip)
{
    return mState.state.player[mClinets[ip].first];
}

GameState::game_state & GameServer::getGameState()
{
    return mState.state;
}

void GameServer::stopGame()
{
    status = Status::Stopped;
}

void GameServer::update()
{
    mState.client_deserialize();
    
    mState.incrementTick();

    int ticks = mState.ticks();


    BonusManager::getInstance().update(mState);

    auto& players = mState.players();
    auto& board = mState.board();

    for (unsigned int i = 0; i < players.size(); ++i) {
        auto& pl = players[i];
        auto& pBonus = board[pl->pos].pBonus;
        if (pBonus) {
            pBonus->apply(mState, i);

            BonusManager::getInstance().removeBonus(pBonus);
            board[pl->pos].pBonus = nullptr;

        }
        board[pl->pos].color = pl->color;
    }

    for (auto& pl : players) {
        auto dir = pl->getDirection();

        for (auto& other : players) {
            if (other != pl && board[board.moveInDir(pl->pos, pl->currentDirection)] == board[other->pos]) {
                dir = Direction::None;
                break;
            }
        }

        auto res = board.moveInDir(pl->pos, dir);
        if (pl->pos != res) {
            pl->pos = res;
        }
    }

    if (ticks % 2) {
        for (auto& pBonus : BonusManager::getInstance().getBonuses()) {
            pBonus->update(mState);
        }
    }
    
    
    mState.serialize();
}

ClientConnection::ClientConnection(const std::string& ipaddrs, int time)
: mSocket(), mTimer(time), ipAddress(ipaddrs), started(false)
{
    this->registerPlayers();
}

void ClientConnection::registerWithServer()
{
    if(!mSocket.Connect(ipAddress, gPortNumber)) {
        log("coundnt connect to server");
        exit(1);
    }
    while(mSocket.Available() != sizeof(mPlayer))
        ;
    mSocket.ReceiveBytes(&mPlayer, sizeof(mPlayer));
    
    started = true;
    
    ADD_DELEGATE("Swipe", [this](EventCustom* e) {
        auto pDir = static_cast<Direction*>(e->getUserData());
        
        mPlayer.dir = *pDir;
    });
}

bool ClientConnection::checkForSignal()
{
    static const int size = sizeof(GameState::game_state);
    static const size_t signalSize = sizeof(char);
    
    if(mSocket.Available() < signalSize)
        return false;
    
    char signal;
    mSocket.ReceiveBytes(&signal, signalSize);
    if (signal == SEND_DIR_SIGNAL)
        this->sendPlayerState();
    
    while(mSocket.Available() < size)
        ;
    
    mSocket.ReceiveBytes(&mState.state, size);
    
    return true;
}

void ClientConnection::registerPlayers()
{
    mState.players().push_back(PlayerPtr(new ServerPlayer(Vec2(0, 0), Color::Red, Direction::Up)));
    mState.players().push_back(PlayerPtr(new ServerPlayer(Vec2(0, 7), Color::Green, Direction::Right)));
    mState.players().push_back(PlayerPtr(new ServerPlayer(Vec2(7, 7), Color::Blue, Direction::Down)));
    mState.players().push_back(PlayerPtr(new ServerPlayer(Vec2(7, 0), Color::Yellow, Direction::Left)));
}

void ClientConnection::deserializeAndSendEvents()
{
    auto& state = mState.state;
    
    mState.setTicks(state.tick);
    
    int cell_id = 0;
    
    for (auto & cell : mState.board().cells) {
        auto & state_cell = state.board[cell_id++];
        if(cell.color != Color::Gray && state_cell.color == Color::Gray) {
            SEND_EVENT("RemoveColor", &cell);
        }
        cell.color = state_cell.color;
        
        if(state_cell.has_bonus) {
            if(!cell.pBonus) {
                cell.pBonus = BonusInitializers.at(state_cell.bonus_type)(cell);
                SEND_EVENT("NewBonus", cell.pBonus);
            } else if(state_cell.bonus_type == Bonus::Type::Arrow) {
                if(cell.pBonus->getData() != state_cell.bonus_data) {
                    SEND_EVENT("RotateArrow", cell.pBonus);
                }
            }
            
            cell.pBonus->setData(state_cell.bonus_data);
        } else if (cell.pBonus) {
            if(cell.pBonus->type == Bonus::Type::Arrow) {
                SEND_EVENT("TriggerArrow", cell.pBonus);
            }
            
            SEND_EVENT("RemoveBonus", cell.pBonus);
            delete cell.pBonus;
            cell.pBonus = nullptr;
        }
    }
    
    int pid = 0;
    for (auto & player : mState.players()) {
        auto & state_player = state.player[pid++];
        
        player->color = state_player.color;
        player->currentDirection = state_player.dir;
        player->pos = Vec2(state_player.pos[0], state_player.pos[1]);
        player->points = state_player.points;
        
        SEND_EVENT("RotatePlayer", &*player);
    }
}

void ClientConnection::sendPlayerState()
{
    mSocket.SendBytes(&mPlayer.dir, sizeof(mPlayer.dir));
}
