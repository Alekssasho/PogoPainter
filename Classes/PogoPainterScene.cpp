#include "PogoPainterScene.h"
#include "PogoPainterMenuScene.h"
#include "SimpleAudioEngine.h"  

#include "PogoPainterResultScene.h"
#include "PPGameManager.h"

USING_NS_CC;

Scene* PogoPainter::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = PogoPainter::create();
    auto menu = PogoPainterMenu::create();

    // add layer as a child to scene
    scene->addChild(layer);
    scene->addChild(menu);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool PogoPainter::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(PogoPainter::menuCloseCallback, this));
    
	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
    
	timer = 90;

    textures[PPColor::Blue]   = Director::getInstance()->getTextureCache()->addImage("Cell/cell_blue.png");
    textures[PPColor::Red]    = Director::getInstance()->getTextureCache()->addImage("Cell/cell_red.png");
    textures[PPColor::Yellow] = Director::getInstance()->getTextureCache()->addImage("Cell/cell_yellow.png");
    textures[PPColor::Green]  = Director::getInstance()->getTextureCache()->addImage("Cell/cell_green.png");
    textures[PPColor::Empty]  = Director::getInstance()->getTextureCache()->addImage("Cell/cell_empty.png");
    
    player_textures[PPPlayer::Color::R] = Director::getInstance()->getTextureCache()->addImage("Player/player_red.png");
    player_textures[PPPlayer::Color::G] = Director::getInstance()->getTextureCache()->addImage("Player/player_green.png");
    player_textures[PPPlayer::Color::B] = Director::getInstance()->getTextureCache()->addImage("Player/player_blue.png");
    player_textures[PPPlayer::Color::Y] = Director::getInstance()->getTextureCache()->addImage("Player/player_yellow.png");

    Vec2 offset(4, 4 - 60);
    auto cellSize = (visibleSize.width - offset.x) / 8;
    
    for(int x = 0; x < 8; ++x) {
        for(int y = 0; y < 8; ++y) {
            board.at(x, y).sprite = Sprite::createWithTexture(textures[board.at(x, y).color]);
            Sprite* pSprite = board.at(x, y).sprite;
            pSprite->setPosition(
                offset +
                Vec2(x * cellSize, y * cellSize) +
                Vec2(cellSize / 2, cellSize / 2)
            );

            pSprite->setPosition(pSprite->getPosition() + Vec2(0, visibleSize.height - 8 * cellSize - 8));
            pSprite->setScale((cellSize - 4) / pSprite->getBoundingBox().size.width);
            this->addChild(pSprite);
        }
    }
    
    PPGameManager::getInstance().setScene(this);
    PPGameManager::getInstance().setBoard(&board);
    PPGameManager::getInstance().setRole(PPGameManager::Server);

    PPGameManager::getInstance().initHuman();
    PPGameManager::getInstance().fillWithAi(3);

    for (auto & player : PPGameManager::getInstance().getPlayers()) {
        attachPlayer(player);
    }
    
    
    PPBonusManager::getInstance().surface = this;

	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("Sounds/checkpoint.wav");
	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("Sounds/arrow.wav");
    
	auto label = Label::createWithTTF("Timer: " + to_string(timer/2), "fonts/Marker Felt.ttf", 30);
	label->setTag(42);

	// position the label on the center of the screen
	label->setPosition(Vec2(visibleSize.width / 2,
		(visibleSize.height - 8 * cellSize - 8)/2.0));

	// add the label as a child to this layer
	this->addChild(label, 1);
    
    return true;
}

Texture2D * PogoPainter::getPlayerTexture(PPPlayer::Color col)
{
    if (player_textures.find(col) != player_textures.end()) {
        return player_textures[col];
    }
    CCLOG("Player texture not found!");
    return NULL;
}

