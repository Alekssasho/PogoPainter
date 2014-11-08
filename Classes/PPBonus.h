//
//  PPBonus.h
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/8/14.
//
//

#ifndef __PogoPainter__PPBonus__
#define __PogoPainter__PPBonus__

class PPPlayer;
class PPBoard;
class PPCell;

enum PPDirection
{
    Left, Up, Right, Down
};

class PPBonus
{
public:
    PPBonus(PPCell& cell)
    : mCell(cell)
    {}
    
    virtual void apply(PPPlayer& pPlayer, PPBoard& board) = 0;
    virtual void update(PPBoard& board);
    virtual ~PPBonus() {}
private:
    PPCell& mCell;
};

class PPCheckpoint : public PPBonus
{
public:
    PPCheckpoint(PPCell& cell)
    :PPBonus(cell)
    {}
    
    void apply(PPPlayer& pPlayer, PPBoard& board) override;
private:
};

class PPArrow : public PPBonus
{
    PPDirection dir;
public:
    PPArrow(PPCell& cell)
        :PPBonus(cell)
    {}

    void update(PPBoard& board) override;
    void apply(PPPlayer& pPlayer, PPBoard& board) override;
};

#endif /* defined(__PogoPainter__PPBonus__) */
