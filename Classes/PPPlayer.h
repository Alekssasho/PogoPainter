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
    PPPlayer(const Vec2 & pos, PPColor c, PogoPainter& scene, Sprite* pSprite);
    
    virtual PPDirection getDirection() = 0;
    Vec2 & getPosition() {
        return pos;
    }
    
    void setDirection(PPDirection dir)
    {
        currentDirection = dir;
    }

    Vec2 pos;
    int points;
    PPColor color;
    Sprite* pSprite;
    
protected:
    PPDirection currentDirection;
    PPBoard& board;
};

class PPHumanPlayer : public PPPlayer
{
public:
    PPHumanPlayer(const Vec2 & pos, PPColor c, PogoPainter& scene, Sprite* pSprite);
    
    PPDirection getDirection() override;
private:
};

class PPStupidAiPlayer : public PPPlayer
{
public:
    PPStupidAiPlayer(const Vec2 & pos, PPColor c, PogoPainter& scene, Sprite* pSprite);
    
    PPDirection getDirection() override;
private:
    double calcDistanceTo(int x, int y) const;
};

#endif /* defined(__PogoPainter__PPPlayer__) */
