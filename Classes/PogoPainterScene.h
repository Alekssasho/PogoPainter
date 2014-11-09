#ifndef __POGOPAINTER_SCENE_H__
#define __POGOPAINTER_SCENE_H__

#include <map>
#include <vector>
#include <memory>
#include "cocos2d.h"
#include "PPBoard.h"
#include "PPBonus.h"
#include "PPPlayer.h"

using namespace cocos2d;
using namespace std;

#define TICK_DELAY 0.5f

class PogoPainter : public Layer
{
private:
    PPBoard board;
    vector<unique_ptr<PPPlayer>> players;
    map<PPColor, Texture2D*> textures;
	int timer;
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    bool init() override;
    
    void update(float dt) override;
    
    void gameTick(float dt);
    
    PPBoard& getBoard() { return board; }
    vector<unique_ptr<PPPlayer>>& getPlayers() { return players; }
    
    void registerEventListener(EventListener* listener);
    
    // a selector callback
    void menuCloseCallback(Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(PogoPainter);
};

#endif // __HELLOWORLD_SCENE_H__
