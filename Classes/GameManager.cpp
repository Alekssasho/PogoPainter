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

#include <Poco/ByteOrder.h>

#include "Macros.h"

using namespace cocos2d;
using namespace std;

const float GameServer::tickDelay = 0.5f;
const float GameManager::tickDelay = 0.5f;

//Globals
const int gPortNumber = 8126;


using namespace Poco::Net;
GameServer * GameServer::self = nullptr;


GameServer::GameServer(int gameTime, int clients)
: factory(new TCPServerConnectionFactoryImpl<ServerConnection>()), mMaxClients(4), mRemoteClients(clients), mAlive(mRemoteClients, false), mTimer(gameTime),
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
    ServerSocket sock(SocketAddress(IPAddress(), gPortNumber));
    server = new TCPServer(&*factory, sock);
    server->start();
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

GameServer::~GameServer()
{
    server->stop();
    delete server;
}

void GameServer::waitToSend(function<bool()> until)
{
    unique_lock<mutex> unqLock(mSendMutex);
    mCanSendState.wait(unqLock, until);
}

void GameServer::addClient(const std::string & ip)
{
    if (mClinets.find(ip) == mClinets.end()) {
        mClinets[ip] = make_pair(server->currentConnections() - 1, nullptr);
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

bool GameServer::startGame()
{
    if (mClinets.size() != mRemoteClients) {
        return false;
    }
    fill(mAlive.begin(), mAlive.end(), false);
    status = Status::Running;
    mCanSendState.notify_all();
    
    return true;
}

void GameServer::stopGame()
{
    status = Status::Stopped;
}

void GameServer::update(float deltaTime)
{
    lock_guard<mutex> lock(mPingLock);

    if (any_of(mAlive.begin(), mAlive.end(), [](const bool & alive) { return !alive; })) {
        // handle someone not responding
        this->stopGame();
    }
    fill(mAlive.begin(), mAlive.end(), false);

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
        } else {
            //TODO: feedback on wall hit
        }
    }

    if (ticks % 2) {
        for (auto& pBonus : BonusManager::getInstance().getBonuses()) {
            pBonus->update(mState);
        }
    }
    
    
    mState.serialize();

    // time to send clients the state
    mCanSendState.notify_all();
}

void GameServer::ping(const std::string & ip)
{
    lock_guard<mutex> lock(mPingLock);
    mAlive[mClinets[ip].first] = true;
}

ServerConnection::ServerConnection(const StreamSocket & s): Poco::Net::TCPServerConnection(s), server(GameServer::getServer())
{}

void ServerConnection::run()
{
    StreamSocket& ss = socket();
    if (server->status == GameServer::Status::Running) {
        return;
    }
    const auto & ip = ss.address().host().toString();
    server->addClient(ip);

    // will unregister player on function exit
    auto deleter = [&ip](GameServer * s) { s->removeClient(ip); };
    unique_ptr<GameServer, decltype(deleter)> unregister(server, deleter);

    auto myState = &server->getPlayerState(ip);
    auto gameState = &server->getGameState();
    
    auto receiveDuration(server->getReceiveTime());

    Poco::Timespan sendTimeout(0, chrono::duration_cast<chrono::microseconds>(server->getSendTime()).count());
    ss.setSendTimeout(sendTimeout);
    
    chrono::milliseconds yeildTime(1);
 
    if (ss.sendBytes(myState, sizeof(*myState)) != sizeof(*myState)) {
        server->removeClient(ip);
    }

    // wait for the game to start
    server->waitToSend([this] () {
        return this->server->status == GameServer::Status::Running;
    });

    // notifies all clients that game started
    if (ss.sendBytes(gameState, sizeof(*gameState)) != sizeof(*gameState)) {
        return;
    }
    
    auto pPlayerDir = &myState->dir;
        
    while (server->status == GameServer::Status::Running) {
        auto gotResponse = false;
        auto thisTick = server->getThisTick();

        auto stop = chrono::system_clock::now() + receiveDuration;
        while (stop > chrono::system_clock::now()) {
            if (ss.available() >= sizeof(*pPlayerDir)) {
                gotResponse = 0 != ss.receiveBytes(pPlayerDir, sizeof(*pPlayerDir));
            }
            this_thread::sleep_for(yeildTime);
        }

//        if (!gotResponse) {
//            return;
//        }

        if (ss.sendBytes(gameState, sizeof(*gameState)) != sizeof(*gameState)) {
            return;
        }

        server->ping(ip);

        // wait for next tick
        server->waitToSend([this, &thisTick] () {
            return this->server->getThisTick() == thisTick + 1;
        });
    }
}

ClientConnection::ClientConnection(const std::string& ipaddrs, int time)
: mSocket(), mTimer(time), ipAddress(ipaddrs), started(false), mReceived(false)
{
    this->registerPlayers();
}

void ClientConnection::registerWithServer()
{
    mSocket.connect(SocketAddress(ipAddress, gPortNumber));
    while(mSocket.available() != sizeof(mPlayer))
        ;
    mSocket.receiveBytes(&mPlayer, sizeof(mPlayer));
    this->sendPlayerState();
    
    started = true;
    this->gameStarted();
}

void ClientConnection::gameStarted()
{
    //Register listening for swipes
    ADD_DELEGATE("Swipe", [this](EventCustom* e) {
        auto pDir = static_cast<Direction*>(e->getUserData());
        
        mPlayer.dir = *pDir;
        this->sendPlayerState();
    });

    const int size = sizeof(GameState::game_state);
    char data[size];
    int received = 0;
    int got = 0;
    //Server will tell us to stop the game  when it is finished
    while(true) {
//        this->sendPlayerState();
        while(received != size) {
            got = mSocket.receiveBytes(&data, size - received);
            
            memcpy(reinterpret_cast<char*>(&mState.state) + received, data, got);
            received += got;
            
            mReceived = true;
        }
        
        received = 0;
        
        if(mState.ticks() == mTimer)
            return;
    }
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
    if(!mReceived)
        return;
    
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
    mSocket.sendBytes(&mPlayer.dir, sizeof(mPlayer.dir));
}
