//
//  PogoPainterMenuScene.cpp
//  PogoPainter
//
//  Created by Asen Lekov on 11/8/14.
//
//

#include "PogoPainterMenuScene.h"

USING_NS_CC;

Scene* PogoPainterMenu::createScene()
{
    auto scene = Scene::create();
    auto layer = PogoPainterMenu::create();
    
    scene->addChild(layer);
    
    return scene;
}

bool PogoPainterMenu::init()
{
    if (!LayerColor::initWithColor(Color4B(255, 0, 0, 128))) {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    auto startGameLabel = LabelTTF::create("Touch to start game...", "fonts/Marker Felt.ttf", 32);
    startGameLabel->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    
    this->addChild(startGameLabel);
    
    auto eventListener = EventListenerTouchOneByOne::create();
    eventListener->setSwallowTouches(true);
    
    eventListener->onTouchBegan = [](Touch* touch, Event* event) {
        return true;
    };
    
    eventListener->onTouchEnded = [this](Touch* touch, Event* event) {
        auto startLocation = touch->getStartLocation();
        auto diff = touch->getLocation() - startLocation;
        
        if (diff.getLength() <= 50) {
            Director::getInstance()->getRunningScene()->removeChild(this);
            Director::getInstance()->getRunningScene()->getChildren().back()->scheduleUpdate();
        }
    };
    
    registerEventListener(startGameLabel, eventListener);
    
    FadeIn* fadeIn = FadeIn::create(.5f);
    fadeIn->setDuration(1.f);
    FadeOut* fadeOut = FadeOut::create(1.0f);
    fadeOut->setDuration(.5f);
    Sequence* seq = Sequence::createWithTwoActions(fadeIn, fadeOut);
    
    RepeatForever* repeat = RepeatForever::create(seq);
    
    startGameLabel->runAction(repeat);
    
    return true;
}

void PogoPainterMenu::update(float dt)
{
    
}

void PogoPainterMenu::registerEventListener(Node* element, EventListener* listener) {
    element->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, element);
}

void PogoPainterMenu::registerEventListener(EventListener* listener) {
    this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}
