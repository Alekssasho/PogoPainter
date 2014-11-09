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
	layer->setTag(2);

	scene->addChild(layer);

	return scene;
}

bool PogoPainterResults::init()
{
	if (!LayerColor::initWithColor(Color4B(0, 0, 0, 128))) {
		return false;
	}

	return true;
}

void PogoPainterResults::update(float dt)
{

}

void PogoPainterResults::setResults(vector<int> res)
{
	results = res;



	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();


	auto statLabel = LabelTTF::create("", "Marker Felt.ttf", 48);
	statLabel->setPosition(visibleSize.width / 2, visibleSize.height - 40);

	bool win = true;
	for (auto& p : results){
		if (results.front() < p){
			win = false;
			break;
		}
	}
	statLabel->setString(win ? "You Win!" : "Ha noob");
	this->addChild(statLabel);

	FadeIn* fadeIn = FadeIn::create(.5f);
	fadeIn->setDuration(1.f);
	FadeOut* fadeOut = FadeOut::create(1.0f);
	fadeOut->setDuration(.5f);
	Sequence* seq = Sequence::createWithTwoActions(fadeIn, fadeOut);

	RepeatForever* repeat = RepeatForever::create(seq);

	statLabel->runAction(repeat);

	auto label = LabelTTF::create("Points:", "Marker Felt.ttf", 30);
	label->setPosition(visibleSize.width / 8, visibleSize.height - 120);
	this->addChild(label);

	Sprite* red;
	int rotation = -45;
	red = Sprite::create("Player/player_red.png");
	red->setRotation(rotation);
	red->setPosition(visibleSize.width / 8.0, visibleSize.height - 200);
	this->addChild(red);

	Sprite* green;
	green = Sprite::create("Player/player_green.png");
	green->setPosition(visibleSize.width / 8.0, visibleSize.height - 300);
	green->setRotation(rotation);
	//green->setScale();
	this->addChild(green);

	Sprite* blue;
	blue = Sprite::create("Player/player_blue.png");
	blue->setPosition(visibleSize.width / 8.0, visibleSize.height - 400);
	blue->setRotation(rotation);
	//blue->setScale();
	this->addChild(blue);

	Sprite* yellow;
	yellow = Sprite::create("Player/player_yellow.png");
	yellow->setPosition(visibleSize.width / 8.0, visibleSize.height - 500);
	yellow->setRotation(rotation);
	//yellow->setScale();
	this->addChild(yellow);


	int i = 200;
	for (auto& p : results){

		auto score = LabelTTF::create(to_string(p), "Marker Felt.ttf", 48);
		score->setPosition(visibleSize.width / 2, visibleSize.height - i);
		this->addChild(score);
		i += 100;
	}

}
