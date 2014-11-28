#include "PogoPainterScene.h"
#include "PogoPainterMenuScene.h"
#include "SimpleAudioEngine.h"  

#include "PogoPainterResultScene.h"
#include "Macros.h"

USING_NS_CC;

#define SPRITE_CELL(pos) this->spriteCells[(pos).x + (pos).y * Board::boardSize]

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
template<typename T>
string to_string(const T& t) {
    ostringstream os;
    os << t;
    return os.str();
}
#endif

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
//TODO: we need destroy method so we can clean up the whole mess, restarting the game in the moment doesn't work properly

// on "init" you need to initialize your instance
bool PogoPainter::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}

	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
    manager.init();

	textures[Color::Blue] = Director::getInstance()->getTextureCache()->addImage("Cell/cell_blue.png");
	textures[Color::Red] = Director::getInstance()->getTextureCache()->addImage("Cell/cell_red.png");
	textures[Color::Yellow] = Director::getInstance()->getTextureCache()->addImage("Cell/cell_yellow.png");
	textures[Color::Green] = Director::getInstance()->getTextureCache()->addImage("Cell/cell_green.png");
	textures[Color::Gray] = Director::getInstance()->getTextureCache()->addImage("Cell/cell_empty.png");

	Vec2 offset(4, 4 - 60);
	auto cellSize = (visibleSize.width - offset.x) / 8;

    auto& board = manager.state().board();
    
    for (int x = 0; x < Board::boardSize; ++x) {
        for (int y = 0; y < Board::boardSize; ++y) {
            Sprite* pSprite = spriteCells[x + y * Board::boardSize] = Sprite::createWithTexture(textures[board[Vec2(x, y)].color]);
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

    this->attachPlayers();

	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("Sounds/checkpoint.wav");
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("Sounds/arrow.wav");
	CocosDenshion::SimpleAudioEngine::getInstance()->preloadEffect("Sounds/speed.wav");

	auto label = Label::createWithTTF("Timer: " + to_string(manager.timer() / 2), "fonts/Marker Felt.ttf", 30);
	label->setTag(4200);

	// position the label on the center of the screen
	label->setPosition(Vec2(visibleSize.width / 2,
		(visibleSize.height - 8 * cellSize - 8) / 2.0));

	// add the label as a child to this layer
	this->addChild(label, 1);
    
    
    //TODO: make this resolution independant
    auto indicator = Sprite::create("General/indicator.png");
    
    indicator->setPosition(Vec2(visibleSize.width / 2, (visibleSize.height - 8 * cellSize) / 2.0));
    indicator->setScale(visibleSize.width / indicator->getContentSize().width,
                        (visibleSize.height - 8 * cellSize) / indicator->getContentSize().height);
    
    indicator->setOpacity(175);
    indicator->setPosition(indicator->getPosition() - Vec2(0, cellSize - 24));
    this->addChild(indicator);
    
    
    //Attach delegates

    ADD_DELEGATE("RotatePlayer", [this](EventCustom* e) {
        auto pl = static_cast<Player*>(e->getUserData());
        int spriteRotateAngle = -45;
        switch (pl->currentDirection) {
            case Up:
                spriteRotateAngle -= 90;
                break;
            case Down:
                spriteRotateAngle += 90;
                break;
            case Left:
                spriteRotateAngle += 180;
                break;
            default:
                break;
        }
        auto pSprite = this->spritePlayers[pl->color];
        pSprite->stopActionByTag(15);
        auto rotate = RotateTo::create(0.15f, spriteRotateAngle);
        rotate->setTag(15);
        pSprite->runAction(rotate);
    });

    ADD_DELEGATE("NewBonus", [this](EventCustom* e) {
        auto pBonus = static_cast<Bonus*>(e->getUserData());
        Sprite* pSprite;
        if(dynamic_cast<Checkpoint*>(pBonus)) {
            pSprite = Sprite::create("Bonuses/bonus_checkpoint.png");
        } else if(dynamic_cast<Arrow*>(pBonus)) {
            pSprite = Sprite::create("Bonuses/bonus_arrow.png");
        }
        
        auto& spriteCell = SPRITE_CELL(Vec2(pBonus->cell.x, pBonus->cell.y));
        pSprite->setPosition(spriteCell->getPosition());
        pSprite->setScale(0.01);
        auto targetScale = spriteCell->getScale();
        pSprite->runAction(ScaleTo::create(0.20f, targetScale));
        pSprite->setTag(pBonus->cell.x + pBonus->cell.y * Board::boardSize);
        this->addChild(pSprite);
    });
    
    ADD_DELEGATE("RemoveBonus", [this](EventCustom* e) {
        auto pBonus = static_cast<Bonus*>(e->getUserData());
        this->removeChildByTag(pBonus->cell.x + pBonus->cell.y * Board::boardSize);
    });
    
    ADD_DELEGATE("RemoveColor", [this](EventCustom* e) {
        auto pCell = static_cast<Cell*>(e->getUserData());
        auto spriteCell = SPRITE_CELL(Vec2(pCell->x, pCell->y));
        auto currentTexture = spriteCell->getTexture();
        
        //TODO: check if there is bug when cells are blinking and new player colors the blinking cell ???
        auto action = Repeat::create(Sequence::create(
                                                      DelayTime::create(0.08f),
                                                      CallFunc::create([spriteCell, currentTexture]{ spriteCell->setTexture(currentTexture); }),
                                                      DelayTime::create(0.08f),
                                                      CallFunc::create([spriteCell, this]{ spriteCell->setTexture(this->textures[Color::Gray]); }), NULL), 2);
        
        spriteCell->runAction(action);
    });
    
    ADD_DELEGATE("Score", [this](EventCustom* e) {
        auto pPoints = static_cast<int*>(e->getUserData());
        auto label = Label::createWithTTF("+ " + to_string(*pPoints), "fonts/Marker Felt.ttf", 50);
        label->setTextColor(Color4B::RED);
        label->setPosition(this->spritePlayers[Color::Red]->getPosition());
        label->runAction(Sequence::create(
                                          DelayTime::create(0.5f),
                                          MoveTo::create(1, Vec2(this->getChildByTag(100)->getPosition().x,
                                                                 Director::getInstance()->getVisibleSize().height)),
                                          RemoveSelf::create(),
                                          NULL));
        this->addChild(label, 10);
    });
    
    
    ADD_DELEGATE("RotateArrow", [this](EventCustom* e) {
        auto pBonus = static_cast<Bonus*>(e->getUserData());
        this->getChildByTag(pBonus->cell.x + pBonus->cell.y * Board::boardSize)->runAction(RotateBy::create(GameManager::tickDelay / 2, 90));
    });
    
    ADD_DELEGATE("TriggerArrow", [this](EventCustom* e) {
        auto pPl = static_cast<Player*>(e->getUserData());
        auto dir = static_cast<Arrow*>(this->manager.state().board()[pPl->pos].pBonus)->dir;
        auto pos = pPl->pos;
        auto color = pPl->color;
        
        switch (dir)
        {
            case Left:
                for (int c = pos.x; c >= 0; --c){
                    int y = pos.y;
                    SPRITE_CELL(Vec2(c, pos.y))->runAction(
                                      Sequence::createWithTwoActions(
                                                                     DelayTime::create(0.05f * (pos.x - c)),
                                                                     CallFunc::create([color, y, c, this] {
                                          SPRITE_CELL(Vec2(c, y))->setTexture(this->textures[color]);
                                      })));
                }
                break;
            case Up:
                for (int c = pos.y; c < Board::boardSize; ++c) {
                    int x = pos.x;
                    SPRITE_CELL(Vec2(pos.x, c))->runAction(
                                  Sequence::createWithTwoActions(
                                                                 DelayTime::create(0.05f * (c - pos.y)),
                                                                 CallFunc::create([color, this, x, c] {
                                      SPRITE_CELL(Vec2(x, c))->setTexture(this->textures[color]);
                                  })));
                }
                break;
            case Right:
                for (int c = pos.x; c < Board::boardSize; ++c) {
                    int y = pos.y;
                    SPRITE_CELL(Vec2(c, pos.y))->runAction(
                                  Sequence::createWithTwoActions(
                                                                 DelayTime::create(0.05f * (c - pos.x)),
                                                                 CallFunc::create([color, this, y, c] {
                                      SPRITE_CELL(Vec2(c, y))->setTexture(this->textures[color]);
                                  })));
                }
                break;
            case Down:
                for (int c = pos.y; c >= 0; --c) {
                    int x = pos.x;
                    SPRITE_CELL(Vec2(pos.x, c))->runAction(
                                  Sequence::createWithTwoActions(
                                                                 DelayTime::create(0.05f * (pos.y - c)),
                                                                 CallFunc::create([color, this, x, c] {
                                      SPRITE_CELL(Vec2(x, c))->setTexture(this->textures[color]);
                                  })));
                    
                }
                break;
            default:
                break;
        }
    });
    
    //Event listener for player input
    auto eventListener = EventListenerTouchOneByOne::create();
    
    eventListener->onTouchBegan = [](Touch* touch, Event* event) {
        return true;
    };
    
    eventListener->onTouchEnded = [this](Touch* touch, Event* event) {
        auto startTouch = touch->getStartLocation();
        auto diff = touch->getLocation() - startTouch;
        Direction dir;
        
        //check some minimal length for drag
        if(diff.lengthSquared() <= 50*50)
            return;
        diff.normalize();
        
        float angle = Vec2::angle(diff, Vec2(1, 0));
        
        if(angle <= M_PI_4) {
            dir = Direction::Right;
        } else if(angle <= M_PI_2 + M_PI_4) {
            dir = diff.y > 0 ? Direction::Up : Direction::Down;
        } else {
            dir = Direction::Left;
        }
        
        SEND_EVENT("Swipe", &dir);
    };
    
    this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, this);
    
	return true;
}

