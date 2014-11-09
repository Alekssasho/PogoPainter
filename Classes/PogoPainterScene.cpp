#include "PogoPainterScene.h"
#include "PogoPainterMenuScene.h"
#include "SimpleAudioEngine.h"  

#include "PogoPainterResultScene.h"

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

    auto pSprite = Sprite::create("Player/player_red.png");
    pSprite->setPosition(board.at(0, 0).sprite->getPosition());
    pSprite->setScale(board.at(0, 0).sprite->getScale());
    pSprite->setRotation(-(45 + 90));
    this->addChild(pSprite, 2);
    
    auto pHumanPlayer = unique_ptr<PPHumanPlayer>(new PPHumanPlayer(Vec2::ZERO, PPColor::Red, *this, pSprite));
    pHumanPlayer->setDirection(PPDirection::Up);
    
    board.at(0, 0).color = pHumanPlayer->color;
    players.push_back(move(pHumanPlayer));
    
    auto aiSprite = Sprite::create("Player/player_blue.png");
    aiSprite->setPosition(board.at(7, 7).sprite->getPosition());
    aiSprite->setScale(board.at(7, 7).sprite->getScale());
    aiSprite->setRotation(45);
    this->addChild(aiSprite, 2);
    
    auto aiPlayer = unique_ptr<PPStupidAiPlayer>(new PPStupidAiPlayer(Vec2(7, 7), PPColor::Blue, *this, aiSprite));
    aiPlayer->setDirection(PPDirection::Down);
    
    board.at(7, 7).color = aiPlayer->color;
    players.push_back(move(aiPlayer));

    PPBonusManager::getInstance().surface = this;

	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("Sounds/checkpoint.wav");
	CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("Sounds/arrow.wav");
    
    pSprite = Sprite::createWithTexture(textures[PPColor::Red]);
    
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

void PogoPainter::gameTick(float dt)
{
    static int ticks = 0;
    ++ticks;

    PPBonusManager::getInstance().update(board, players);
    for(auto& pl : players) {
        auto& pBonus = board.at(pl->getPosition()).bonus;
        if(pBonus) {
			pBonus->apply(*pl, board);

			if (pl->color == PPColor::Red) {
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
    
    for(auto& pl : players) {
        auto dir = pl->getDirection();
        pl->autorotate();
    
        for (auto& other : players) {
            if (other != pl && &pl->getNextDirectionCell() == &board.at(other->pos)) {
                dir = PPDirection::None;
                break;
            }
        }
        
        auto res = board.moveInDir(pl->getPosition(), dir);
        if (pl->getPosition() != res) {
            pl->pos = res;
            auto action = MoveTo::create(TICK_DELAY, board.at(pl->getPosition()).sprite->getPosition());
            pl->pSprite->runAction(action);
            
            pl->pSprite->scheduleOnce([this, &pl](float dt) {
                this->board.at(pl->getPosition()).color = pl->color;
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
    
    for(auto& pl : players) {
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
