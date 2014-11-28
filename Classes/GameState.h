//
//  PPGameState.h
//  PogoPainter
//
//  Created by Aleksandar Angelov on 11/27/14.
//
//

#ifndef __PogoPainter__PPGameState__
#define __PogoPainter__PPGameState__

#include <vector>

#include "Board.h"
#include "BaseClasses.h"

using Players = std::vector<PlayerPtr>;

class GameState
{
public:
    Board& board() { return mBoard; }
    Players& players() { return mPlayers; }
    int ticks() { return mTicks; }
    
    void serialize();
    void deserialize();
    
private:
    friend class GameManager;
    
    Board mBoard;
    Players mPlayers;
    int mTicks = 0;
};

#endif /* defined(__PogoPainter__PPGameState__) */
