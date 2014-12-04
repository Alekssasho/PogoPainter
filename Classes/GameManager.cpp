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
const float GameManager::tickDelay = 0.5f;

//Globals
const int gPortNumber = 8126;


using namespace Poco::Net;
GameServer * GameServer::self = nullptr;

struct PlayerData {
    Color color;
    Vec2 pos;
    Direction dir;
};


GameServer::GameServer(int gameTime, int clients)
: factory(new TCPServerConnectionFactoryImpl<ServerConnection>()), mMaxClients(4), mRemoteClients(clients), mAlive(mRemoteClients, false), mTimer(gameTime)
{
    static const std::vector<PlayerData> playerData = {
        { Color::Red, Vec2(0, 0), Direction::Up },
        { Color::Green, Vec2(0, 7), Direction::Right },
        { Color::Blue, Vec2(7, 0), Direction::Down },
        { Color::Yellow, Vec2(7, 7), Direction::Left }
    };

    int c;
    for (c = 0; c < clients; ++c) {
        auto player = new ServerPlayer;
        player->color = playerData[c].color;
        player->pos = playerData[c].pos;
        player->currentDirection = playerData[c].dir;

        mState.players().push_back(PlayerPtr(player));
    }

    for (; c < 4; ++c) {
        auto pAi = new StupidAiPlayer;
        pAi->color = playerData[c].color;
        pAi->currentDirection = playerData[c].dir;
        pAi->pos = playerData[c].pos;
        mState.players().push_back(PlayerPtr(pAi));
    }


    GameServer::self = this;
    ServerSocket sock(SocketAddress(IPAddress(), gPortNumber));
    server = new TCPServer(&*factory, sock);
    server->start();
}

GameServer::~GameServer()
{
    server->stop();
    delete server;
}

void GameServer::waitToSend()
{
    unique_lock<mutex> unqLock(mWaitMutex);
    mCanSendState.wait(unqLock, [this] {
        return this->mRemoteClients == count(mAlive.begin(), mAlive.end(), true);
    });
}

void GameServer::addClient(const std::string & ip)
{
    if (mClinets.find(ip) == mClinets.end()) {
        mClinets[ip] = make_pair(server->currentConnections(), nullptr);
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

bool GameServer::startGame()
{
    if (mClinets.size() != mRemoteClients) {
        return false;
    }
    fill(mAlive.begin(), mAlive.end(), false);
    status = Running;
    // wait for all connections to go waiting on condvar
    this_thread::sleep_for(std::chrono::milliseconds(100));
    mCanSendState.notify_all();
    
    return true;
}

void GameServer::stopGame()
{
    status = Stopped;
}

void GameServer::update(float deltaTime)
{
    unique_lock<mutex> lock(mPingLock);

    if (any_of(mAlive.begin(), mAlive.end(), [](const bool & alive) { return !alive; })) {
        // handle someone not responding
        this->stopGame();
    }
    fill(mAlive.begin(), mAlive.end(), false);

    mState.incrementTick();
    mState.serialize();
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
        } else {
            //TODO: feedback on wall hit
        }
    }

    if (ticks % 2) {
        for (auto& pBonus : BonusManager::getInstance().getBonuses()) {
            pBonus->update(mState);
        }
    }
}

void GameServer::ping(const std::string & ip)
{
    unique_lock<mutex> lock(mPingLock);
    mAlive[mClinets[ip].first] = true;
}

ServerConnection::ServerConnection(const StreamSocket & s): Poco::Net::TCPServerConnection(s), server(GameServer::getServer())
{}

void ServerConnection::run()
{
    StreamSocket& ss = socket();
    if (server->status == GameServer::Running) {
        return;
    }
    const auto & ip = ss.address().host().toString();
    server->addClient(ip);

    std::chrono::milliseconds snooze_time(10), sleep_time(300);

    auto * myState = &server->getPlayerState(ip);
    auto * gameState = &server->getGameState();

    try {
        while (server->status == GameServer::Waiting) {
            ss.sendBytes(myState, sizeof(*myState));
            this_thread::sleep_for(snooze_time);
        }

        server->waitToSend();
        
        while (server->status == GameServer::Running) {
            if (ss.sendBytes(gameState, sizeof(*gameState)) != sizeof(*gameState)) {
                throw 42;
            }

            unsigned char data[sizeof(*myState)];
            int received = 0;

            for (;;) {
                int got = ss.receiveBytes(myState, sizeof(*myState) - received);
                if (!got) {
                    // graceful shutdown from peer
                    return;
                }
                memcpy(reinterpret_cast<void*>(myState + received), data, got);
                received += got;
            }            
            
            // if we'r not the last pinger wait for everyone
            server->ping(ip);
            server->waitToSend();
        }

    } catch (Poco::Exception &) {
        
    } catch (int) {

    }
}

ClientConnection::ClientConnection(const std::string& ipaddrs, int time)
: mSocket(SocketAddress(ipaddrs, gPortNumber)), mTimer(time)
{
    this->registerWithServer();
}

void ClientConnection::registerWithServer()
{
    //TODO: init sequance with server
    
    //TODO: receive signal from server to start game
    this->gameStarted();
}

void ClientConnection::gameStarted()
{
    //Register listening for swipes
    ADD_DELEGATE("Swipe", [this](EventCustom* e) {
        auto pDir = static_cast<Direction*>(e->getUserData());
        
        this->sendDirection(*pDir);
    });

    int size = sizeof(GameState::state);
    char data[size];
    int received = 0;
    int got = 0;
    //Server will tell us to stop the game  when it is finished
    while(true) {
        while(received != size) {
            got = mSocket.receiveBytes(&data, size - received);
            
            memcpy(reinterpret_cast<char*>(&mState.state) + received, data, got);
            received += got;
        }
        
        received = 0;
        
        this->deserializeAndSendEvents();
    }
}

void ClientConnection::deserializeAndSendEvents()
{
    auto& state = mState.state;
    
    mState.setTicks(state.tick);
    
    int pid = 0;
    for (auto & player : mState.players()) {
        auto & state_player = state.player[pid++];
        
        player->color = state_player.color;
        player->currentDirection = state_player.dir;
        player->pos = Vec2(state_player.pos[0], state_player.pos[1]);
        player->points = state_player.points;
        
        SEND_EVENT("RotatePlayer", &*player);
    }
    
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
            SEND_EVENT("RemoveBonus", cell.pBonus);
            
            if(cell.pBonus->type == Bonus::Type::Arrow) {
                //this will always find something
                auto index = std::find_if(state.player, state.player + 4, [&cell](const GameState::game_state::player_state& pl) {
                    return pl.pos[0] == cell.x && pl.pos[1] == cell.y;
                }) - state.player;
                
                SEND_EVENT("TriggerArrow", &mState.players()[index]);
            }
            
            delete cell.pBonus;
        }
    }
}

void ClientConnection::sendDirection(Direction dir)
{
    mSocket.sendBytes(&dir, sizeof(dir));
}
