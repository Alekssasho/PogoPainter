//
//  PPPlayer.h
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/8/14.
//
//

#ifndef __PogoPainter__PPPlayer__
#define __PogoPainter__PPPlayer__

#include "PPBoard.h"
#include "PPBonus.h"

class PogoPainter;

class PPPlayer
{
public:
    PPPlayer(int cx, int cy, PPColor c, PogoPainter& scene, Sprite* pSprite);
    
    virtual PPDirection getDirection() = 0;
    Vec2 getPosition() const {
        return Vec2(x, y);
    }

    int x, y;
    int points;
    PPColor color;
    Sprite* pSprite;
private:
    PPBoard& board;
};

class PPHumanPlayer : public PPPlayer
{
public:
    PPHumanPlayer(int x, int y, PPColor c, PogoPainter& scene, Sprite* pSprite);
    
    PPDirection getDirection() override;
    
    PPDirection currentDirection;
private:
};

#endif /* defined(__PogoPainter__PPPlayer__) */
