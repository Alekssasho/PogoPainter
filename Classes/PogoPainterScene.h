#ifndef __POGOPAINTER_SCENE_H__
#define __POGOPAINTER_SCENE_H__

#include <map>
#include <vector>
#include <array>
#include "cocos2d.h"
#include "GameManager.h"

using namespace cocos2d;
using namespace std;

class PogoPainter : public Layer
{
private:
    map<Color, Texture2D*> textures;
    std::array<Sprite*, Board::boardSize * Board::boardSize> spriteCells;
    std::vector<Sprite*> spritePlayers;
    ClientConnection manager;
    
    void handleLine();
public:
    PogoPainter();
    
    static Scene* createScene();

    bool init() override;
    
    void update(float dt) override;
    
    void gameTick(float dt);
    
    void attachPlayers();
    void registerEventListener(EventListener* listener);
    
    CREATE_FUNC(PogoPainter);
};

#endif // __HELLOWORLD_SCENE_H__
