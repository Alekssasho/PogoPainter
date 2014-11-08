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
    
    PPCell& getNextDirectionCell()
    {
        switch (getDirection()) {
            case Up: return board.at(pos.x, pos.y + 1);
            case Down: return board.at(pos.x, pos.y - 1);
            case Left: return board.at(pos.x - 1, pos.y);
            case Right: return board.at(pos.x + 1, pos.y);
            default: return board.at(pos.x, pos.y);
        }
    }

    Vec2 pos;
    int points;
    PPColor color;
    Sprite* pSprite;
    
    virtual ~PPPlayer() {}
    
protected:
    PPDirection currentDirection;
    PPBoard& board;
};

class PPHumanPlayer : public PPPlayer
{
public:
    PPHumanPlayer(const Vec2 & pos, PPColor c, PogoPainter& scene, Sprite* pSprite);
    
    PPCell& getNextDirectionCell();
    PPDirection getDirection() override;
private:
};

class PPStupidAiPlayer : public PPPlayer
{
public:
    PPStupidAiPlayer(const Vec2 & pos, PPColor c, PogoPainter& scene, Sprite* pSprite);
    
    PPDirection getDirection() override;
private:
    PPBonus* following = nullptr;
    
    double calcDistanceFromPlayerTo(int x, int y) const;
    double calcDistanceFromTo(int fromX, int fromY, int toX, int toY) const;
    PPBonus* getNearestBonus(std::vector<PPBonus*> bonuses);
    PPCheckpoint* getNearestCheckpoint(std::vector<PPCheckpoint*> checkpoints);
    PPDirection getNextDirection();
};

#endif /* defined(__PogoPainter__PPPlayer__) */
