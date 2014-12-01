//
//  SpecificPlayers.cpp
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/27/14.
//
//

#include "SpecificPlayers.h"

#include <cocos2d.h>
using namespace cocos2d;
#include "Macros.h"

HumanPlayer::HumanPlayer()
{
    ADD_DELEGATE("Swipe", [this](EventCustom* e) {
        auto pDir = static_cast<Direction*>(e->getUserData());
        
        this->currentDirection = *pDir;
        SEND_EVENT("RotatePlayer", this);
        
    });
}

Direction HumanPlayer::getDirection()
{
    return currentDirection;
}

Direction StupidAiPlayer::getDirection()
{
    
    auto& checkpoints = BonusManager::getInstance().getCheckpoints();
    auto& bonuses = BonusManager::getInstance().getBonuses();
    
    if (checkpoints.size() > 0 ) {
        following = getNearestCheckpoint(checkpoints);
    } else if (bonuses.size() > 0) {
        following = getNearestBonus(bonuses);
    } else {
        following = nullptr;
    }
    
    return getNextDirection();
}

double StupidAiPlayer::calcDistanceFromPlayerTo(int x, int y) const
{
    return calcDistanceFromTo(pos.x, pos.y, x, y);
}

double StupidAiPlayer::calcDistanceFromTo(int fromX, int fromY, int toX, int toY) const
{
    return sqrt(pow(fromX - toX, 2) + pow(fromY - toY, 2));
}

Bonus* StupidAiPlayer::getNearestBonus(const std::vector<BonusPtr>& bonuses)
{
    double minDistance = 1000;
    Bonus* selected = nullptr;
    
    for (auto& bonus: bonuses) {
        double distance = this->calcDistanceFromPlayerTo(bonus->cell.x, bonus->cell.y);
        
        if (distance < minDistance) {
            minDistance = distance;
            selected = bonus.get();
        }
    }
    return selected;
}

Bonus* StupidAiPlayer::getNearestCheckpoint(const std::vector<BonusPtr>& checkpoints)
{
    double minDistance = 1000;
    Bonus* selected = nullptr;
    
    for (auto& bonus: checkpoints) {
        double distance = this->calcDistanceFromPlayerTo(bonus->cell.x, bonus->cell.y);
        
        if (distance < minDistance) {
            minDistance = distance;
            selected = bonus.get();
        }
    }
    return selected;
}

Direction StupidAiPlayer::getNextDirection()
{
    if(!following) {
        return currentDirection;
    }
    
    double distance = calcDistanceFromPlayerTo(following->cell.x, following->cell.y);
    double nearCellDistance = 10000;
    
    Direction directions[] = {Direction::Down, Direction::Up, Direction::Left, Direction::Right};
    
    for (Direction dir: directions) {
        switch (dir) {
            case Up:
                nearCellDistance = calcDistanceFromTo(pos.x, pos.y + 1, following->cell.x, following->cell.y);
                break;
            case Down:
                nearCellDistance = calcDistanceFromTo(pos.x, pos.y - 1, following->cell.x, following->cell.y);
                break;
            case Left:
                nearCellDistance = calcDistanceFromTo(pos.x - 1, pos.y, following->cell.x, following->cell.y);
                break;
            case Right:
                nearCellDistance =  calcDistanceFromTo(pos.x + 1, pos.y, following->cell.x, following->cell.y);
                break;
            case None:
                nearCellDistance = calcDistanceFromPlayerTo(following->cell.x, following->cell.y);
                break;
        }
        
        if (distance > nearCellDistance) {
            distance = nearCellDistance;
            currentDirection = dir;
        }
    }
    return currentDirection;
}


