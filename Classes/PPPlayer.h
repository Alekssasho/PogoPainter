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
        return board.at(board.moveInDir(pos, getDirection()));
    }
    
    void autorotate();

    Vec2 pos;
    int points = 0;
    int slowed = 0;
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
protected:
    PPBonus* following = nullptr;
    
    double calcDistanceFromPlayerTo(int x, int y) const;
    double calcDistanceFromTo(int fromX, int fromY, int toX, int toY) const;
    PPBonus* getNearestBonus(std::vector<PPBonus*> bonuses);
    PPCheckpoint* getNearestCheckpoint(std::vector<PPCheckpoint*> checkpoints);
    PPDirection getNextDirection();
};

class PPLessStupidAiPlayer : public PPStupidAiPlayer
{
public:

    bool go_for_arrow = true;
    PPLessStupidAiPlayer(const Vec2 & pos, PPColor c, PogoPainter& scene, Sprite* pSprite)
        : PPStupidAiPlayer(pos, c, scene, pSprite)
    {}

    PPDirection getDirection() override;
private:
};


#endif /* defined(__PogoPainter__PPPlayer__) */
