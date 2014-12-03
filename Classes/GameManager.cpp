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
    ServerSocket sock(SocketAddress(IPAddress(), 8126));
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