void PogoPainter::gameTick(float dt)
{	
    static int ticks = 0;
	if (ticks % 2 == 0){
		if ((timer - ticks) / 2 <= 10 && (timer - ticks) / 2 > 5)
			CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("Sounds/beep-06.wav");
		else if ((timer - ticks) / 2 <= 5 && (timer - ticks)/2 > 0)
			CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("Sounds/beep-08.wav");
	}
	static_cast<Label*>(this->getChildByTag(42))->setString("Timer: " + to_string((timer - ticks)/2));


	if (timer == ticks) {
		this->unscheduleAllSelectors();		
		this->unscheduleUpdate();
		//TODO: asene call game over sreen here
	}

    ++ticks;

    PPGameManager::getInstance().update(ticks, dt);

    auto active_players = PPGameManager::getInstance().getPlayers();

    PPBonusManager::getInstance().update(board, active_players);
    for (auto& pl : active_players) {
        auto& pBonus = board.at(pl->getPosition()).bonus;
        if(pBonus) {
            auto p = pl->points;
			pBonus->apply(*pl, board);

			if (dynamic_cast<PPHumanPlayer*>(pl)) {
				if (dynamic_cast<PPCheckpoint*>(&*pBonus)) {
					CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect(
						"Sounds/checkpoint.wav");
                    
                    auto label = Label::createWithTTF("+ " + to_string(pl->points - p), "fonts/Marker Felt.ttf", 50);
                    label->setTextColor(Color4B::RED);
                    label->setPosition(pl->pSprite->getPosition());
                    label->runAction(Sequence::create(
                                                      DelayTime::create(0.5f),
                                                      MoveTo::create(1, Vec2(this->getChildByTag(100)->getPosition().x,
                                                                             Director::getInstance()->getVisibleSize().height)),
                                                      RemoveSelf::create(),
                                                      NULL));
                    this->addChild(label, 10);
                    
				}
				else if (dynamic_cast<PPArrow*>(&*pBonus)) {
					CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect(
						"Sounds/arrow.wav");
				}
			}

            this->removeChild(pBonus->sprite);
            PPBonusManager::getInstance().removeBonus(&*pBonus);
            board.at(pl->getPosition()).bonus.reset();

        }
        
        board.at(pl->getPosition()).color = pl->color;
    }
    
    int maxPoints = 0;
    
    for (auto& pl : active_players) {
        auto dir = pl->getDirection();
        pl->autorotate();
        
        /*
        for (auto& other : active_players) {
            if (other != pl && &pl->getNextDirectionCell() == &board.at(other->pos)) {
                dir = PPDirection::None;
                break;
            }
        }
        */        
        auto res = board.moveInDir(pl->getPosition(), dir);
        if (pl->getPosition() != res) {
            pl->pos = res;
            auto action = MoveTo::create(TICK_DELAY, board.at(pl->getPosition()).sprite->getPosition());
            pl->pSprite->runAction(action);
            auto dest = pl->getPosition();
            auto color = pl->color;
            
            pl->pSprite->scheduleOnce([this, dest, color](float dt) {
                this->board.at(dest).color = color;
            }, TICK_DELAY / 2, "color");
        } else {
            //TODO: feedback on wall hit
        }
        
        maxPoints += pl->points + 10;
    }

    if (ticks % 2) {
        PPBonusManager::getInstance().each([&](PPBonus * bonus) {
            bonus->update(board);
            if (dynamic_cast<PPArrow*>(bonus)) {
                bonus->sprite->runAction(
                    RotateBy::create(TICK_DELAY / 2, 90)
                    );
            }
        });
    }
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto coef = visibleSize.width / maxPoints;
    
    float sizeTillNow = 0;
    for (auto& pl : active_players) {
        float size = (pl->points + 10) * coef;
        
        auto pSprite = static_cast<Sprite*>(this->getChildByTag(100 + static_cast<int>(pl->color) * 100));
        pSprite->setPosition(Vec2(sizeTillNow + size / 2.0, visibleSize.height - 30));
        pSprite->setScaleX(size / pSprite->getContentSize().width);
        
        sizeTillNow += pSprite->getBoundingBox().size.width;
    }
}

void PogoPainter::update(float dt)
{
    static bool init = false;
    if(!init) {
        this->schedule(CC_SCHEDULE_SELECTOR(PogoPainter::gameTick), TICK_DELAY);
        init = true;
    }
    
    for(auto& cell : board.cells) {
        cell.sprite->setTexture(textures[cell.color]);
    }
}

void PogoPainter::attachPlayer(PPPlayer* player)
{
    int rotation;
    switch (player->color) {
        case PPColor::Red:
        {
            rotation = - (45 + 90);
            
            auto visibleSize = Director::getInstance()->getVisibleSize();
            Sprite* pS = Sprite::createWithTexture(textures[player->color]);
            
            pS->setPosition(visibleSize.width / 8.0, visibleSize.height - 30);
            pS->setScaleY(60 / pS->getBoundingBox().size.height);
            pS->setScaleX((visibleSize.width / 4) / pS->getContentSize().width);
            this->addChild(pS, 0, 100);
            
            break;
        }
        case PPColor::Blue:
        {
            rotation = 45;
            
            auto visibleSize = Director::getInstance()->getVisibleSize();
            Sprite* pS = Sprite::createWithTexture(textures[player->color]);
            
            pS->setPosition((visibleSize.width / 8.0) * 3, visibleSize.height - 30);
            pS->setScaleY(60 / pS->getBoundingBox().size.height);
            pS->setScaleX((visibleSize.width / 4) / pS->getContentSize().width);
            this->addChild(pS, 0, 200);
            break;
        }
        case PPColor::Green:
        {
            rotation = -45;
            
            auto visibleSize = Director::getInstance()->getVisibleSize();
            Sprite* pS = Sprite::createWithTexture(textures[player->color]);
            
            pS->setPosition((visibleSize.width / 8.0) * 5, visibleSize.height - 30);
            pS->setScaleY(60 / pS->getBoundingBox().size.height);
            pS->setScaleX((visibleSize.width / 4) / pS->getContentSize().width);
            this->addChild(pS, 0, 300);
            break;
        }
        case PPColor::Yellow:
        {
            rotation = -45 + 180;
            
            auto visibleSize = Director::getInstance()->getVisibleSize();
            Sprite* pS = Sprite::createWithTexture(textures[player->color]);
            
            pS->setPosition((visibleSize.width / 8.0) * 7, visibleSize.height - 30);
            pS->setScaleY(60 / pS->getBoundingBox().size.height);
            pS->setScaleX((visibleSize.width / 4) / pS->getContentSize().width);
            this->addChild(pS, 0, 400);
            
            break;
        }
        default:
            break;
    }
    
    auto pos = player->getPosition();
    player->pSprite->setPosition(board.at(pos).sprite->getPosition());
    player->pSprite->setScale(board.at(pos).sprite->getScale());
    player->pSprite->setRotation(rotation);
    this->addChild(player->pSprite, 2);
    
    board.at(pos).color = player->color;
}

void PogoPainter::registerEventListener(EventListener* listener)
{
    this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void PogoPainter::menuCloseCallback(Ref* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
    return;
#endif

    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}
