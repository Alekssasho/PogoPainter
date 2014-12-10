//
//  SpecificPlayers.h
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/27/14.
//
//

#ifndef __PogoPainter__SpecificPlayers__
#define __PogoPainter__SpecificPlayers__

#include "BaseClasses.h"
#include "BonusManager.h"

class HumanPlayer : public Player
{
public:
    HumanPlayer();
    Direction getDirection() override;
private:
};

class StupidAiPlayer : public Player
{
public:
    Direction getDirection() override;
    
protected:
    Bonus* following = nullptr;
    
    double calcDistanceFromPlayerTo(int x, int y) const;
    double calcDistanceFromTo(int fromX, int fromY, int toX, int toY) const;
    Bonus* getNearestBonus(const std::vector<BonusPtr>& bonuses);
    Bonus* getNearestCheckpoint(const std::vector<BonusPtr>& checkpoints);
    Direction getNextDirection();
};

struct ServerPlayer : public Player 
{
    ServerPlayer() {}
    ServerPlayer(Vec2 p, Color c, Direction initialDirection)
    {
        color = c;
        pos = p;
        currentDirection = initialDirection;
    }
    Direction getDirection() override { return currentDirection; };
};

#endif /* defined(__PogoPainter__SpecificPlayers__) */
