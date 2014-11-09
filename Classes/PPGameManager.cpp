#include "PPGameManager.h"
#include "PogoPainterScene.h"
#include <algorithm>

using namespace std;

void PPGameConnection::run()
{

    StreamSocket& ss = socket();
    try {
        char buffer[256];
        int n = ss.receiveBytes(buffer, sizeof(buffer));
        while (n > 0) {
            ss.sendBytes(buffer, n);
            n = ss.receiveBytes(buffer, sizeof(buffer));
        }
    } catch (Poco::Exception& exc) {
        //std::cerr << "EchoConnection: " << exc.displayText() << std::endl;
    }
}


std::vector<PPPlayer*> PPGameManager::getPlayers()
{
    vector<PPPlayer*> players;
    players.reserve(4);
    copy(ai.begin(), ai.end(), back_inserter(players));
    
    for (auto & remote : clients) {
        players.push_back(remote.second);
    }

    players.push_back(human);
    return players;
}


bool PPGameManager::addClient(const std::string & ip, PPPlayer * player)
{
    if (clients.find(ip) == clients.end()) {
        clients[ip] = player;
        return true;
    }
    return false;
}


bool PPGameManager::disconnectClient(const std::string & ip)
{
    auto remote = clients.find(ip);
    if (remote == clients.end()) {
        clients.erase(remote);
        return true;
    }
    return false;
}

bool PPGameManager::hasClient(const std::string & ip)
{
    return clients.find(ip) == clients.end();
}


void PPGameManager::initHuman()
{
    if (this->role == Server) {
        this->human = new PPHumanPlayer(
            Vec2::ZERO, PPColor::Red, *this->scene, 
            Sprite::createWithTexture(this->scene->getPlayerTexture(PPPlayer::Color::R)));

    }
}

void PPGameManager::fillWithAi(int max)
{
    if (!board) {
        CCLOG("PPGameManager missing board!!");
        return;
    }

    int missing = std::min((int)(4 - 1 - clients.size()), max);

    vector<const PPPlayerData*> available;
    auto players = getPlayers();

    for (const auto & data : PlayerData) {
        bool add = true;
        for (auto & p : players) {
            add = add && p->getPosition() != data.pos;
        }
        if (add) {
            available.push_back(&data);
        }
    }

    for (auto & data : available) {
        auto player = new PPStupidAiPlayer(
            data->pos, data->board_color, *(this->scene), 
            Sprite::createWithTexture(this->scene->getPlayerTexture(data->player_color)));
        ai.push_back(player);
    }

}

void PPGameManager::update(int tick, float dt)
{

}