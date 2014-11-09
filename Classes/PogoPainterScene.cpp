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
        player->pSprite->setPosition(board.at(player->getPosition()).sprite->getPosition());
        player->pSprite->setScale(board.at(player->getPosition()).sprite->getScale());
        player->pSprite->setRotation(-(45 + 90));
        board.at(player->getPosition()).color = player->color;
        this->addChild(player->pSprite, 2);
    }
    
    
    PPBonusManager::getInstance().surface = this;

	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("Sounds/checkpoint.wav");
	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("Sounds/arrow.wav");
    
    auto pSprite = Sprite::createWithTexture(textures[PPColor::Red]);
    
    pSprite->setPosition(visibleSize.width / 4.0, visibleSize.height - 30);
    pSprite->setScaleY(60 / pSprite->getBoundingBox().size.height);
    pSprite->setScaleX((visibleSize.width / 2) / pSprite->getContentSize().width);
    this->addChild(pSprite, 0, 100);
    pSprite = Sprite::createWithTexture(textures[PPColor::Blue]);
    pSprite->setPosition((visibleSize.width / 4.0) * 3, visibleSize.height - 30);
    pSprite->setScaleY(60 / pSprite->getBoundingBox().size.height);
    pSprite->setScaleX((visibleSize.width / 2) / pSprite->getContentSize().width);
    this->addChild(pSprite, 0, 200);
    
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
    ++ticks;

    PPGameManager::getInstance().update(ticks, dt);

    auto active_players = PPGameManager::getInstance().getPlayers();

    PPBonusManager::getInstance().update(board, active_players);
    for (auto& pl : active_players) {
        auto& pBonus = board.at(pl->getPosition()).bonus;
        if(pBonus) {
			pBonus->apply(*pl, board);

			if (dynamic_cast<PPHumanPlayer*>(pl)) {
				if (dynamic_cast<PPCheckpoint*>(&*pBonus)) {
					CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect(
						"Sounds/checkpoint.wav");
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
    
    for (auto& pl : active_players) {
        float size = (pl->points + 10) * coef;
        switch (pl->color) {
            case PPColor::Red:
            {
                auto pSprite = static_cast<Sprite*>(this->getChildByTag(100));
                pSprite->setPosition(Vec2(size / 2.0, visibleSize.height - 30));
                auto s = pSprite->getBoundingBox();
                pSprite->setScaleX(size / pSprite->getContentSize().width);
                break;
            }
            case PPColor::Blue:
            {
                auto pSprite = static_cast<Sprite*>(this->getChildByTag(200));
                pSprite->setPosition(Vec2(visibleSize.width - size / 2.0, visibleSize.height - 30));
                pSprite->setScaleX(size / pSprite->getContentSize().width);
                break;
            }
            default:
                break;
        }
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