void PogoPainter::gameTick(float dt)
{
    int timer = manager.timer();
    int ticks = manager.state().ticks();
	static_cast<Label*>(this->getChildByTag(4200))->setString("Timer: " + to_string((timer - ticks) / 2));

	if (timer == ticks) {
		this->unscheduleAllCallbacks();
		this->unscheduleUpdate();

		vector<int> scores;
		for (auto& pl : manager.state().players()) {
			scores.push_back(pl->points);
		}

		auto director = Director::getInstance();
		auto newScene = PogoPainterResults::createScene();
		
		static_cast<PogoPainterResults*>(newScene->getChildByTag(2))->setResults(scores);
		director->pushScene(newScene);
        return;
	}
    
    manager.update(dt);
    
    auto& players = manager.state().players();
    
    //Init animation for players
    for(auto& pl : players) {
        auto action = MoveTo::create(GameManager::tickDelay, SPRITE_CELL(pl->pos)->getPosition());
        this->spritePlayers[static_cast<int>(pl->color)]->runAction(action);
    
        auto secondAction = Sequence::createWithTwoActions(DelayTime::create(GameManager::tickDelay / 2), CallFunc::create([this, &pl]{
            SPRITE_CELL(pl->pos)->setTexture(this->textures[pl->color]);
        }));
        
        SPRITE_CELL(pl->pos)->runAction(secondAction);
    }
    
    
    //Handle line above
    
    int maxPoints = std::accumulate(players.begin(), players.end(), 0, [](const int acc, const PlayerPtr& pPl) {
        return acc + pPl->points;
    });
    
    //Ugly hack ask Aleksandar for explanation
    maxPoints += 40;
    
    Color maxPointColor = std::max_element(players.begin(), players.end(), [](const PlayerPtr& lhs, const PlayerPtr& rhs) {
        return lhs->points < rhs->points;
    })->get()->color;
    
	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto coef = visibleSize.width / maxPoints;

	float sizeTillNow = 0;
	for (auto& pl : players) {
        //+ 10 because of ugly hack look up :D
		float size = (pl->points + 10) * coef;

		auto pSprite = static_cast<Sprite*>(this->getChildByTag(100 + static_cast<int>(pl->color) * 100));
		pSprite->setPosition(Vec2(sizeTillNow + size / 2.0, visibleSize.height - 30));
		pSprite->setScaleX(size / pSprite->getContentSize().width);
        
        if(maxPointColor != pl->color) {
            pSprite->setOpacity(111);
        } else {
            pSprite->setOpacity(255);
        }

		sizeTillNow += pSprite->getBoundingBox().size.width;
	}
}

