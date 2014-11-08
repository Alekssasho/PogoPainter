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

class PPBonus
{
public:
    PPBonus(PPCell& cell);
    virtual void apply(PPPlayer& pPlayer, PPBoard& board) = 0;
    virtual ~PPBonus();
private:
    PPCell& mCell;
};

class PPCheckpoint : public PPBonus
{
public:
    void apply(PPPlayer& pPlayer, PPBoard& board) override;
private:
};

#endif /* defined(__PogoPainter__PPBonus__) */
