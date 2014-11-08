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

PPPlayer::PPPlayer(int x, int y, PPColor c, PogoPainter& scene, Sprite* pS)
: x(x), y(y), color(c), board(scene.getBoard()), pSprite(pS)
{}

PPHumanPlayer::PPHumanPlayer(int x, int y, PPColor c, PogoPainter& scene, Sprite* pS)
: PPPlayer(x, y, c, scene, pS)
{
    auto eventListener = EventListenerTouchOneByOne::create();
    
    eventListener->onTouchBegan = [](Touch* touch, Event* event) {
        return true;
    };

    eventListener->onTouchEnded = [this](Touch* touch, Event* event) {
        auto startTouch = touch->getStartLocation();
        auto diff = touch->getLocation() - startTouch;
        //TODO: check some minimal length for drag
        if(diff.lengthSquared() <= 100*100)
            return;
        diff.normalize();

        float angle = Vec2::angle(diff, Vec2(1, 0));
        
        if(angle <= M_PI_4)
            this->currentDirection = PPDirection::Right;
        else if(angle <= M_PI_2 + M_PI_4)
            this->currentDirection = diff.y > 0 ? PPDirection::Up : PPDirection::Down;
        else
            this->currentDirection = PPDirection::Left;
    };
    
    scene.registerEventListener(eventListener);
}

PPDirection PPHumanPlayer::getDirection()
{
    return currentDirection;
}