void PogoPainter::update(float dt)
{
	if (!mInit) {
		this->schedule(CC_SCHEDULE_SELECTOR(PogoPainter::gameTick), GameManager::tickDelay);
		mInit = true;
	}
}

void PogoPainter::attachPlayers()
{
	int rotation;
	Sprite* pSprite;
    for(auto& pPl : manager.state().players()) {
        switch (pPl->color) {
        case Color::Red:
        {
             rotation = -(45 + 90);
             pSprite = Sprite::create("Player/player_red.png");

             auto visibleSize = Director::getInstance()->getVisibleSize();
             Sprite* pS = Sprite::createWithTexture(textures[pPl->color]);

             pS->setPosition(visibleSize.width / 8.0, visibleSize.height - 30);
             pS->setScaleY(60 / pS->getBoundingBox().size.height);
             pS->setScaleX((visibleSize.width / 4) / pS->getContentSize().width);
             this->addChild(pS, 0, 100);

             break;
        }
        case Color::Blue:
        {
              rotation = 45;
              pSprite = Sprite::create("Player/player_blue.png");
            
              auto visibleSize = Director::getInstance()->getVisibleSize();
              Sprite* pS = Sprite::createWithTexture(textures[pPl->color]);

              pS->setPosition((visibleSize.width / 8.0) * 5, visibleSize.height - 30);
              pS->setScaleY(60 / pS->getBoundingBox().size.height);
              pS->setScaleX((visibleSize.width / 4) / pS->getContentSize().width);
              this->addChild(pS, 0, 200);
              break;
        }
        case Color::Green:
        {
               rotation = -45;
               pSprite = Sprite::create("Player/player_green.png");
            
               auto visibleSize = Director::getInstance()->getVisibleSize();
               Sprite* pS = Sprite::createWithTexture(textures[pPl->color]);

               pS->setPosition((visibleSize.width / 8.0) * 3, visibleSize.height - 30);
               pS->setScaleY(60 / pS->getBoundingBox().size.height);
               pS->setScaleX((visibleSize.width / 4) / pS->getContentSize().width);
               this->addChild(pS, 0, 300);
               break;
        }
        case Color::Yellow:
        {
                rotation = -45 + 180;
            pSprite = Sprite::create("Player/player_yellow.png");
            
                auto visibleSize = Director::getInstance()->getVisibleSize();
                Sprite* pS = Sprite::createWithTexture(textures[pPl->color]);

                pS->setPosition((visibleSize.width / 8.0) * 7, visibleSize.height - 30);
                pS->setScaleY(60 / pS->getBoundingBox().size.height);
                pS->setScaleX((visibleSize.width / 4) / pS->getContentSize().width);
                this->addChild(pS, 0, 400);
                break;
        }
        default:
            break;
        }
        
        auto& pos = pPl->pos;

        pSprite->setPosition(SPRITE_CELL(pos)->getPosition());
        pSprite->setScale(SPRITE_CELL(pos)->getScale());
        pSprite->setRotation(rotation);
        this->addChild(pSprite, 2);
        
        SPRITE_CELL(pos)->setTexture(textures[pPl->color]);

        spritePlayers.push_back(pSprite);
    }
}

