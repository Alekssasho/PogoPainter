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
    PPPlayer(int cx, int cy, PPColor c, PogoPainter& scene);
    
    virtual PPDirection getDirection() = 0;
    
    int x, y;
    PPColor color;
private:
    PPBoard& board;
};

class PPHumanPlayer : public PPPlayer
{
public:
    PPHumanPlayer(int x, int y, PPColor c, PogoPainter& scene);
    
    PPDirection getDirection() override;
private:
    PPDirection currentDirection;
};

#endif /* defined(__PogoPainter__PPPlayer__) */
