#include "PogoPainterScene.h"

USING_NS_CC;

Scene* PogoPainter::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = PogoPainter::create();

    // add layer as a child to scene
    scene->addChild(layer);

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
    
    Vec2 offset(4, 4);
    auto cellSize = (visibleSize.width - offset.x) / 8;
    
    for(int x = 0; x < 8; ++x) {
        for(int y = 0; y < 8; ++y) {
            board.at(x, y).sprite = Sprite::createWithTexture(textures[board.at(x, y).color]);
            Sprite* pSprite = board.at(x, y).sprite;
            pSprite->setPosition(
                offset +
                Vec2(y * cellSize, x * cellSize) +
                Vec2(cellSize / 2, cellSize / 2)
            );

            pSprite->setPosition(pSprite->getPosition() + Vec2(0, visibleSize.height - 8 * cellSize - 8));
            pSprite->setScale((cellSize - 4) / pSprite->getBoundingBox().size.width);
            this->addChild(pSprite);
        }
    }
    
    players.push_back(unique_ptr<PPHumanPlayer>(new PPHumanPlayer(0, 0, PPColor::Red, *this)));
    
    this->scheduleUpdate();
    
    this->schedule(CC_SCHEDULE_SELECTOR(PogoPainter::gameTick), 1);
    
    return true;
}

void PogoPainter::gameTick(float dt)
{
    for(auto& pl : players) {
        auto pBonus = board.at(pl.x, pl.y).bonus;
        if(pBonus) {
            pBonus->apply(pl, board);
            board.at(pl.x, pl.y).bonus.reset();
        }
    }
    
    for(auto& pl : players) {
        auto dir = pl->getDirection();
        auto res = board.moveInDir(Vec2(pl.x, pl.y), dir);
        if(Vec2(pl.x, pl.y) != res) {
            pl.x = res.x, pl.y = res.y;
            //Add move animation
        } else {
            //TODO: feedback on wall hit
        }
    }
    
    for(auto& cell : board.cells) {
        if(cell.bonus) {
            cell.bonus->update(board);
        }
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
