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


PPStupidAiPlayer::PPStupidAiPlayer(const Vec2 & pos, PPColor c, PogoPainter& scene, Sprite* pSprite)
    : PPPlayer(pos, c, scene, pSprite)
{

}