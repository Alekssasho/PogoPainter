//
//  PogoPainterResultScene.h
//  PogoPainter
//
//  Created by Asen Lekov on 11/8/14.
//
//

#ifndef __PogoPainter__PogoPainterResultScene__
#define __PogoPainter__PogoPainterResultScene__

#include "cocos2d.h"

using namespace cocos2d;
using namespace std;

class PogoPainterResults : public LayerColor
{
private:
    
public:
    static Scene* createScene();
    bool init() override;
    void update(float dt) override;
    
    void registerEventListener(EventListener* listener);
    
    CREATE_FUNC(PogoPainterResults);
};

#endif /* defined(__PogoPainter__PogoPainterResultScene__) */
