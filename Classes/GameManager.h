//
//  PPGameManager.h
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/27/14.
//
//

#ifndef __PogoPainter__PPGameManager__
#define __PogoPainter__PPGameManager__

#include "GameState.h"
#include "BonusManager.h"

class GameManager
{
public:
    GameManager(int t = 90);
    
    void init();
    void update(float deltaTime);
    
    GameState& state() { return mState; }
    int timer() const { return mTimer; }
    
    static const float tickDelay;
    
private:
    GameState mState;
    const int mTimer;
};

#endif /* defined(__PogoPainter__PPGameManager__) */
