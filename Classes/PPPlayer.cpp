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

PPPlayer::PPPlayer(int x, int y, PPColor c, PogoPainter& scene)
: x(x), y(y), color(c), board(scene.getBoard())
{}

PPHumanPlayer::PPHumanPlayer(int x, int y, PPColor c, PogoPainter& scene)
: PPPlayer(x, y, c, scene)
{
    auto eventListener = EventListenerTouchOneByOne::create();
    
    eventListener->onTouchBegan = [](Touch* touch, Event* event) {
        return true;
    };

    eventListener->onTouchEnded = [](Touch* touch, Event* event) {
        auto startTouch = touch->getStartLocation();
        auto diff = touch->getLocation() - startTouch;
        CCLOG("%f : %f", diff.x, diff.y);
    };
    
    scene.registerEventListener(eventListener);
}

PPDirection PPHumanPlayer::getDirection()
{
    return currentDirection;
}