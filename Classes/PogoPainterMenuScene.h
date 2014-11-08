//
//  PogoPainterMenuScene.h
//  PogoPainter
//
//  Created by Asen Lekov on 11/8/14.
//
//

#ifndef __PogoPainter__PogoPainterMenuScene__
#define __PogoPainter__PogoPainterMenuScene__

#include "cocos2d.h"

using namespace cocos2d;
using namespace std;

class PogoPainterMenu : public LayerColor
{
private:
    void registerEventListener(Node* element, EventListener* listener);
public:
    static Scene* createScene();
    bool init() override;
    void update(float dt) override;
    
    void registerEventListener(EventListener* listener);
    
    CREATE_FUNC(PogoPainterMenu);
};

#endif /* defined(__PogoPainter__PogoPainterMenuScene__) */
