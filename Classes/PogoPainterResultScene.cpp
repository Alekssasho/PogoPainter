//
//  PogoPainterResultScene.cpp
//  PogoPainter
//
//  Created by Asen Lekov on 11/8/14.
//
//

#include "PogoPainterResultScene.h"

USING_NS_CC;

Scene* PogoPainterResults::createScene()
{
    auto scene = Scene::create();
    auto layer = PogoPainterResults::create();
    
    scene->addChild(layer);
    
    return scene;
}

bool PogoPainterResults::init()
{
    if (!LayerColor::initWithColor(Color4B(0, 0, 0, 128))) {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    auto startGameLabel = LabelTTF::create("YOU WIN!", "Marker Felt.ttf", 48);
    startGameLabel->setPosition(visibleSize.width / 2, visibleSize.height - 48);
    
    this->addChild(startGameLabel);
    
    FadeIn* fadeIn = FadeIn::create(.5f);
    fadeIn->setDuration(1.f);
    FadeOut* fadeOut = FadeOut::create(1.0f);
    fadeOut->setDuration(.5f);
    Sequence* seq = Sequence::createWithTwoActions(fadeIn, fadeOut);
    
    RepeatForever* repeat = RepeatForever::create(seq);
    
    startGameLabel->runAction(repeat);
    
    return true;
}

void PogoPainterResults::update(float dt)
{
    
}
