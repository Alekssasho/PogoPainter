#include "PogoPainterScene.h"
#include "PogoPainterMenuScene.h"

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
    this->addChild(pSprite, 2);
    
    auto pHumanPlayer = unique_ptr<PPHumanPlayer>(new PPHumanPlayer(Vec2::ZERO, PPColor::Red, *this, pSprite));
    pHumanPlayer->currentDirection = PPDirection::Up;
    
    board.at(0, 0).color = pHumanPlayer->color;
    players.push_back(move(pHumanPlayer));

    this->scheduleUpdate();
    
    this->schedule(CC_SCHEDULE_SELECTOR(PogoPainter::gameTick), TICK_DELAY);

    PPBonusManager::getInstance().surface = this;

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
            this->removeChild(pBonus->sprite);
            PPBonusManager::getInstance().removeBonus(&*pBonus);
            board.at(pl->getPosition()).bonus.reset();
        }
        
        board.at(pl->getPosition()).color = pl->color;
    }
    
    for(auto& pl : players) {
        auto dir = pl->getDirection();
        auto res = board.moveInDir(Vec2(pl->getPosition()), dir);
        if (Vec2(pl->getPosition()) != res) {
            pl->getPosition() = res;
            auto action = MoveTo::create(TICK_DELAY, board.at(pl->getPosition()).sprite->getPosition());
            pl->pSprite->runAction(action);
            
            pl->pSprite->scheduleOnce([this, &pl](float dt) {
                this->board.at(pl->getPosition()).color = pl->color;
            }, TICK_DELAY / 2, "color");
        } else {
            //TODO: feedback on wall hit
        }
    }

    if (ticks % 2) {
        PPBonusManager::getInstance().each([&](PPBonus * bonus) {
            bonus->update(board);
            if (PPArrow * arrow = dynamic_cast<PPArrow*>(bonus)) {
                bonus->sprite->runAction(
                    RotateBy::create(TICK_DELAY / 2, 90)
                    );
            }
        });
    }

    
    //TODO: handle bonus handler
}

void PogoPainter::update(float dt)
{
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
