//
//  PPPlayer.cpp
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/8/14.
//
//

#include "PPPlayer.h"
#include "PogoPainterScene.h"
#include <cocos2d.h>

using namespace cocos2d;

PPPlayer::PPPlayer(const Vec2 & pos, PPColor c, PogoPainter& scene, Sprite* pS)
: pos(pos), color(c), board(scene.getBoard()), pSprite(pS)
{}

//
//
//
//
//
PPHumanPlayer::PPHumanPlayer(const Vec2 & pos, PPColor c, PogoPainter& scene, Sprite* pS)
: PPPlayer(pos, c, scene, pS)
{
    auto eventListener = EventListenerTouchOneByOne::create();
    
    eventListener->onTouchBegan = [](Touch* touch, Event* event) {
        return true;
    };

    eventListener->onTouchEnded = [this](Touch* touch, Event* event) {
        auto startTouch = touch->getStartLocation();
        auto diff = touch->getLocation() - startTouch;
        //TODO: check some minimal length for drag
        if(diff.lengthSquared() <= 50*50)
            return;
        diff.normalize();

        float angle = Vec2::angle(diff, Vec2(1, 0));
        
        float spriteRotateAngle = -45;
        
        if(angle <= M_PI_4) {
            
            this->currentDirection = PPDirection::Right;
        } else if(angle <= M_PI_2 + M_PI_4) {
            this->currentDirection = diff.y > 0 ? PPDirection::Up : PPDirection::Down;
            this->currentDirection == PPDirection::Up ? spriteRotateAngle -= 90 : spriteRotateAngle += 90;
        } else {
            this->currentDirection = PPDirection::Left;
            spriteRotateAngle += 180;
        }
        
        this->autorotate();
    };
    
    scene.registerEventListener(eventListener);
}

void PPPlayer::autorotate()
{
    int spriteRotateAngle = -45;
    switch (currentDirection) {
        case Up:
            spriteRotateAngle -= 90;
            break;
        case Down:
            spriteRotateAngle += 90;
            break;
        case Left:
            spriteRotateAngle += 180;
            break;
        default:
            break;
    }
    
    this->pSprite->stopActionByTag(15);
    auto rotate = RotateTo::create(0.15f, spriteRotateAngle);
    rotate->setTag(15);
    this->pSprite->runAction(rotate);
}

PPDirection PPHumanPlayer::getDirection()
{
    return currentDirection;
}


PPStupidAiPlayer::PPStupidAiPlayer(const Vec2 & pos, PPColor c, PogoPainter& scene, Sprite* pSprite)
    : PPPlayer(pos, c, scene, pSprite)
{
    
}

//
//
//
//
//
PPDirection PPStupidAiPlayer::getDirection()
{
    
    vector<PPCheckpoint*> checkpoints = PPBonusManager::getInstance().getCheckpoints();
    vector<PPBonus*> bonuses = PPBonusManager::getInstance().getBonuses();
    
    if (checkpoints.size() > 0 ) {
        following = getNearestCheckpoint(checkpoints);
    } else if (bonuses.size() > 0) {
        following = getNearestBonus(bonuses);
    } else {
        following = nullptr;
    }
    
    return getNextDirection();
}

double PPStupidAiPlayer::calcDistanceFromPlayerTo(int x, int y) const
{
    return calcDistanceFromTo(pos.x, pos.y, x, y);
}

double PPStupidAiPlayer::calcDistanceFromTo(int fromX, int fromY, int toX, int toY) const
{
    return sqrt(pow(fromX - toX, 2) + pow(fromY - toY, 2));
}

PPBonus* PPStupidAiPlayer::getNearestBonus(vector<PPBonus*> bonuses)
{
    double minDistance = 1000;
    PPBonus* selected;
   
    for (PPBonus* bonus: bonuses) {
        double distance = this->calcDistanceFromPlayerTo(bonus->getCell().x, bonus->getCell().y);
        
        if (distance < minDistance) {
            minDistance = distance;
            selected = bonus;
        }
    }
    return selected;
}

PPCheckpoint* PPStupidAiPlayer::getNearestCheckpoint(std::vector<PPCheckpoint*> checkpoints)
{
    double minDistance = 1000;
    PPCheckpoint* selected;
    
    for (PPCheckpoint* bonus: checkpoints) {
        double distance = this->calcDistanceFromPlayerTo(bonus->getCell().x, bonus->getCell().y);
        
        if (distance < minDistance) {
            minDistance = distance;
            selected = bonus;
        }
    }
    return selected;
}

PPDirection PPStupidAiPlayer::getNextDirection()
{
    if(!following) {
        return currentDirection;
    }
    
    double distance = calcDistanceFromPlayerTo(following->getCell().x, following->getCell().y);
    double nearCellDistance = 10000;
    
    PPDirection directions[] = {PPDirection::Down, PPDirection::Up, PPDirection::Left, PPDirection::Right};
    
    for (PPDirection dir: directions) {
        switch (dir) {
            case Up:
                nearCellDistance = calcDistanceFromTo(pos.x, pos.y + 1, following->getCell().x, following->getCell().y);
                break;
            case Down:
                nearCellDistance = calcDistanceFromTo(pos.x, pos.y - 1, following->getCell().x, following->getCell().y);
                break;
            case Left:
                nearCellDistance = calcDistanceFromTo(pos.x - 1, pos.y, following->getCell().x, following->getCell().y);
                break;
            case Right:
                nearCellDistance =  calcDistanceFromTo(pos.x + 1, pos.y, following->getCell().x, following->getCell().y);
                break;
            case None:
                nearCellDistance = calcDistanceFromPlayerTo(following->getCell().x, following->getCell().y);
                break;
        }
        
        if (distance > nearCellDistance) {
            distance = nearCellDistance;
            currentDirection = dir;
        }
    }
    return currentDirection;
}